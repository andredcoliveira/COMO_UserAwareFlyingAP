/******************************************************************************
*                         User-Aware Flying AP Project
*                       FAP Management Protocol (Server)
*******************************************************************************
*                        Comunicacoes Moveis 2017/2018
*                             FEUP | MIEEC / MIEIC
*******************************************************************************/

#define _GNU_SOURCE
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
//#include "json/json-builder.h"
#include "json/parson.h"

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
#define PROTOCOL_PARAMETERS_USER_ID                     "userId"
#define PROTOCOL_PARAMETERS_MSG_TYPE                    "msgType"
#define PROTOCOL_PARAMETERS_GPS_COORDINATES             "gpsCoordinates"
#define PROTOCOL_PARAMETERS_GPS_COORDINATES_LAT         "lat"
#define PROTOCOL_PARAMETERS_GPS_COORDINATES_LON         "lon"
#define PROTOCOL_PARAMETERS_GPS_COORDINATES_ALT         "alt"
#define PROTOCOL_PARAMETERS_GPS_COORDINATES_TIMESTAMP   "timestamp"
#define PROTOCOL_PARAMETERS_GPS_TIMESTAMP               "gpsTimestamp"

// Protocol "msgType" values
typedef enum _ProtocolMsgType
{
    USER_ASSOCIATION_REQUEST        = 1,
    USER_ASSOCIATION_ACCEPTED       = 2,
    USER_ASSOCIATION_REJECTED       = 3,
    USER_DESASSOCIATION_REQUEST     = 4,
    USER_DESASSOCIATION_ACK         = 5,
    GPS_COORDINATES_UPDATE          = 6,
    GPS_COORDINATES_ACK             = 7
} ProtocolMsgType;

// ----- FAP MANAGEMENT PROTOCOL - PARAMETERS ----- //

// GPS coordinates update period (in seconds)
#define GPS_COORDINATES_UPDATE_PERIOD_SECONDS           10
#define GPS_COORDINATES_UPDATE_TIMEOUT_SECONDS          (2 * GPS_COORDINATES_UPDATE_PERIOD_SECONDS)

// Max allowed distance from the users to the FAP (in meters)
#define MAX_ALLOWED_DISTANCE_FROM_FAP_METERS            300
#define MAX_BUFFER                                      1024

#define TRUE    1
#define FALSE   0

#define HEARTBEAT_INTERVAL_NS   500000000L

// ----- FAP MANAGEMENT PROTOCOL - SERVER ADDRESS ----- //
#define SERVER_IP_ADDRESS       "127.0.0.1"
#define SERVER_PORT_NUMBER      40123

static GpsRawCoordinates fapOriginRawCoordinates = {0};


// ----- FAP MANAGEMENT PROTOCOL - GLOBAL VARIABLES ----- //

GpsNedCoordinates clients[MAX_ASSOCIATED_USERS]; 
int server_fd;
struct sockaddr_in address;
int alarm_flag = FALSE;
int exit_flag = FALSE;
int addrlen = sizeof(address);
threads_clients threads[MAX_ASSOCIATED_USERS+MAX_REJECTED_USERS]; 
pthread_mutex_t lock;
pthread_t t_main;
int active_users = 0;

pthread_t t_heartbeat;
int alive = FALSE;


// =========================================================
//           FUNCTIONS
// =========================================================
double calculate_distance(GpsNedCoordinates x1, GpsNedCoordinates x2) {
    return sqrt(pow((x1.x-x2.x), 2) + 
				pow((x1.y-x2.y), 2) + 
				pow((x1.z-x2.z), 2));
}

int get_free_thread() {
    for(int i = 0; i < (MAX_ASSOCIATED_USERS + MAX_REJECTED_USERS); i++) {
        if(threads[i].status == 0) {
			return i;
		}
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
            FAP_SERVER_PRINT("Too much time without updating coordinates, exiting now.");
            threads[user_id].alarm_flag = TRUE;
        }
    }
    shutdown(threads[user_id].socket, SHUT_RDWR);
    // pthread_mutex_lock(&lock);
    // active_users--;
    // pthread_mutex_unlock(&lock);
    FAP_SERVER_PRINT("Exiting Alarm.");
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
        FAP_SERVER_PRINT_ERROR("Distance longer than 300m.");
        return NULL;
    }
    //Create Response
    response = GPS_COORDINATES_ACK; 
    root = json_value_init_object();
    object = json_value_get_object(root);
    int id=threads[thread_id].user_id;

    json_object_set_number(
        object, 
        PROTOCOL_PARAMETERS_USER_ID, 
        id
    ); 
    json_object_set_number(object, PROTOCOL_PARAMETERS_MSG_TYPE, response);
    strcpyTimestampIso8601(Time, time(NULL));
    json_object_set_string(object, PROTOCOL_PARAMETERS_GPS_TIMESTAMP, Time);

    string = json_serialize_to_string(root);

    return string;
}

char *handle_association(int id) {
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
        id
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

char *handle_desassociation(int id) {
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
        id
        ); 
    json_object_set_number(root_object, PROTOCOL_PARAMETERS_MSG_TYPE, response);

    string = json_serialize_to_string(root_value);
    json_value_free(root_value);

    return string;
}

void *handler(void *thread_id) { 
    int id = *(int *) thread_id;

    FAP_SERVER_PRINT("Handler Created with ID: %d", id);
    threads[id].status = 1;
    ProtocolMsgType response;
    //set of socket descriptors
    fd_set readfds;
    int max_sd;
    struct timeval timeout;

	int bad = 0;
	int res = 0;

    // JSON inicialization 
    JSON_Value *root_value;
    JSON_Object *root_object;
    root_value = json_value_init_object();

    // Waits for a response
    pthread_t alarm;
    char buffer[MAX_BUFFER]; 
    char *serialized_string = NULL;

    if(pthread_create(&alarm, NULL, handler_alarm, (void *) &id) != 0) {
		FAP_SERVER_PRINT("Error starting GPS Coordinates update handler thread");
	}

    while(threads[id].alarm_flag == 0) {

		if(bad >= 1) {
			// timed out
			threads[id].alarm_flag = TRUE;
			FAP_SERVER_PRINT("Reached attempt limit. Ending connection.");
			break;
		}

        memset(buffer, 0, strlen(buffer));
        FD_ZERO(&readfds);
        FD_SET(threads[id].socket, &readfds);
        max_sd = threads[id].socket;
        timeout.tv_sec = GPS_COORDINATES_UPDATE_TIMEOUT_SECONDS*1.5; 
        timeout.tv_usec = 0;

		res = select(max_sd + 1, &readfds, NULL, NULL, &timeout);
		if(res == -1) {
			threads[id].alarm_flag = TRUE;
            FAP_SERVER_PRINT("Error when using Select.");
            break;
		} else if(res == 0) {
			bad++;
			memset(buffer, 0, strlen(buffer));
			FAP_SERVER_PRINT("Timed-out. Trying again..");
			continue;
		} else if(FD_ISSET(threads[id].socket, &readfds)) {
			// socket has data
			if(recv(threads[id].socket, buffer, MAX_BUFFER, 0) <= 0) {
				threads[id].alarm_flag = TRUE;
				FAP_SERVER_PRINT("Ending Connection.");
				break;
			}
		}

        root_value = json_parse_string(buffer);
        FAP_SERVER_PRINT("New Message: \n%s\n", json_serialize_to_string_pretty(root_value));
        root_object = json_value_get_object(root_value);
        response = json_object_get_number(root_object, "msgType");

        if(response == USER_ASSOCIATION_REQUEST) {
            threads[id].user_id = json_object_get_number(root_object, PROTOCOL_PARAMETERS_USER_ID);
            serialized_string = handle_association(threads[id].user_id);
            FAP_SERVER_PRINT("Active Users: %d", active_users);
            send(threads[id].socket, serialized_string, strlen(serialized_string), 0);
        }
        else if(response == GPS_COORDINATES_UPDATE) {    
            serialized_string = handle_Gps_Update(id, root_value);
            if(serialized_string == NULL){
                threads[id].alarm_flag = TRUE;
                break;
            }
            FAP_SERVER_PRINT("Gps Coordinates Updated [User ID - %d]",threads[id].user_id);
            send(threads[id].socket, serialized_string, strlen(serialized_string),0);
        }
        else if((response == USER_DESASSOCIATION_REQUEST) && (active_users > 0)) {
            serialized_string = handle_desassociation(threads[id].user_id);
            FAP_SERVER_PRINT("Active Users: %d", active_users - 1);
            send(threads[id].socket, serialized_string, strlen(serialized_string), 0);
            threads[id].alarm_flag = TRUE;
            break;
        }
    }

    pthread_join(alarm, NULL);
    threads[id].alarm_flag = FALSE;
    threads[id].user_id = 0;
    threads[id].status = 0;
    if(active_users > 0){
        pthread_mutex_lock(&lock);
        active_users--;
        pthread_mutex_unlock(&lock);
    }
    FAP_SERVER_PRINT("Socket closed.");
    json_free_serialized_string(serialized_string);
    json_value_free(root_value);
    close(threads[id].socket);
    return (void *) RETURN_VALUE_OK;
}

void *WaitConnection(void *socket) {
    int sk_main = *(int *) socket;

    while(exit_flag == FALSE) {
		int new = accept(sk_main, (struct sockaddr *) &address, (socklen_t *) &addrlen);
        if((new < 0) && exit_flag == FALSE) {
            FAP_SERVER_PRINT("Error accepting connection.");
            return (void *) RETURN_VALUE_ERROR;
        }

		if(exit_flag == TRUE) {
			break;
		}
        int t = get_free_thread();
        if(t != -1) {
            threads[t].socket = new;
            if(pthread_create(&threads[t].tid, NULL, handler, (void *) &t) != 0) {
				FAP_SERVER_PRINT("Error starting a handler thread.");
				return (void *) RETURN_VALUE_ERROR;
			}
        } else {
            FAP_SERVER_PRINT("Reached user limit. Dropping incoming connection.");
        }
    }

    return (void *) RETURN_VALUE_OK;
}

void *sendHeartbeat() {

    struct timespec start, stop, remaining;

    while(alive) {
        if(clock_gettime(CLOCK_MONOTONIC, &start) < 0) {
           FAP_SERVER_PRINT("Error clocking heartbeat.");
            alive = FALSE;
            return (void *) RETURN_VALUE_ERROR;
        }

        // send Mavlink message - HEARTBEAT
        if(sendMavlinkMsg_heartbeat() != RETURN_VALUE_OK) {
            FAP_SERVER_PRINT("Error sending Heartbeat message.");
            alive = FALSE;
            return (void *) RETURN_VALUE_ERROR;
        }

        if(clock_gettime(CLOCK_MONOTONIC, &stop) < 0) {
            FAP_SERVER_PRINT("Error clocking heartbeat.");
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
    memset(&threads, 0 , (MAX_ASSOCIATED_USERS+MAX_REJECTED_USERS)*sizeof(threads_clients)); 
    int opt = 1;
	exit_flag = FALSE;

    if(initializeMavlink() != RETURN_VALUE_OK 
            || sendMavlinkMsg_gpsGlobalOrigin(&fapOriginRawCoordinates) != RETURN_VALUE_OK)
        return RETURN_VALUE_ERROR;

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        FAP_SERVER_PRINT_ERROR("socket failed.");
        return RETURN_VALUE_ERROR;
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        FAP_SERVER_PRINT_ERROR("setsockopt.");
        return RETURN_VALUE_ERROR;
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr(SERVER_IP_ADDRESS);
    address.sin_port = htons(SERVER_PORT_NUMBER);

    if (bind(server_fd, (struct sockaddr *) &address, sizeof(address)) < 0) {
        FAP_SERVER_PRINT_ERROR("bind.");
        return RETURN_VALUE_ERROR;
    }

    if (listen(server_fd, 3) < 0) {
        FAP_SERVER_PRINT_ERROR("listen.");
        return RETURN_VALUE_ERROR;
    }

    if (pthread_mutex_init(&lock, NULL) != 0)
    {
        FAP_SERVER_PRINT_ERROR("Error starting lock.");
        return RETURN_VALUE_ERROR;

    }
    if(pthread_create(&t_main, NULL, WaitConnection, (void *) &server_fd) != 0){
        FAP_SERVER_PRINT_ERROR("Error starting main thread.");
        return RETURN_VALUE_ERROR;
    }

    // Start heartbeat
    alive = TRUE;

    if(pthread_create(&t_heartbeat, NULL, sendHeartbeat, NULL) != 0) {
        FAP_SERVER_PRINT_ERROR("Error starting heartbeat.");
        return RETURN_VALUE_ERROR;
    }

    return RETURN_VALUE_OK;
}


int terminateFapManagementProtocol()
{
    exit_flag = 1;
    void *retval;

    for(int i = 0; i < (MAX_ASSOCIATED_USERS+MAX_REJECTED_USERS); i++){
        if(threads[i].status == 1){
            shutdown(threads[i].socket, SHUT_RDWR);
             if(pthread_join(threads[i].tid, &retval) != 0 || ((intptr_t) retval != RETURN_VALUE_OK)){
                FAP_SERVER_PRINT("Error exiting thread #%d: %s", i, strerror(errno));
            }
            else
                FAP_SERVER_PRINT("Ending thread's id: %d", i);
        }
    }

    shutdown(server_fd, SHUT_RDWR);
    close(server_fd);

    if(pthread_join(t_main, &retval) != 0 || ((intptr_t) retval != RETURN_VALUE_OK)) {
		FAP_SERVER_PRINT_ERROR("Error exiting server thread.");
		return RETURN_VALUE_ERROR;
    }

    // KILL HEARTBEAT
    
    if(pthread_mutex_destroy(&lock)!=0){
        FAP_SERVER_PRINT_ERROR("Error destroying lock.");
        return RETURN_VALUE_ERROR;
    }
    alive = FALSE;
    if((pthread_join(t_heartbeat, &retval) != 0) || ((intptr_t) retval != RETURN_VALUE_OK)) {
        FAP_SERVER_PRINT_ERROR("Error stopping heartbeat.");
        return RETURN_VALUE_ERROR;
    }

	if(terminateMavlink() != RETURN_VALUE_OK)
		return RETURN_VALUE_ERROR;

    return RETURN_VALUE_OK;
}


int moveFapToGpsNedCoordinates(const GpsNedCoordinates *gpsNedCoordinates)
{
    // check if pointer is valid
    if(gpsNedCoordinates == NULL) {
        FAP_SERVER_PRINT_ERROR("Can't move FAP to target NED coordinates: Invalid coordinates struct.");
        return RETURN_VALUE_ERROR;
    }

    // send Mavlink message - SET_POSITION_TARGET_LOCAL_NED
    if(sendMavlinkMsg_setPositionTargetLocalNed(gpsNedCoordinates) != RETURN_VALUE_OK) {
        FAP_SERVER_PRINT_ERROR("Can't move FAP to target NED coordinates: Error sending Mavlink message.");
        return RETURN_VALUE_ERROR;
    }

    // print status message
    FAP_SERVER_PRINT("Moving FAP to NED coordinates: ");
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
        FAP_SERVER_PRINT_ERROR("Can't obtain FAP NED coordinates: Error sending Mavlink message.");
        return RETURN_VALUE_ERROR;
    }

    // print status message
    FAP_SERVER_PRINT("FAP is at NED coordinates: ");
    PRINT_GPS_NED_COORDINATES((*gpsNedCoordinates));

    return RETURN_VALUE_OK;
}


int getAllUsersGpsNedCoordinates(GpsNedCoordinates *gpsNedCoordinates, int *n)
{
    if(gpsNedCoordinates == NULL) {
        FAP_SERVER_PRINT_ERROR("Invalid coordinates pointer.");
        return RETURN_VALUE_ERROR;
    }
	if(n == NULL) {
		FAP_SERVER_PRINT_ERROR("Invalid users number pointer.");
        return RETURN_VALUE_ERROR;
	}

    (*n) = 0;

    for(int i = 0; i < MAX_ASSOCIATED_USERS; i++) { 
        if(strcmp(clients[i].timestamp, "") != 0) {
            gpsNedCoordinates[(*n)] = clients[i];
            (*n)++;
        }
    }

    return RETURN_VALUE_OK;
}
