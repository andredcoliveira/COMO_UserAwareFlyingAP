/******************************************************************************
*                         User-Aware Flying AP Project
*                       FAP Management Protocol (Server)
*******************************************************************************
*                        Comunicacoes Moveis 2017/2018
*                             FEUP | MIEEC / MIEIC
*******************************************************************************/

// Module headers
#include "FapManagementProtocol_Server.h"

// MAVLink library
// [https://mavlink.io/en/getting_started/use_source.html]
#include "mavlink/common/mavlink.h"

// JSON parser
// [https://github.com/udp/json-parser and https://github.com/udp/json-builder]
#include "json/json.h"
#include "json/json-builder.h"

// C headers
// (...)


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


// =========================================================
//           FUNCTIONS
// =========================================================

// TODO: Develop the required functions to implement the FAP Management
// Protocol (Server)


// =========================================================
//           PUBLIC API
// =========================================================
int initializeFapManagementProtocol()
{
	// TODO: Implement function

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