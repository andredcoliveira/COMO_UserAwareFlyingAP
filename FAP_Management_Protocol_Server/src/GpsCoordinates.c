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
#include "GpsCoordinates.h"

// C headers
#include <string.h>
#include <time.h>
#include <math.h>
#include <stdio.h>

// =========================================================
//           DEFINES
// =========================================================

// Earth's semi-axis
#define EARTH_SEMI_MAJOR_AXIS	6378137.0f
#define EARTH_SEMI_MINOR_AXIS	6356752.314245f


// =========================================================
//           AUXILIARY FUNCTIONS
// =========================================================

/**
 * Determine the Earth's radius at a given latitude.
 * 
 * @param latRadians 	Latitude (in radians).
 * @return				Earth's radius at the given latitude.
 */
float earthRadiusAtLatitude(float latRadians)
{
	return sqrt((pow(EARTH_SEMI_MAJOR_AXIS * EARTH_SEMI_MAJOR_AXIS * cos(latRadians), 2) +
					pow(EARTH_SEMI_MINOR_AXIS * EARTH_SEMI_MINOR_AXIS * sin(latRadians), 2)) /
				(pow(EARTH_SEMI_MAJOR_AXIS * cos(latRadians), 2) +
					pow(EARTH_SEMI_MINOR_AXIS * sin(latRadians), 2)));
}


/**
 * Convert a set of GPS RAW coordinates to NED, considering the Earth's center as the origin.
 * 
 * @param gpsNedCoordinates		Pointer to the GpsNedCoordinates to be initialized with the converted
 * 								GpsRawCoordinates.
 * @param gpsRawCoordinates		Pointer to the source GpsRawCoordinates to be converted.
 * @param earthRadius			Earth's radius at the given latitude.
 * @return						Return RETURN_VALUE_OK if there are no errors;
 *								otherwise, return RETURN_VALUE_ERROR.
 */
int gpsRawCoordinates2gpsNedCoordinates_EarthOrigin(GpsNedCoordinates *gpsNedCoordinates, const GpsRawCoordinates *gpsRawCoordinates, float earthRadius)
{
	// Check arguments
	if (gpsNedCoordinates == NULL || gpsRawCoordinates == NULL)
		return RETURN_VALUE_ERROR;

	// Convert RAW coordinates to NED
	gpsNedCoordinates->x = earthRadius * sin(gpsRawCoordinates->latitude * M_PI / 180) * cos(gpsRawCoordinates->longitude * M_PI / 180);
	gpsNedCoordinates->y = earthRadius * sin(gpsRawCoordinates->latitude * M_PI / 180) * sin(gpsRawCoordinates->longitude * M_PI / 180);
	gpsNedCoordinates->z = gpsRawCoordinates->altitude;
	strcpy(gpsNedCoordinates->timestamp, gpsRawCoordinates->timestamp);

	return RETURN_VALUE_OK;
}


// =========================================================
//           PUBLIC API
// =========================================================
int initializeGpsRawCoordinates(GpsRawCoordinates *gpsRawCoordinates, float latitude, float longitude, float altitude, time_t timestamp)
{
	// Check arguments
	if (gpsRawCoordinates == NULL)
		return RETURN_VALUE_ERROR;

	// Initialize structure
	gpsRawCoordinates->latitude = latitude;
	gpsRawCoordinates->longitude = longitude;
	gpsRawCoordinates->altitude = altitude;
	strcpyTimestampIso8601(gpsRawCoordinates->timestamp, timestamp);

	return RETURN_VALUE_OK;
}


int initializeGpsNedCoordinates(GpsNedCoordinates *gpsNedCoordinates, float x, float y, float z, time_t timestamp)
{
	// Check arguments
	if (gpsNedCoordinates == NULL)
		return RETURN_VALUE_ERROR;

	// Initialize structure
	gpsNedCoordinates->x = x;
	gpsNedCoordinates->y = y;
	gpsNedCoordinates->z = z;
	strcpyTimestampIso8601(gpsNedCoordinates->timestamp, timestamp);

	return RETURN_VALUE_OK;
}

int copyGpsRawCoordinates(GpsRawCoordinates *dst, const GpsRawCoordinates *src)
{
	// Check arguments
	if (dst == NULL || src == NULL)
		return RETURN_VALUE_ERROR;

	// Copy GPS RAW coordinates
	dst->latitude = src->latitude;
	dst->longitude = src->longitude;
	dst->altitude = src->altitude;
	strcpy(dst->timestamp, src->timestamp);

	return RETURN_VALUE_OK;
}


int copyGpsNedCoordinates(GpsNedCoordinates *dst, const GpsNedCoordinates *src)
{
	// Check arguments
	if (dst == NULL || src == NULL)
		return RETURN_VALUE_ERROR;

	// Copy GPS NED coordinates
	dst->x = src->x;
	dst->y = src->y;
	dst->z = src->z;
	strcpy(dst->timestamp, src->timestamp);

	return RETURN_VALUE_OK;
}


int areGpsRawCoordinatesEqual(const GpsRawCoordinates *grc1, const GpsRawCoordinates *grc2)
{
	return ((grc1->latitude == grc2->latitude) &&
			(grc1->longitude == grc2->longitude) &&
			(grc1->altitude == grc2->altitude));// &&
			//(strcmp(grc1->timestamp, grc2->timestamp) == 0));
}


int areGpsNedCoordinatesEqual(const GpsNedCoordinates *gnc1, const GpsNedCoordinates *gnc2)
{
	return ((gnc1->x == gnc2->x) &&
			(gnc1->y == gnc2->y) &&
			(gnc1->z == gnc2->z)); // &&
			//(strcmp(gnc1->timestamp, gnc2->timestamp) == 0));
}


int gpsRawCoordinates2gpsNedCoordinates(GpsNedCoordinates *gpsNedCoordinates, const GpsRawCoordinates *gpsRawCoordinates, const GpsRawCoordinates *originRawCoordinates)
{
	// Check arguments
	if (gpsNedCoordinates == NULL || gpsRawCoordinates == NULL || originRawCoordinates == NULL)
		return RETURN_VALUE_ERROR;

	// Obtain Earth's radius at the origin's latitude
	float earthRadius = earthRadiusAtLatitude(originRawCoordinates->latitude);

	// Convert origin RAW coordinates to NED
	GpsNedCoordinates originNedCoordinates;
	gpsRawCoordinates2gpsNedCoordinates_EarthOrigin(&originNedCoordinates, originRawCoordinates, earthRadius);

	// Convert target RAW coordinates to NED
	GpsNedCoordinates targetNedCoordinates;
	gpsRawCoordinates2gpsNedCoordinates_EarthOrigin(&targetNedCoordinates, gpsRawCoordinates, earthRadius);

	// Convert absolute NED coordinates to relative (considering the origin coordinates)
	gpsNedCoordinates->x = targetNedCoordinates.x - originNedCoordinates.x;
	gpsNedCoordinates->y = targetNedCoordinates.y - originNedCoordinates.y;
	gpsNedCoordinates->z = targetNedCoordinates.z - originNedCoordinates.z;
	strcpy(gpsNedCoordinates->timestamp, targetNedCoordinates.timestamp);


	return RETURN_VALUE_OK;
}


//int gpsNedCoordinates2gpsRawCoordinates(GpsRawCoordinates *gpsRawCoordinates, const GpsNedCoordinates *gpsNedCoordinates, const GpsRawCoordinates *originRawCoordinates)
//{
//	return RETURN_VALUE_OK;
//}


int strcpyTimestampIso8601(char *destStr, time_t timestamp)
{
	strftime(destStr, TIMESTAMP_ISO8601_SIZE, "%Y-%m-%dT%H:%M:%SZ", gmtime(&timestamp));

	return RETURN_VALUE_OK;
}