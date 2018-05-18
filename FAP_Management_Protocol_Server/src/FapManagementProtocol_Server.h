/******************************************************************************
*                         User-Aware Flying AP Project
*                       FAP Management Protocol (Server)
*******************************************************************************
*                        Comunicacoes Moveis 2017/2018
*                             FEUP | MIEEC / MIEIC
*******************************************************************************/

#include <pthread.h>
#pragma once
// Module headers
#include "GpsCoordinates.h"



// =========================================================
//           DEFINES

#define RED		"\x1B[31m"
#define YEL		"\x1B[33m"
#define BLUE	"\x1B[34m"
#define RESET	"\x1B[0m"

#define FAP_SERVER_PRINT(...)                                             \
	do                                                                    \
	{                                                                     \
		printf(BLUE ">> FapManagementProtocol_Server::%s(): ", __func__); \
		printf(BLUE __VA_ARGS__);                                         \
		printf(RESET "\n");                                               \
	} while (0);

#define FAP_SERVER_PRINT_ERROR(...)                                       \
	do                                                                    \
	{                                                                     \
		printf(RED ">> FapManagementProtocol_Server::%s(): ", __func__);  \
		printf(RED __VA_ARGS__);                                          \
		printf(RESET "\n");                                               \
	} while (0);


// =========================================================
//           DEFINES
// =========================================================

// Return codes
#define RETURN_VALUE_OK				0
#define RETURN_VALUE_ERROR			(-1)

// Maximum simultaneously associated users
#define MAX_ASSOCIATED_USERS		10
#define SO_MAX_CONN					MAX_ASSOCIATED_USERS


// =========================================================

typedef struct _threads_clients
{
	pthread_t tid;
	int       socket;
	int 	  status;
	int 	  user_id;
	int 	  alarm_flag;
} threads_clients;

//           STRUCTS
// =========================================================

/**
 * GPS coordinates in RAW format (lat, lon, alt).
 */


// =========================================================
//           PUBLIC API
// =========================================================

/**
 * Initialize the FAP Management Protocol operation.
 * This function should initialize the FAP Management Protocol's internal components,
 * namely the server socket.
 * 
 * @return		Return RETURN_VALUE_OK if there are no errors;
 * 				otherwise, return RETURN_VALUE_ERROR.
 */
int initializeFapManagementProtocol();

/**
 * Terminate the FAP Management Protocol operation.
 * This function should terminate the FAP Management Protocol's operation,
 * namely close and dispose the server socket.
 * 
 * @return		Return RETURN_VALUE_OK if there are no errors;
 * 				otherwise, return RETURN_VALUE_ERROR.
 */
int terminateFapManagementProtocol();

/**
 * Move the FAP to the target GPS Coordinates (in NED format).
 * Note: This function only needs to send the appropriate MAVLink message
 * to the Autopilot.
 *
 * @param gpsNedCoordinates		Pointer to the target GPS coordinates (in NED format).
 * @return int 					Return RETURN_VALUE_OK if there are no errors;
 * 								otherwise, return RETURN_VALUE_ERROR.
 */
int moveFapToGpsNedCoordinates(const GpsNedCoordinates *gpsNedCoordinates);

/**
 * Get the FAP's current GPS coordinates (in NED format).
 * Note: This function only needs to send the appropriate MAVLink message
 * to the Autopilot.
 *
 * @param gpsNedCoordinates 	Pointer to the GPS coordinates to be initialized
 * 								with the FAP's GPS coordinates.
 * @return int 					Return RETURN_VALUE_OK if there are no errors;
 * 								otherwise, return RETURN_VALUE_ERROR.
 */
int getFapGpsNedCoordinates(GpsNedCoordinates *gpsNedCoordinates);

/**
 * Get the GPS coordinates (in NED format) of all associated users.
 *
 * Note 1: The GPS coordinates received from the users are in RAW format.
 * This function needs to convert them to NED format.
 *
 * Note 2: The array pointed by the gpsNedCoordinates pointer must be able to
 * accommodate the maximum number of associated users (i.e., MAX_ASSOCIATED_USERS).
 * The function will return the number of associated users (i.e., the number of elements
 * in the array) through the pointer *n.
 *
 * @param gpsNedCoordinates 	Pointer to an array of GpsNedCoordinates to be
 * 								initialized with the users' GPS coordinates.
 * @param n 					Pointer to be initialized with the number of
 * 								associated users.
 * @return int 					Return RETURN_VALUE_OK if there are no errors;
 * 								otherwise, return RETURN_VALUE_ERROR.
 */
int getAllUsersGpsNedCoordinates(GpsNedCoordinates *gpsNedCoordinates, int *n);