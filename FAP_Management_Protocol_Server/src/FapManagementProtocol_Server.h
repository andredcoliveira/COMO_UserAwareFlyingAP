/******************************************************************************
*                         User-Aware Flying AP Project
*                       FAP Management Protocol (Server)
*******************************************************************************
*                        Comunicacoes Moveis 2017/2018
*                             FEUP | MIEEC / MIEIC
*******************************************************************************/

#pragma once


// =========================================================
//           DEFINES
// =========================================================

// Return codes
#define RETURN_VALUE_OK				0
#define RETURN_VALUE_ERROR			(-1)

// Size of an ISO8601 timestamp (including '\0')
#define TIMESTAMP_ISO8601_SIZE		21

// Maximum simultaneously associated users
#define MAX_ASSOCIATED_USERS		10


// =========================================================
//           STRUCTS
// =========================================================

/**
 * GPS coordinates in RAW format (lat, lon, alt).
 */
typedef struct _GpsRawCoordinates
{
	float latitude;								// Latitude (in degrees)
	float longitude;							// Longitude (in degrees)
	float altitude;								// Altitude (in meters)
	char timestamp[TIMESTAMP_ISO8601_SIZE];		// Timestamp in ISO8601 format
} GpsRawCoordinates;


/**
 * GPS coordinates in NED format (x, y, z), relative to a given
 * origin coordinates.
 */
typedef struct _GpsNedCoordinates
{
	float x;									// X relative to origin coordinates (in meters)
	float y;									// Y relative to origin coordinates (in meters)
	float z;									// Z relative to origin coordinates (in meters)
	char timestamp[TIMESTAMP_ISO8601_SIZE];		// Timestamp in ISO8601 format
} GpsNedCoordinates;


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