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
// =========================================================
//           DEFINES
// =========================================================

// ----- MAVLINK PROTOCOL ----- //

// Use MAVLink helper functions
//#define MAVLINK_USE_CONVENIENCE_FUNCTIONS


// ----- FAP MANAGEMENT PROTOCOL - MESSAGES ----- //

#define MAVLINK_USE_CONVENIENCE_FUNCTIONS

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


// ----- FAP MANAGEMENT PROTOCOL - SERVER ADDRESS ----- //
#define SERVER_IP_ADDRESS		"10.0.0.254"
#define SERVER_PORT_NUMBER		40123

static GpsRawCoordinates fapOriginRawCoordinates = {0};


GpsNedCoordinates clients[254];
int server_fd;
struct sockaddr_in address;
int opt = 1;
int alarm_flag=0;
int exit_flag=0;
int addrlen = sizeof(address);
threads_clients threads[11];

pthread_t t_main;
int active_users=0;

// =========================================================
//           FUNCTIONS
// =========================================================

// TODO: Develop the required functions to implement the FAP Management
// Protocol (Server)
int get_free_threads(){
    int i;
    for(i=0; i<11; i++){
        if(threads[i].status==0)
            return i;
    }
    return -1;
}

void * handler_alarm(void *id){
    int user_id= *(int*)id;
    time_t now, past_time;
    struct tm now_t, past;
    printf("Thread id:%d\n", user_id);
    printf("User_ID: %d\n", threads[user_id].user_id);
    char *timestamp=NULL;
    while(threads[user_id].alarm_flag==0)
    {
        if(exit_flag==1)
            break;
       // printf("ALARM CREATED\n");
        if(strcmp(clients[threads[user_id].user_id].timestamp,"")==0)
            continue;
        
        
        
        now=time(&now);
        now_t=*gmtime(&now);
        timestamp=clients[threads[user_id].user_id].timestamp;
        strptime(timestamp,"%Y-%m-%dT%H:%M:%SZ",&past);
        past.tm_isdst=now_t.tm_isdst;
        past_time=mktime(&past);
        if(difftime(now,past_time)>GPS_COORDINATES_UPDATE_TIMEOUT_SECONDS){
            printf("Too much time without updating coordinates, exiting now \n");
            threads[user_id].alarm_flag=1;
        }

    }
    shutdown(threads[user_id].socket, SHUT_RDWR);
    active_users--;
    strcpy(clients[threads[user_id].user_id].timestamp,"");
    return NULL;
}

char * handle_Gps_Update(ProtocolMsgType response, JSON_Value *root){
	char *string=NULL;
    GpsRawCoordinates ClientRawCoordinates = {0};
	JSON_Object *object=json_value_get_object(root);

    //Handle Request
	int user_id=(int)json_object_get_number(object, PROTOCOL_PARAMETERS_USER_ID);
	double latitude=json_object_dotget_number(object, PROTOCOL_PARAMETERS_GPS_COORDINATES "." PROTOCOL_PARAMETERS_GPS_COORDINATES_LAT);
	double longitude=json_object_dotget_number(object, PROTOCOL_PARAMETERS_GPS_COORDINATES "." PROTOCOL_PARAMETERS_GPS_COORDINATES_LON);
	double altitude=json_object_dotget_number(object, PROTOCOL_PARAMETERS_GPS_COORDINATES "." PROTOCOL_PARAMETERS_GPS_COORDINATES_ALT);
    char *Time=json_object_dotget_string(object,PROTOCOL_PARAMETERS_GPS_COORDINATES "." PROTOCOL_PARAMETERS_GPS_COORDINATES_TIMESTAMP);
    initializeGpsRawCoordinates(&ClientRawCoordinates, latitude, longitude, altitude, (time_t )NULL);
    strcpy(ClientRawCoordinates.timestamp,Time);
    gpsRawCoordinates2gpsNedCoordinates(&clients[user_id], &ClientRawCoordinates, &fapOriginRawCoordinates);
    //Create Response
    response=GPS_COORDINATES_ACK;
    root=json_value_init_object();
    object=json_value_get_object(root);
    json_object_set_number(object, PROTOCOL_PARAMETERS_USER_ID , 254);
    json_object_set_number(object, PROTOCOL_PARAMETERS_MSG_TYPE, response);
    strcpyTimestampIso8601(Time,(time(NULL)));
    puts(Time);
    json_object_set_string(object, PROTOCOL_PARAMETERS_GPS_TIMESTAMP, Time);
    string=json_serialize_to_string(root);
    puts(string);
	return string;
}
char * handle_association(ProtocolMsgType response){
    char *string=NULL;
        if(active_users<10){
        active_users++;
        printf("Entrei\n");
        response=USER_ASSOCIATION_ACCEPTED;
    }
    else
        response=USER_ASSOCIATION_REJECTED;
    
    JSON_Value *root_value;
    JSON_Object *root_object;
    root_value = json_value_init_object();
    root_object = json_value_get_object(root_value);
    json_object_set_number(root_object, PROTOCOL_PARAMETERS_USER_ID , 254);
    
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



void * handler (void *status){
    int id= *(int*)status;
    printf("ID: %d\n", id);
    threads[id].status=1;
     ProtocolMsgType response;
    // JSON inicialization 
    JSON_Value *root_value;
    JSON_Object *root_object;
    root_value = json_value_init_object();
    //Waits for a response
    pthread_t alarm;
    char buffer[1024]={0};    
    char *serialized_string = NULL;
   while(threads[id].alarm_flag==0){
    memset(buffer,0,strlen(buffer));
    if((recv(threads[id].socket,buffer, 1024, 0)<=0) &&(threads[id].alarm_flag==0))
    {
        printf("Ending Connection \n");
        break;
    }
    puts(buffer);
    root_value=json_parse_string(buffer);
    root_object = json_value_get_object(root_value);
    response=json_object_get_number(root_object, PROTOCOL_PARAMETERS_MSG_TYPE);
        if(response==USER_ASSOCIATION_REQUEST){
            threads[id].user_id=json_object_get_number(root_object, PROTOCOL_PARAMETERS_USER_ID);
            pthread_create(&alarm, NULL, handler_alarm, (void*)&id);
            serialized_string=handle_association(response);
            printf("Active Users: %d\n", active_users);
            send(threads[id].socket,serialized_string, strlen(serialized_string),0);
        }
        else if(response==GPS_COORDINATES_UPDATE){         
            serialized_string=handle_Gps_Update(response, root_value);
        }
        else if((response==USER_DESASSOCIATION_REQUEST)&&(active_users>0)){
            serialized_string=handle_desassociation(response);
            active_users--;
            printf("Active Users: %d\n", active_users);
            send(threads[id].socket,serialized_string, strlen(serialized_string),0);
            break;
        }
        
        
}

    pthread_join(alarm, NULL);
    close(threads[id].socket);
    threads[id].alarm_flag=0;
    threads[id].user_id=0;
    threads[id].status=0;
    printf("Socket closed\n");
    json_free_serialized_string(serialized_string);
    json_value_free(root_value);



    return NULL;

}
void *WaitConnection(void * socket){
    int sk_main= *(int*)socket;
    while(exit_flag==0){
    int new;
    if ((new = accept(sk_main, (struct sockaddr *)&address,(socklen_t*)&addrlen))<0)
    {
        break;
        perror("accept");
        exit(EXIT_FAILURE);
    }

    int t=get_free_threads();
    printf("AQUII: %d\n",t);
    if(t!=-1){
    threads[t].socket=new;
    pthread_create(&threads[t].tid, NULL, handler, (void*)&t); 
}
}
    return NULL;
}

// =========================================================
//           PUBLIC API
// =========================================================
int initializeFapManagementProtocol()
{

    memset(clients, 0, sizeof(clients));
    memset(&threads, 0 , 11*sizeof(threads_clients));
    initializeMavlink();
    sendMavlinkMsg_gpsGlobalOrigin(&fapOriginRawCoordinates);
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
   // address.sin_addr.s_addr = INADDR_ANY;
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
    pthread_create(&t_main, NULL, WaitConnection, (void *) &server_fd);

    
	return RETURN_VALUE_OK;
}


int terminateFapManagementProtocol()
{
	// TODO: Implement function
    exit_flag=1;
    printf("Exit Flag: %d\n", exit_flag);
    int i=0;
    for(i=0;i<11; i++){
        if(threads[i].status==1){
        shutdown(threads[i].socket, SHUT_RDWR);
        pthread_join(threads[i].tid, NULL);
        printf("Ending threads id: %d\n", i);
    }
}
    shutdown(server_fd,SHUT_RDWR);
    close(server_fd);
    pthread_join(t_main, NULL);
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
    (*n)=0;
    int i=0;
    for(i=0;i<254;i++)
    {
        if(strcmp(clients[i].timestamp,"")!=0){
           
            gpsNedCoordinates[(*n)]=clients[i];
            (*n)++;
        }
    }
	return RETURN_VALUE_OK;
}

