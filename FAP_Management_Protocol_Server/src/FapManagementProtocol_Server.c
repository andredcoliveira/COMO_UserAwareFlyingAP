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
#include <math.h>
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
#define MAX_BUFFER                                      1024

#define TRUE	1
#define FALSE	0

#define HEARTBEAT_INTERVAL_NS	500000000L

// ----- FAP MANAGEMENT PROTOCOL - SERVER ADDRESS ----- //
#define SERVER_IP_ADDRESS		"10.0.0.254"
#define SERVER_PORT_NUMBER		40123

static GpsRawCoordinates fapOriginRawCoordinates = {0};


// ----- FAP MANAGEMENT PROTOCOL - GLOBAL VARIABLES ----- //

GpsNedCoordinates clients[MAX_ASSOCIATED_USERS]; 
int server_fd;
struct sockaddr_in address;
int alarm_flag = FALSE;
int exit_flag = FALSE;
int addrlen = sizeof(address);
threads_clients threads[MAX_ASSOCIATED_USERS+1]; 
pthread_mutex_t lock;
pthread_t t_main;
int active_users = 0;

pthread_t t_heartbeat;
int alive = FALSE;


// =========================================================
//           FUNCTIONS
// =========================================================
double calculate_distance(GpsNedCoordinates x1, GpsNedCoordinates x2)
{
    return sqrt(pow((x1.x-x2.x),2)+pow((x1.y-x2.y),2)+pow((x1.z-x2.z),2));
}


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

    char *timestamp = NULL;

    while(threads[user_id].alarm_flag == 0) {
        if(exit_flag == 1)
            break;

        if(strcmp(clients[user_id].timestamp, "") == 0)
            continue;
        
        now = time(&now);
        now_t = *gmtime(&now);
        timestamp = clients[user_id].timestamp;
        strptime(timestamp, "%Y-%m-%dT%H:%M:%SZ", &past);
        past.tm_isdst = now_t.tm_isdst;
        past_time = mktime(&past);
        if(difftime(now, past_time) > GPS_COORDINATES_UPDATE_TIMEOUT_SECONDS) {
            fprintf(stderr, "Too much time without updating coordinates, exiting now \n");
            threads[user_id].alarm_flag = TRUE;
        }
    }

    shutdown(threads[user_id].socket, SHUT_RDWR);
    pthread_mutex_lock(&lock);
    active_users--;
    pthread_mutex_unlock(&lock);
    clients[user_id].x= clients[user_id].y=clients[user_id].z=0;
    strcpy(clients[user_id].timestamp, "");
    return NULL;
}

char *handle_Gps_Update(int thread_id, JSON_Value *root) {
	char *string = NULL;
    ProtocolMsgType response;
    GpsRawCoordinates ClientRawCoordinates = {0};
    GpsNedCoordinates fapActualPosition    = {0};
	JSON_Object *object = json_value_get_object(root);
    //Handle Request
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
		&clients[thread_id], 
		&ClientRawCoordinates, 
		&fapOriginRawCoordinates
	);
    //Determine Fap Actual Position
    sendMavlinkMsg_localPositionNed(&fapActualPosition);
    if(calculate_distance(fapActualPosition, clients[thread_id])>MAX_ALLOWED_DISTANCE_FROM_FAP_METERS){
        printf("Distance Higher than 300m\n");
        return NULL;
    }
    //Create Response
    response = GPS_COORDINATES_ACK; 
    root = json_value_init_object();
    object = json_value_get_object(root);

    json_object_set_number(
		object, 
		PROTOCOL_PARAMETERS_USER_ID, 
		atoi(strrchr(SERVER_IP_ADDRESS, '.') + 1)
	); 
    json_object_set_number(object, PROTOCOL_PARAMETERS_MSG_TYPE, response);
    strcpyTimestampIso8601(Time, time(NULL));
    json_object_set_string(object, PROTOCOL_PARAMETERS_GPS_TIMESTAMP, Time);

    string = json_serialize_to_string(root);

	return string;
}

char *handle_association() {
    char *string = NULL;
    ProtocolMsgType response;

	if(active_users < MAX_ASSOCIATED_USERS) { 
        pthread_mutex_lock(&lock);
        active_users++;
        pthread_mutex_unlock(&lock);
        response = USER_ASSOCIATION_ACCEPTED;
    } else
        response = USER_ASSOCIATION_REJECTED;
    
    JSON_Value *root_value;
    JSON_Object *root_object;
    root_value = json_value_init_object();
    root_object = json_value_get_object(root_value);
 
    json_object_set_number(
		root_object, 
		PROTOCOL_PARAMETERS_USER_ID, 
		atoi(strrchr(SERVER_IP_ADDRESS, '.') + 1)
	); 
    json_object_set_number(
        root_object, 
        PROTOCOL_PARAMETERS_MSG_TYPE,
        response
    );
    string = json_serialize_to_string(root_value);
    json_value_free(root_value);

    return string;
}

char *handle_desassociation() {
    char *string = NULL;
    ProtocolMsgType response;

    response = USER_DESASSOCIATION_ACK;

    JSON_Value *root_value;
    JSON_Object *root_object;
    root_value = json_value_init_object();
    root_object = json_value_get_object(root_value);

    json_object_set_number(
		root_object, 
		PROTOCOL_PARAMETERS_USER_ID, 
		atoi(strrchr(SERVER_IP_ADDRESS, '.') + 1)
	); 
    json_object_set_number(root_object, PROTOCOL_PARAMETERS_MSG_TYPE, response);

    string = json_serialize_to_string(root_value);
    json_value_free(root_value);

    return string;
}

void *handler(void *thread_id) { 
    int id = *(int *) thread_id;

    fprintf(stderr, "ID: %d\n", id);
    threads[id].status = 1;
    ProtocolMsgType response;

    // JSON inicialization 
    JSON_Value *root_value;
    JSON_Object *root_object;
    root_value = json_value_init_object();

    // Waits for a response
    pthread_t alarm;
    char buffer[MAX_BUFFER]; 
    char *serialized_string = NULL;

	while(threads[id].alarm_flag == 0) {
		memset(buffer, 0, strlen(buffer));
		if((recv(threads[id].socket, buffer, MAX_BUFFER, 0) <= 0)) {
             if(active_users>0){
                threads[id].alarm_flag=TRUE;
                pthread_mutex_lock(&lock);
                active_users--;
                pthread_mutex_unlock(&lock);
            }
		  	fprintf(stderr, "Ending Connection \n");
			break;
		}
		printf("New Message: %s\n", buffer);
		root_value = json_parse_string(buffer);
		root_object = json_value_get_object(root_value);
		response = json_object_get_number(root_object, "msgType");
        if(response == USER_ASSOCIATION_REQUEST) {
            threads[id].user_id = json_object_get_number(root_object, PROTOCOL_PARAMETERS_USER_ID);
            pthread_create(&alarm, NULL, handler_alarm, (void *) &id);
            serialized_string = handle_association();
            fprintf(stderr, "Active Users: %d\n", active_users);
            send(threads[id].socket, serialized_string, strlen(serialized_string), 0);
        }
        else if(response == GPS_COORDINATES_UPDATE) {    
            serialized_string = handle_Gps_Update(id, root_value);
             if(serialized_string==NULL){
                threads[id].alarm_flag=TRUE;
                break;
            }
            printf("Gps Coordinates Updated\nUser_ID:%d\n", threads[id].user_id);
            send(threads[id].socket, serialized_string, strlen(serialized_string),0);
        }
        else if((response == USER_DESASSOCIATION_REQUEST) && (active_users > 0)) {
            serialized_string = handle_desassociation();
            pthread_mutex_lock(&lock);
            active_users--; // ! variável global numa operação não atómica -> usar mutex pcausa do 'if'
            pthread_mutex_unlock(&lock);
            fprintf(stderr, "Active Users: %d\n", active_users);
            send(threads[id].socket, serialized_string, strlen(serialized_string), 0);
            break;
        }
	}

    pthread_join(alarm, NULL);
    close(threads[id].socket);
    threads[id].alarm_flag = FALSE;
    threads[id].user_id = 0;
    threads[id].status = 0;
    fprintf(stderr, "Socket closed\n");
    json_free_serialized_string(serialized_string);
    json_value_free(root_value);

    return (void *) RETURN_VALUE_OK;
}

void *WaitConnection(void *socket) {
    int sk_main = *(int *) socket;

    while(exit_flag == 0) { // ! acho que isto devia levar mutex;
		int new = accept(sk_main, (struct sockaddr *) &address, (socklen_t *) &addrlen);
		if(new < 0) {
			perror("accept");
			return (void *) RETURN_VALUE_ERROR; // ! exit() manda abaixo a main() que estiver a usar a API, em vez de lhe retornar um valor de erro (i.e., RETURN_VALUE_ERROR)
		}

		int t = get_free_thread();
		if(t != -1) {
			threads[t].socket = new;
			pthread_create(&threads[t].tid, NULL, handler, (void *) &t); 
		} else {
			fprintf(stderr, "Reached user limit. Dropping incoming connection.\n");
		}
	}

    return (void *) RETURN_VALUE_OK;
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
		if(sendMavlinkMsg_heartbeat() != RETURN_VALUE_OK) {
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
    memset(&threads, 0 , (MAX_ASSOCIATED_USERS+1)*sizeof(threads_clients));  // ! nr max de threads deve depender de MAX_ASSOCIATED_USERS (pq 11 e não 10?)
    int opt = 1;

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
	//address.sin_addr.s_addr = SERVER_IP_ADDRESS;
    address.sin_port = htons(SERVER_PORT_NUMBER);

  	if (bind(server_fd, (struct sockaddr *) &address, sizeof(address)) < 0) {
        perror("bind");
        return RETURN_VALUE_ERROR;
    }

    if (listen(server_fd, 3) < 0) { // ! backlog = 3 (?); arbitrário? 
        perror("listen");
        return RETURN_VALUE_ERROR;
    }

    if (pthread_mutex_init(&lock, NULL) != 0)
    {
        printf("Error starting lock\n");
        return RETURN_VALUE_ERROR;

    }
    if(pthread_create(&t_main, NULL, WaitConnection, (void *) &server_fd)!=0){
        perror("Error starting main thread.");
        return RETURN_VALUE_ERROR;
    }

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
    void *retval;

    for(int i = 0; i < MAX_ASSOCIATED_USERS; i++){ // ! usar algo derivado de MAX_ASSOCIATED_USERS em vez de 11
        if(threads[i].status == 1){
			shutdown(threads[i].socket, SHUT_RDWR);
			if(pthread_join(threads[i].tid, &retval) != 0|| ((intptr_t) retval != RETURN_VALUE_OK))
				fprintf(stderr, "Error exiting thread #%d: %s\n", i, strerror(errno));
			else
				fprintf(stderr, "Ending thread's id: %d\n", i);
		}
	}

    shutdown(server_fd, SHUT_RDWR);
    close(server_fd);

    if(pthread_join(t_main, &retval) != 0|| ((intptr_t) retval != RETURN_VALUE_OK)) {
		perror("Error exiting server thread.");
        return RETURN_VALUE_ERROR;
	}

	// KILL HEARTBEAT
	
    if(pthread_mutex_destroy(&lock)!=0){
        perror("Error destroying lock\n");
        return RETURN_VALUE_ERROR;
    }
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
