/******************************************************************************
*                         User-Aware Flying AP Project
*                       FAP Management Protocol (Server)
*******************************************************************************
*                        Comunicacoes Moveis 2017/2018
*                             FEUP | MIEEC / MIEIC
*******************************************************************************/

// =========================================================
// IMPORTANT NOTE:
//
// These files should not be modified.
// =========================================================

// Module headers
#include "MavlinkEmulator.h"
#include "GpsCoordinates.h"

// C headers
#include <stdio.h>
#include <time.h>


// =========================================================
//           DEFINES
// =========================================================

// Default origin coordinates
#define ORIGIN_DEFAULT_RAW_COORDINATES_LATITUDE		41.1779656
#define ORIGIN_DEFAULT_RAW_COORDINATES_LONGITUDE	(-8.5971899)
#define ORIGIN_DEFAULT_RAW_COORDINATES_ALTITUDE		0



// =========================================================
//           GLOBAL VARIABLES
// =========================================================

// Flag indicating if the MAVLink emulator was initialized
static int mavlinkEmulatorInitialized = 0;

// Origin GPS RAW coordinates, cooresponding to (0,0,0)
static GpsRawCoordinates fapOriginRawCoordinates = {0};

// FAP's GPS NED coordinates
static GpsNedCoordinates fapGpsNedCoordinates = {0};


// =========================================================
//           MACROS
// =========================================================

/**
 * Helper function to print a message inside the MAVLink emulator library.
 * 
 * @param ...	Variable arguments to be passed to the printf().
 */
#define MAVLINK_EMULATOR_PRINT(...)                     \
	do                                                  \
	{                                                   \
		printf(">> MavlinkEmulator::%s(): ", __func__); \
		printf(__VA_ARGS__);                            \
		printf("\n");                                   \
	} while (0);



// =========================================================
//           PUBLIC API
// =========================================================
int initializeMavlink()
{
	// Check if the MAVLink emulator was already initialized
	if (mavlinkEmulatorInitialized)
	{
		MAVLINK_EMULATOR_PRINT("ERROR: MAVLink emulator was already initialized");
		return RETURN_VALUE_ERROR;
	}

	// Initialize the FAP's coordinates
	initializeGpsRawCoordinates(&fapOriginRawCoordinates,
								ORIGIN_DEFAULT_RAW_COORDINATES_LATITUDE,
								ORIGIN_DEFAULT_RAW_COORDINATES_LONGITUDE,
								ORIGIN_DEFAULT_RAW_COORDINATES_ALTITUDE,
								time(NULL));

	initializeGpsNedCoordinates(&fapGpsNedCoordinates,
								0,
								0,
								0,
								time(NULL));

	
	// Initialize MAVLink emulator
	mavlinkEmulatorInitialized = 1;
	MAVLINK_EMULATOR_PRINT("MAVLink emulator initialized");

	return RETURN_VALUE_OK;
}


int terminateMavlink()
{
	// Check if the MAVLink emulator is intialized
	if (!mavlinkEmulatorInitialized)
	{
		MAVLINK_EMULATOR_PRINT("ERROR: Trying to terminate MAVLink emulator, but it is not yet initialized");
		return RETURN_VALUE_ERROR;
	}

	// Reset the FAP's coordinates
	initializeGpsRawCoordinates(&fapOriginRawCoordinates,
								ORIGIN_DEFAULT_RAW_COORDINATES_LATITUDE,
								ORIGIN_DEFAULT_RAW_COORDINATES_LONGITUDE,
								ORIGIN_DEFAULT_RAW_COORDINATES_ALTITUDE,
								time(NULL));

	initializeGpsNedCoordinates(&fapGpsNedCoordinates,
								0,
								0,
								0,
								time(NULL));

	// Terminate MAVLink emulator
	mavlinkEmulatorInitialized = 0;
	MAVLINK_EMULATOR_PRINT("MAVLink emulator terminated");

	return RETURN_VALUE_OK;
}


int sendMavlinkMsg_heartbeat()
{
	// Check if the MAVLink emulator is initialized
	if (!mavlinkEmulatorInitialized)
	{
		MAVLINK_EMULATOR_PRINT("ERROR: MAVLink emulator was not yet initialized");
		return RETURN_VALUE_ERROR;
	}

	// Get current time
	char timeStr[TIMESTAMP_ISO8601_SIZE];
	strcpyTimestampIso8601(timeStr, time(NULL));

	// Print
	MAVLINK_EMULATOR_PRINT("Heartbeat received on %s", timeStr);

	return RETURN_VALUE_OK;
}


int sendMavlinkMsg_localPositionNed(GpsNedCoordinates *gpsNedCoordinates)
{
	// Check arguments
	if (gpsNedCoordinates == NULL)
	{
		MAVLINK_EMULATOR_PRINT("ERROR: Invalid pointer");
		return RETURN_VALUE_ERROR;
	}

	// Check if the MAVLink emulator is initialized
	if (!mavlinkEmulatorInitialized)
	{
		MAVLINK_EMULATOR_PRINT("ERROR: MAVLink emulator was not yet initialized");
		return RETURN_VALUE_ERROR;
	}

	// Initialize gpsNedCoordinates with the FAP's current coordinates
	if (copyGpsNedCoordinates(gpsNedCoordinates, &fapGpsNedCoordinates) != RETURN_VALUE_OK)
	{
		MAVLINK_EMULATOR_PRINT("ERROR: Copying GPS NED Coordinates");
		return RETURN_VALUE_ERROR;
	}

	// Print
	MAVLINK_EMULATOR_PRINT("Local position NED:");
	PRINT_GPS_NED_COORDINATES(fapGpsNedCoordinates);

	return RETURN_VALUE_OK;
}


int sendMavlinkMsg_gpsGlobalOrigin(GpsRawCoordinates *originRawCoordinates)
{
	// Check arguments
	if (originRawCoordinates == NULL)
	{
		MAVLINK_EMULATOR_PRINT("ERROR: Invalid pointer");
		return RETURN_VALUE_ERROR;
	}

	// Check if the MAVLink emulator is initialized
	if (!mavlinkEmulatorInitialized)
	{
		MAVLINK_EMULATOR_PRINT("ERROR: MAVLink emulator was not yet initialized");
		return RETURN_VALUE_ERROR;
	}

	// Initialize origin coordinates
	if (copyGpsRawCoordinates(originRawCoordinates, &fapOriginRawCoordinates) != RETURN_VALUE_OK)
	{
		MAVLINK_EMULATOR_PRINT("ERROR: Copying GPS RAW Coordinates");
		return RETURN_VALUE_ERROR;
	}

	// Print
	MAVLINK_EMULATOR_PRINT("GPS global origin:");
	PRINT_GPS_RAW_COORDINATES(fapOriginRawCoordinates);

	return RETURN_VALUE_OK;
}


int sendMavlinkMsg_setPositionTargetLocalNed(const GpsNedCoordinates *gpsNedCoordinates)
{
	// Check arguments
	if (gpsNedCoordinates == NULL)
	{
		MAVLINK_EMULATOR_PRINT("ERROR: Invalid pointer");
		return RETURN_VALUE_ERROR;
	}

	// Check if the MAVLink emulator is initialized
	if (!mavlinkEmulatorInitialized)
	{
		MAVLINK_EMULATOR_PRINT("ERROR: MAVLink emulator was not yet initialized");
		return RETURN_VALUE_ERROR;
	}

	// Update fapGpsNedCoordinates with the provided coordinates
	if (copyGpsNedCoordinates(&fapGpsNedCoordinates, gpsNedCoordinates) != RETURN_VALUE_OK)
	{
		MAVLINK_EMULATOR_PRINT("ERROR: Copying GPS NED Coordinates");
		return RETURN_VALUE_ERROR;
	}

	// Print
	MAVLINK_EMULATOR_PRINT("Set position target local NED:");
	PRINT_GPS_NED_COORDINATES(fapGpsNedCoordinates);

	return RETURN_VALUE_OK;
}