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

#pragma once

// Module headers
#include "GpsCoordinates.h"


// =========================================================
//           DEFINES
// =========================================================

// Return codes
#define RETURN_VALUE_OK				0
#define RETURN_VALUE_ERROR			(-1)


// =========================================================
//           PUBLIC API
// =========================================================

/**
 * Initialize the MAVLink protocol emulator.
 * 
 * @return		Return RETURN_VALUE_OK if there are no errors;
 * 				otherwise, return RETURN_VALUE_ERROR.
 */
int initializeMavlink();

/**
 * Terminate the MAVLink protocol emulator.
 * 
 * @return		Return RETURN_VALUE_OK if there are no errors;
 * 				otherwise, return RETURN_VALUE_ERROR.
 */
int terminateMavlink();


/**
 * Send a MAVLink message to the Autopilot (emulated) - HEARTBEAT.
 * 
 * @return		Return RETURN_VALUE_OK if there are no errors;
 * 				otherwise, return RETURN_VALUE_ERROR.
 */
int sendMavlinkMsg_heartbeat();

/**
 * Send a MAVLink message to the Autopilot (emulated) - LOCAL_POSITION_NED.
 * 
 * @param gpsNedCoordinates 	Pointer to the GPS NED coordinates to be initialized
 * 								with the FAP's GPS NED coordinates.
 * 
 * @return						Return RETURN_VALUE_OK if there are no errors;
 * 								otherwise, return RETURN_VALUE_ERROR.
 */
int sendMavlinkMsg_localPositionNed(GpsNedCoordinates *gpsNedCoordinates);

/**
 * Send a MAVLink message to the Autopilot (emulated) - GPS_GLOBAL_ORIGIN.
 * 
 * @param originRawCoordinates 	Pointer to the GPS RAW coordinates to be initialized
 * 								with the coordinates corresponding to the origin (0,0,0).
 * 
 * @return						Return RETURN_VALUE_OK if there are no errors;
 * 								otherwise, return RETURN_VALUE_ERROR.
 */
int sendMavlinkMsg_gpsGlobalOrigin(GpsRawCoordinates *originRawCoordinates);

/**
 * Send a MAVLink message to the Autopilot (emulated) - SET_POSITION_TARGET_LOCAL_NED.
 * 
 * @param gpsNedCoordinates 	Pointer to the target GPS NED coordinates.
 * 
 * @return						Return RETURN_VALUE_OK if there are no errors;
 * 								otherwise, return RETURN_VALUE_ERROR.
 */
int sendMavlinkMsg_setPositionTargetLocalNed(const GpsNedCoordinates *gpsNedCoordinates);