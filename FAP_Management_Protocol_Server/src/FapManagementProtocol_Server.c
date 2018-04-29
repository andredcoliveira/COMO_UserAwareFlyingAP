/******************************************************************************
*                         User-Aware Flying AP Project
*                       FAP Management Protocol (Server)
*******************************************************************************
*                        Comunicacoes Moveis 2017/2018
*                             FEUP | MIEEC / MIEIC
*******************************************************************************/


#include <unistd.h>


// Module headers
#include "FapManagementProtocol_Server.h"
#include "MavlinkEmulator.h"
#include "GpsCoordinates.h"


// MAVLink library
// [https://mavlink.io/en/getting_started/use_source.html]
//#include "mavlink/common/mavlink.h"

// JSON parser
// [https://github.com/udp/json-parser and https://github.com/udp/json-builder]
#include "json/parson.h"
//#include "json/json-builder.h"

// C headers
// (...)
#include <signal.h>
#include <pthread.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <errno.h>

// =========================================================
//           DEFINES
// =========================================================

// ----- MAVLINK PROTOCOL ----- //

// Use MAVLink helper functions
//#define MAVLINK_USE_CONVENIENCE_FUNCTIONS


// ----- FAP MANAGEMENT PROTOCOL - MESSAGES ----- //

// Protocol parameters
#define PROTOCOL_PARAMETERS_USER_ID						"userId"
#define PROTOCOL_PARAMETERS_MSG_TYPE					"msgType"
#define PROTOCOL_PARAMETERS_GPS_COORDINATES				"gpsCoordinates"
#define PROTOCOL_PARAMETERS_GPS_COORDINATES_LAT			"lat"
#define PROTOCOL_PARAMETERS_GPS_COORDINATES_LON			"lon"
#define PROTOCOL_PARAMETERS_GPS_COORDINATES_ALT			"alt"
#define PROTOCOL_PARAMETERS_GPS_COORDINATES_TIMESTAMP	"timestamp"
#define PROTOCOL_PARAMETERS_GPS_TIMESTAMP				"gpsTimestamp"

// Protocol "msgType" values
typedef enum _ProtocolMsgType
{
	USER_ASSOCIATION_REQUEST		= 1,
	USER_ASSOCIATION_ACCEPTED		= 2,
	USER_ASSOCIATION_REJECTED		= 3,
	USER_DESASSOCIATION_REQUEST		= 4,
	USER_DESASSOCIATION_ACK			= 5,
	GPS_COORDINATES_UPDATE			= 6,
	GPS_COORDINATES_ACK				= 7
} ProtocolMsgType;

// ----- FAP MANAGEMENT PROTOCOL - PARAMETERS ----- //

// GPS coordinates update period (in seconds)
#define GPS_COORDINATES_UPDATE_PERIOD_SECONDS			10
#define GPS_COORDINATES_UPDATE_TIMEOUT_SECONDS			(2 * GPS_COORDINATES_UPDATE_PERIOD_SECONDS)

// Max allowed distance from the users to the FAP (in meters)
#define MAX_ALLOWED_DISTANCE_FROM_FAP_METERS			300


#define TRUE	1
#define FALSE	0

#define HEARTBEAT_INTERVAL_NS	500000000L

// ----- FAP MANAGEMENT PROTOCOL - SERVER ADDRESS ----- //
#define SERVER_IP_ADDRESS		"10.0.0.254"
#define SERVER_PORT_NUMBER		40123

static GpsRawCoordinates fapOriginRawCoordinates = {0};


// ----- FAP MANAGEMENT PROTOCOL - GLOBAL VARIABLES ----- //

GpsNedCoordinates clients[254]; // ! talvez MAX_ASSOCIATED_USERS em vez de 254 (mesmo que seja uma rede /24)
int server_fd;
struct sockaddr_in address;
int opt = 1;
int alarm_flag = 0;
int exit_flag = 0;
int addrlen = sizeof(address);
threads_clients threads[11];  // ! nr max de threads deve depender de MAX_ASSOCIATED_USERS (pq 11 e não 10 ou mesmo MAX_ASSOCIATED_USERS?)

pthread_t t_main;
int active_users = 0;

pthread_t t_heartbeat;
int alive = FALSE;


// =========================================================
//           FUNCTIONS
// =========================================================

int get_free_thread() {
    for(int i = 0; i < 11; i++) {
        if(threads[i].status == 0)
            return i;
    }

    return -1;
}

void *handler_alarm(void *id) {
    int user_id = *(int *) id;
    time_t now, past_time;
    struct tm now_t, past;

    fprintf(stderr, "Thread id:%d\n", user_id);
    fprintf(stderr, "User_ID: %d\n", threads[user_id].user_id);

    char *timestamp = NULL;

    while(threads[user_id].alarm_flag == 0) {
        if(exit_flag == 1)
            break;
		// fprintf(stderr, "ALARM CREATED\n");
        if(strcmp(clients[threads[user_id].user_id].timestamp, "") == 0)
            continue;
        
        now = time(&now);
        now_t = *gmtime(&now);
        timestamp = clients[threads[user_id].user_id].timestamp;
        strptime(timestamp, "%Y-%m-%dT%H:%M:%SZ", &past);
        past.tm_isdst = now_t.tm_isdst;
        past_time = mktime(&past);
        if(difftime(now, past_time) > GPS_COORDINATES_UPDATE_TIMEOUT_SECONDS) {
            fprintf(stderr, "Too much time without updating coordinates, exiting now \n");
            threads[user_id].alarm_flag = 1;
        }
    }

    shutdown(threads[user_id].socket, SHUT_RDWR);
    active_users--;
    strcpy(clients[threads[user_id].user_id].timestamp, "");
    return NULL;
}

char *handle_Gps_Update(ProtocolMsgType response, JSON_Value *root) {
	char *string = NULL;
    GpsRawCoordinates ClientRawCoordinates = {0};
	JSON_Object *object = json_value_get_object(root);

    //Handle Request
	int user_id = (int) json_object_get_number(object, PROTOCOL_PARAMETERS_USER_ID);
	double latitude = json_object_dotget_number(
		object, 
		PROTOCOL_PARAMETERS_GPS_COORDINATES "." PROTOCOL_PARAMETERS_GPS_COORDINATES_LAT
	);
	double longitude = json_object_dotget_number(
		object, 
		PROTOCOL_PARAMETERS_GPS_COORDINATES "." PROTOCOL_PARAMETERS_GPS_COORDINATES_LON
	);
	double altitude = json_object_dotget_number(
		object, 
		PROTOCOL_PARAMETERS_GPS_COORDINATES "." PROTOCOL_PARAMETERS_GPS_COORDINATES_ALT
	);
    char *Time = json_object_dotget_string(
		object, 
		PROTOCOL_PARAMETERS_GPS_COORDINATES "." PROTOCOL_PARAMETERS_GPS_COORDINATES_TIMESTAMP
	);

    initializeGpsRawCoordinates(
		&ClientRawCoordinates, 
		latitude, longitude, altitude, (time_t) NULL
	);
    strcpy(ClientRawCoordinates.timestamp, Time);

    gpsRawCoordinates2gpsNedCoordinates(
		&clients[user_id], 
		&ClientRawCoordinates, 
		&fapOriginRawCoordinates
	);

    //Create Response
    response = GPS_COORDINATES_ACK; // ! receber response como parâmetro da função não é um bocado redundante?
    root = json_value_init_object();
    object = json_value_get_object(root);

    // json_object_set_number(object, PROTOCOL_PARAMETERS_USER_ID, 254); // ! alternativa estática
    json_object_set_number(
		object, 
		PROTOCOL_PARAMETERS_USER_ID, 
		atoi(strrchr(SERVER_IP_ADDRESS, '.') + 1)
	); // ! alternativa dinâmica
    json_object_set_number(object, PROTOCOL_PARAMETERS_MSG_TYPE, response);
    strcpyTimestampIso8601(Time, time(NULL));
    puts(Time); // ! imprimir o tempo? (debug?)
    json_object_set_string(object, PROTOCOL_PARAMETERS_GPS_TIMESTAMP, Time);

    string = json_serialize_to_string(root);
    puts(string); // ! imprimir a string? (debug?)

	return string;
}

char *handle_association(ProtocolMsgType response) {
    char *string = NULL;

	if(active_users < 10) { // ! alterar para MAX_ASSOCIATED_USERS
        active_users++;
        // fprintf(stderr, "Entrei\n"); // ! debug (?), comentei
        response = USER_ASSOCIATION_ACCEPTED;
    } else
        response = USER_ASSOCIATION_REJECTED;
    
    JSON_Value *root_value;
    JSON_Object *root_object;
    root_value = json_value_init_object();
    root_object = json_value_get_object(root_value);
    // json_object_set_number(root_object, PROTOCOL_PARAMETERS_USER_ID, 254); // ! alternativa estática
    json_object_set_number(
		root_object, 
		PROTOCOL_PARAMETERS_USER_ID, 
		atoi(strrchr(SERVER_IP_ADDRESS, '.') + 1)
	); // ! alternativa dinâmica
    
    string = json_serialize_to_string(root_value);
    json_value_free(root_value);

    return string;
}

char *handle_desassociation(ProtocolMsgType response) {
    char *string = NULL;
    response = USER_DESASSOCIATION_ACK;

    JSON_Value *root_value;
    JSON_Object *root_object;
    root_value = json_value_init_object();
    root_object = json_value_get_object(root_value);

    // json_object_set_number(root_object, PROTOCOL_PARAMETERS_USER_ID, 254); // ! alternativa estática
    json_object_set_number(
		root_object, 
		PROTOCOL_PARAMETERS_USER_ID, 
		atoi(strrchr(SERVER_IP_ADDRESS, '.') + 1)
	); // ! alternativa dinâmica
    json_object_set_number(root_object, PROTOCOL_PARAMETERS_MSG_TYPE, response);

    string = json_serialize_to_string(root_value);
    json_value_free(root_value);

    return string;
}

void *handler(void *status) { // ! status é capaz de ser misleading na interpretação... talvez thread_id ou algo parecido?
    int id = *(int *) status;
    fprintf(stderr, "ID: %d\n", id);
    threads[id].status = 1;
    ProtocolMsgType response;

    // JSON inicialization 
    JSON_Value *root_value;
    JSON_Object *root_object;
    root_value = json_value_init_object();

    // Waits for a response
    pthread_t alarm;
    char buffer[1024] = { 0 }; // ! talvez definir o tamanho com macro
    char *serialized_string = NULL;

	while(threads[id].alarm_flag == 0) {
		memset(buffer, 0, strlen(buffer)); // ! o buffer não foi já inicializado com '\0' (i.e., 0) nas posições todas?
		if((recv(threads[id].socket, buffer, 1024, 0) <= 0) && (threads[id].alarm_flag == 0)) {
			fprintf(stderr, "Ending Connection \n");
			break;
		}
		puts(buffer); // ! mudar para uma impressão mais formatada (se não for apenas debug)
		root_value = json_parse_string(buffer);
		root_object = json_value_get_object(root_value);
		response = json_object_get_number(root_object, PROTOCOL_PARAMETERS_MSG_TYPE);

        if(response == USER_ASSOCIATION_REQUEST) {
            threads[id].user_id = json_object_get_number(root_object, PROTOCOL_PARAMETERS_USER_ID);
            pthread_create(&alarm, NULL, handler_alarm, (void *) &id);
            serialized_string = handle_association(response);
            fprintf(stderr, "Active Users: %d\n", active_users);
            send(threads[id].socket, serialized_string, strlen(serialized_string), 0);
        }
        else if(response == GPS_COORDINATES_UPDATE) {         
            serialized_string = handle_Gps_Update(response, root_value);
        }
        else if((response == USER_DESASSOCIATION_REQUEST) && (active_users > 0)) {
            serialized_string = handle_desassociation(response);
            active_users--; // ! variável global numa operação não atómica -> usar mutex pcausa do 'if'
            fprintf(stderr, "Active Users: %d\n", active_users);
            send(threads[id].socket, serialized_string, strlen(serialized_string), 0);
            break;
        }
	}

	// ! se houver outras funções que dependam destes valores -> mutex
    pthread_join(alarm, NULL);
    close(threads[id].socket);
    threads[id].alarm_flag = 0;
    threads[id].user_id = 0;
    threads[id].status = 0;
    fprintf(stderr, "Socket closed\n");
    json_free_serialized_string(serialized_string);
    json_value_free(root_value);

    return NULL;
}

void *WaitConnection(void *socket) {
    int sk_main = *(int *) socket;

    while(exit_flag == 0) { // ! acho que isto devia levar mutex;
		int new = accept(sk_main, (struct sockaddr *) &address, (socklen_t *) &addrlen);
		if(new < 0) {
			// break; // ! com este break a perror() e a exit() nunca são executadas, e como a exit() termina o processo...
			perror("accept");
			exit(EXIT_FAILURE); // ! exit() manda abaixo a main() que estiver a usar a API, em vez de lhe retornar um valor de erro (i.e., RETURN_VALUE_ERROR)
		}

		int t = get_free_thread();
		// printf("AQUII: %d\n",t); // ! debug (?), comentei
		if(t != -1) {
			threads[t].socket = new;
			pthread_create(&threads[t].tid, NULL, handler, (void *) &t); 
		} else {
			fprintf(stderr, "Reached user limit. Dropping incoming connection.\n");
		}
	}

    return NULL;
}

void *sendHeartbeat() {

	struct timespec start, stop, remaining;

	while(alive) {
		if(clock_gettime(CLOCK_MONOTONIC, &start) < 0) {
			perror("Error clocking heartbeat.");
			alive = FALSE;
			return (void *) RETURN_VALUE_ERROR;
		}

		// send Mavlink message - HEARTBEAT
		if(sendMavlinkMsg_heartbeat != RETURN_VALUE_OK) {
			fprintf(stderr, "Error sending Heartbeat message.");
			alive = FALSE;
			return (void *) RETURN_VALUE_ERROR;
		}

		if(clock_gettime(CLOCK_MONOTONIC, &stop) < 0) {
			perror("Error clocking heartbeat.");
			alive = FALSE;
			return (void *) RETURN_VALUE_ERROR;
		}

		if(stop.tv_nsec < start.tv_nsec)
			remaining.tv_nsec = HEARTBEAT_INTERVAL_NS - (1000000000L - start.tv_nsec + stop.tv_nsec);
		else
			remaining.tv_nsec = HEARTBEAT_INTERVAL_NS - (stop.tv_nsec - start.tv_nsec);
		remaining.tv_sec = stop.tv_sec - start.tv_sec;

		if(remaining.tv_sec < 0 || remaining.tv_nsec < 0)
			continue;
		nanosleep(&remaining, NULL);
	}

	return (void *) RETURN_VALUE_OK;
}

// =========================================================
//           PUBLIC API
// =========================================================

int initializeFapManagementProtocol()
{
    memset(clients, 0, sizeof(clients));
    memset(&threads, 0 , 11*sizeof(threads_clients));  // ! nr max de threads deve depender de MAX_ASSOCIATED_USERS (pq 11 e não 10?)

    if(initializeMavlink() != RETURN_VALUE_OK 
			|| sendMavlinkMsg_gpsGlobalOrigin(&fapOriginRawCoordinates) != RETURN_VALUE_OK)
		return RETURN_VALUE_ERROR;

	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        return RETURN_VALUE_ERROR;
    }

	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
        return RETURN_VALUE_ERROR;
    }

    address.sin_family = AF_INET;
	// address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(SERVER_PORT_NUMBER);

  	if (bind(server_fd, (struct sockaddr *) &address, sizeof(address)) < 0) {
        perror("bind");
        return RETURN_VALUE_ERROR;
    }

    if (listen(server_fd, 3) < 0) { // ! backlog = 3 (?); arbitrário? 
        perror("listen");
        return RETURN_VALUE_ERROR;
    }
    pthread_create(&t_main, NULL, WaitConnection, (void *) &server_fd);


    // Start heartbeat
	alive = TRUE;

	if(pthread_create(&t_heartbeat, NULL, sendHeartbeat(), NULL) != 0) {
		perror("Error starting heartbeat.");
		return RETURN_VALUE_ERROR;
	}


	return RETURN_VALUE_OK;
}


int terminateFapManagementProtocol()
{
    exit_flag = 1;
    // fprintf(stderr, "Exit Flag: %d\n", exit_flag); // ! debug (?), comentei

    for(int i = 0; i < 11; i++){ // ! usar algo derivado de MAX_ASSOCIATED_USERS em vez de 11
        if(threads[i].status == 1){
			shutdown(threads[i].socket, SHUT_RDWR);
			if(pthread_join(threads[i].tid, NULL) != 0)
				fprintf(stderr, "Error exiting thread #%d: %s\n", i, strerror(errno));
			else
				fprintf(stderr, "Ending thread's id: %d\n", i);
		}
	}

    shutdown(server_fd, SHUT_RDWR);
    close(server_fd);

    if(pthread_join(t_main, NULL) != 0) {
		perror("Error exiting server thread.");
	}

	// KILL HEARTBEAT
	void *retval;

	alive = FALSE;
	if((pthread_join(t_heartbeat, &retval) != 0) || ((intptr_t) retval != RETURN_VALUE_OK)) {
		perror("Error stopping heartbeat.");
		return RETURN_VALUE_ERROR;
	}

	return RETURN_VALUE_OK;
}


int moveFapToGpsNedCoordinates(const GpsNedCoordinates *gpsNedCoordinates)
{
	// check if pointer is valid
	if(gpsNedCoordinates == NULL) {
		fprintf(stderr, "Can't move FAP to target NED coordinates: Invalid coordinates struct\n");
		return RETURN_VALUE_ERROR;
	}

	// send Mavlink message - SET_POSITION_TARGET_LOCAL_NED
	if(sendMavlinkMsg_setPositionTargetLocalNed(gpsNedCoordinates) != RETURN_VALUE_OK) {
		fprintf(stderr, "Can't move FAP to target NED coordinates: Error sending Mavlink message\n");
		return RETURN_VALUE_ERROR;
	}

	// print status message
	fprintf(stderr, "Moving FAP to NED coordinates: ");
	PRINT_GPS_NED_COORDINATES((*gpsNedCoordinates));

	return RETURN_VALUE_OK;
}


int getFapGpsNedCoordinates(GpsNedCoordinates *gpsNedCoordinates)
{
	// check if pointer has been initialized; if not, do it ourselves
	if(gpsNedCoordinates == NULL)
		gpsNedCoordinates = realloc(gpsNedCoordinates, sizeof(GpsNedCoordinates));

	// check realloc's success
	if(gpsNedCoordinates == NULL)
		return RETURN_VALUE_ERROR;
		
	// send Mavlink message - LOCAL_POSITION_NED
	if(sendMavlinkMsg_localPositionNed(gpsNedCoordinates) != RETURN_VALUE_OK) {
		fprintf(stderr, "Can't obtain FAP NED coordinates: Error sending Mavlink message\n");
		return RETURN_VALUE_ERROR;
	}

	// print status message
	fprintf(stderr, "FAP is at NED coordinates: ");
	PRINT_GPS_NED_COORDINATES((*gpsNedCoordinates));

	return RETURN_VALUE_OK;
}


int getAllUsersGpsNedCoordinates(GpsNedCoordinates *gpsNedCoordinates, int *n)
{
	// ! é melhor: ou usar mutex, ou guardar uma cópia de clients (para além de active_users) logo no início da função
	int aux = active_users;

	// making sure there's enough space to accomodate all users' coordinates
	gpsNedCoordinates = realloc(gpsNedCoordinates, aux*sizeof(GpsNedCoordinates));
	if(gpsNedCoordinates == NULL)
		return RETURN_VALUE_ERROR;

    (*n) = 0;

    for(int i = 0; i < 254; i++) { // ! talvez MAX_ASSOCIATED_USERS em vez de 254 (mesmo que seja uma rede /24)
        if(strcmp(clients[i].timestamp, "") != 0) {
			//check if user number increased since previous realloc
			if((*n)+1 > aux) {
				gpsNedCoordinates = realloc(gpsNedCoordinates, (++aux)*sizeof(GpsNedCoordinates));
				if(gpsNedCoordinates == NULL)
					return RETURN_VALUE_ERROR;
			}
			gpsNedCoordinates[(*n)] = clients[i];
            (*n)++;
        }
    }

	return RETURN_VALUE_OK;
}
