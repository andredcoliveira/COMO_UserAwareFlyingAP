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

// C headers
#include <time.h>


// =========================================================
//           DEFINES
// =========================================================

// Return codes
#define RETURN_VALUE_OK				0
#define RETURN_VALUE_ERROR			(-1)

// Size of an ISO8601 timestamp (including '\0')
#define TIMESTAMP_ISO8601_SIZE		21


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
//           PUBLIC MACROS
// =========================================================

/**
 * Print GPS RAW coordinates.
 * 
 * @param gpsRawCoordinates		GPS RAW coordinates.
 */
#define PRINT_GPS_RAW_COORDINATES(gpsRawCoordinates)	\
	printf("\tlat: %f\n\tlon: %f\n\talt: %f\n\ttimestamp: %s\n\n",	\
		   gpsRawCoordinates.latitude, gpsRawCoordinates.longitude, gpsRawCoordinates.altitude, gpsRawCoordinates.timestamp);

/**
 * Print GPS NED coordinates.
 * 
 * @param gpsNedCoordinates		GPS NED coordinates.
 */
#define PRINT_GPS_NED_COORDINATES(gpsNedCoordinates)	\
	printf("\tx: %f\n\ty: %f\n\tz: %f\n\ttimestamp: %s\n\n",	\
		   gpsNedCoordinates.x, gpsNedCoordinates.y, gpsNedCoordinates.z, gpsNedCoordinates.timestamp);


// =========================================================
//           PUBLIC API
// =========================================================

/**
 * Initialize a GpsRawCoordinates structure.
 * 
 * @param gpsRawCoordinates		Pointer to the GpsRawCoordinates to be initialized.
 * @param latitude				Latitude (in degrees).
 * @param longitude				Longitude (in degrees).
 * @param altitude				Altitude (in meters).
 * @param timestamp				Timestamp.
 * @return						Return RETURN_VALUE_OK if there are no errors;
 *								otherwise, return RETURN_VALUE_ERROR.
 */
int initializeGpsRawCoordinates(GpsRawCoordinates *gpsRawCoordinates, float latitude, float longitude, float altitude, time_t timestamp);

/**
 * Initialize a GpsNedCoordinates structure.
 * 
 * @param gpsNedCoordinates		Pointer to the GpsNedCoordinates to be initialized.
 * @param x						X (in meters).
 * @param y						Y (in meters).
 * @param z						Z (in meters).
 * @param timestamp				Timestamp.
 * @return						Return RETURN_VALUE_OK if there are no errors;
 *								otherwise, return RETURN_VALUE_ERROR.
 */
int initializeGpsNedCoordinates(GpsNedCoordinates *gpsNedCoordinates, float x, float y, float z, time_t timestamp);

/**
 * Copy a GPS RAW coordinates structure to another.
 * 
 * @param dst		Destination GpsRawCoordinates to be filled.
 * @param src		Source GpsRawCoordinates.
 * @return			Return RETURN_VALUE_OK if there are no errors;
 *					otherwise, return RETURN_VALUE_ERROR.
 */
int copyGpsRawCoordinates(GpsRawCoordinates *dst, const GpsRawCoordinates *src);

/**
 * Copy a GPS NED coordinates structure to another.
 * 
 * @param dst		Destination GpsNedCoordinates to be filled.
 * @param src		Source GpsNedCoordinates.
 * @return			Return RETURN_VALUE_OK if there are no errors;
 *					otherwise, return RETURN_VALUE_ERROR.
 */
int copyGpsNedCoordinates(GpsNedCoordinates *dst, const GpsNedCoordinates *src);

/**
 * Check if two GPS RAW coordinates are equal (including the timestamp).
 * 
 * @param gnc1 		GPS RAW coordinates 1.
 * @param gnc2 		GPS RAW coordinates 2.
 * @return			True (1) if both GPS RAW coordinates are the same;
 * 					return False (0) otherwise.
 */
int areGpsRawCoordinatesEqual(const GpsRawCoordinates *grc1, const GpsRawCoordinates *grc2);

/**
 * Check if two GPS NED coordinates are equal (including the timestamp).
 * 
 * @param gnc1 		GPS NED coordinates 1.
 * @param gnc2 		GPS NED coordinates 2.
 * @return			True (1) if both GPS NED coordinates are the same;
 * 					return False (0) otherwise.
 */
int areGpsNedCoordinatesEqual(const GpsNedCoordinates *gnc1, const GpsNedCoordinates *gnc2);

/**
 * Convert a set of coordinates from RAW to NED format.
 * 
 * @param gpsNedCoordinates		Pointer to the GpsNedCoordinates to be initialized with the converted
 * 								GpsRawCoordinates.
 * @param gpsRawCoordinates		Pointer to the source GpsRawCoordinates to be converted.
 * @param originRawCoordinates	Pointer to the origin coordinates corresponding to (0,0,0).
 * @return						Return RETURN_VALUE_OK if there are no errors;
 *								otherwise, return RETURN_VALUE_ERROR.
 */
int gpsRawCoordinates2gpsNedCoordinates(GpsNedCoordinates *gpsNedCoordinates, const GpsRawCoordinates *gpsRawCoordinates, const GpsRawCoordinates *originRawCoordinates);

/**
 * Convert a set of coordinates from NED to RAW format.
 * 
 * @param gpsRawCoordinates		Pointer to the GpsRawCoordinates to be initialized with the converted
 * 								GpsNedCoordinates.
 * @param gpsNedCoordinates		Pointer to the source GpsNedCoordinates to be converted.
 * @param originRawCoordinates	Pointer to the origin coordinates corresponding to (0,0,0).
 * @return						Return RETURN_VALUE_OK if there are no errors;
 *								otherwise, return RETURN_VALUE_ERROR.
 */
//int gpsNedCoordinates2gpsRawCoordinates(GpsRawCoordinates *gpsRawCoordinates, const GpsNedCoordinates *gpsNedCoordinates, const GpsRawCoordinates *originRawCoordinates);

/**
 * Format a string with a given timestamp in ISO8601 format
 * 
 * @param destStr		Destination string to be filled with the timestamp (ISO 8601 format).
 * 						The pointer must have sufficient memory to fill the string
 * 						[TIMESTAMP_ISO8601_SIZE = 21].
 * @param timestamp		Timestamp.
 * @return				Return RETURN_VALUE_OK if there are no errors;
 *						otherwise, return RETURN_VALUE_ERROR.
 */
int strcpyTimestampIso8601(char *destStr, time_t timestamp);