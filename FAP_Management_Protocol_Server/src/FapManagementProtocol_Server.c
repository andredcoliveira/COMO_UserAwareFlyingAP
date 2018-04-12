/******************************************************************************
*                         User-Aware Flying AP Project
*                       FAP Management Protocol (Server)
*******************************************************************************
*                        Comunicacoes Moveis 2017/2018
*                             FEUP | MIEEC / MIEIC
*******************************************************************************/

// Module headers
#include "FapManagementProtocol_Server.h"
#include <unistd.h>
// MAVLink library
// [https://mavlink.io/en/getting_started/use_source.html]
#include "mavlink/common/mavlink.h"

// JSON parser
// [https://github.com/udp/json-parser and https://github.com/udp/json-builder]
#include "json/parson.h"
//#include "json/json-builder.h"

// C headers
// (...)
#include <pthread.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
// =========================================================
//           DEFINES
// =========================================================

// ----- MAVLINK PROTOCOL ----- //

// Use MAVLink helper functions
#define MAVLINK_USE_CONVENIENCE_FUNCTIONS


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


// ----- FAP MANAGEMENT PROTOCOL - SERVER ADDRESS ----- //
#define SERVER_IP_ADDRESS		"10.0.0.254"
#define SERVER_PORT_NUMBER		40123

int server_fd;
struct sockaddr_in address;
int opt = 1;
int addrlen = sizeof(address);
char *serialized_string = NULL;
int active_users=0;

// =========================================================
//           FUNCTIONS
// =========================================================

// TODO: Develop the required functions to implement the FAP Management
// Protocol (Server)
char * handle_association(ProtocolMsgType response){
    char *string=NULL;
    response=USER_ASSOCIATION_ACCEPTED;
    JSON_Value *root_value;
    JSON_Object *root_object;
    root_value = json_value_init_object();
    root_object = json_value_get_object(root_value);
    json_object_set_number(root_object, PROTOCOL_PARAMETERS_USER_ID , 254);
    json_object_set_number(root_object, PROTOCOL_PARAMETERS_MSG_TYPE, response);
    string=json_serialize_to_string(root_value);
    json_value_free(root_value);
    return string;
}
char *handle_desassociation(ProtocolMsgType response){
    char *string=NULL;
    response=USER_DESASSOCIATION_ACK;
    JSON_Value *root_value;
    JSON_Object *root_object;
    root_value = json_value_init_object();
    root_object = json_value_get_object(root_value);
    json_object_set_number(root_object, PROTOCOL_PARAMETERS_USER_ID , 254);
    json_object_set_number(root_object, PROTOCOL_PARAMETERS_MSG_TYPE, response);
    string=json_serialize_to_string(root_value);
    json_value_free(root_value);
    return string;
}
void * handler (void *socket){
     ProtocolMsgType response;
    // JSON inicialization 
    JSON_Value *root_value;
    JSON_Object *root_object;
    root_value = json_value_init_object();
    //Waits for a response
    int client= *(int*)socket;
    char buffer[1024]={0};    
   while(1){
    int valread=read(client,buffer, 1024);
    //Read JSON
    root_value=json_parse_string(buffer);
    root_object = json_value_get_object(root_value);
    response=json_object_get_number(root_object, PROTOCOL_PARAMETERS_MSG_TYPE);
        if((response==USER_ASSOCIATION_REQUEST) && (active_users<10)){
            serialized_string=handle_association(response);
            active_users++;
            printf("Active Users: %d\n", active_users);
            send(client,serialized_string, strlen(serialized_string),0);
        }
        else if((response==USER_DESASSOCIATION_REQUEST)&&(active_users>0)){
            serialized_string=handle_desassociation(response);
            active_users--;
            printf("Active Users: %d\n", active_users);
            send(client,serialized_string, strlen(serialized_string),0);
            return NULL;
        }
        else
            return NULL;
        

    
}




    return NULL;

}
void *WaitConnection(void * socket){
    int sk_main= *(int*)socket;
    int new;
    while(1){
    if ((new = accept(sk_main, (struct sockaddr *)&address,(socklen_t*)&addrlen))<0)
    {
        perror("accept");
        exit(EXIT_FAILURE);
    }

    pthread_t pid_new;
    pthread_create(&pid_new, NULL, handler, (void*)&new); 
}
    return NULL;
}

/*void serialization(int msgID) {
    ProtocolMsgType responde= USER_ASSOCIATION_ACCEPTED;
    root_value = json_value_init_object();
    root_object = json_value_get_object(root_value);
    json_object_set_number(root_object, PROTOCOL_PARAMETERS_USER_ID , msgID);
    json_object_set_number(root_object, PROTOCOL_PARAMETERS_MSG_TYPE, responde);

    //json_free_serialized_string(serialized_string);
    //json_value_free(root_value);
    return;
}*/
// =========================================================
//           PUBLIC API
// =========================================================
int initializeFapManagementProtocol()
{
	// TODO: Implement function
	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
      
   if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( SERVER_PORT_NUMBER);
      
  	if (bind(server_fd, (struct sockaddr *)&address, sizeof(address))<0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    pthread_t tid;
    pthread_create(&tid, NULL, WaitConnection, (void *) &server_fd);

    
	return RETURN_VALUE_OK;
}


int terminateFapManagementProtocol()
{
	// TODO: Implement function

	return RETURN_VALUE_OK;
}


int moveFapToGpsNedCoordinates(const GpsNedCoordinates *gpsNedCoordinates)
{
	// TODO: Implement function

	return RETURN_VALUE_OK;
}


int getFapGpsNedCoordinates(GpsNedCoordinates *gpsNedCoordinates)
{
	// TODO: Implement function

	return RETURN_VALUE_OK;
}


int getAllUsersGpsNedCoordinates(GpsNedCoordinates *gpsNedCoordinates, int *n)
{
	// TODO: Implement function

	return RETURN_VALUE_OK;
}

