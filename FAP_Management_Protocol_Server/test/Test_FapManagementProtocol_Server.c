/******************************************************************************
*                         User-Aware Flying AP Project
*                       FAP Management Protocol (Server)
*******************************************************************************
*                        Comunicacoes Moveis 2017/2018
*                             FEUP | MIEEC / MIEIC
*******************************************************************************/

// Module headers
#include "FapManagementProtocol_Server.h"

// C headers
#include <stdio.h>
#include <time.h>


// =========================================================
//           DEFINES
// =========================================================

// ----- FAP MANAGEMENT PROTOCOL - PARAMETERS ----- //

// GPS coordinates update period (in seconds)
#define GPS_COORDINATES_UPDATE_PERIOD_SECONDS		10
#define GPS_COORDINATES_UPDATE_TIMEOUT_SECONDS		(2 * GPS_COORDINATES_UPDATE_PERIOD_SECONDS)

// Max allowed distance from the users to the FAP (in meters)
#define MAX_ALLOWED_DISTANCE_FROM_FAP_METERS		300


// =========================================================
//           MACROS
// =========================================================

/**
 * Print a message.
 * 
 * @param ...		Variable arguments to be passed to the printf().
 */
#define TEST_PRINT(...)      \
	do                       \
	{                        \
		printf(">> TEST: "); \
		printf(__VA_ARGS__); \
		printf("\n");        \
	} while (0);

/**
 * Assert if a condition is true, printing a message if it is not.
 *
 * @param condition		Condition to be tested.
 * @param errorMsg		Message to be displayed if the condition is not valid.
 */
#define ASSERT_CONDITION(condition, errorMsg, nErrors) \
	do                                                 \
	{                                                  \
		if (!(condition))                              \
		{                                              \
			TEST_PRINT("ERROR: %s\n", errorMsg);       \
			nErrors++;                                 \
		}                                              \
	} while (0);

/**
 * Print the test header.
 */
#define PRINT_TEST_HEADER()  \
	printf("==========\n"    \
		   "TEST: %s\n"      \
		   "==========\n\n", __func__);

/**
 * Print a test summary.
 *
 * @param nErrors	Number of errors detected.
 */
#define PRINT_TEST_SUMMARY(nErrors)                                      \
	do                                                                   \
	{                                                                    \
		if (nErrors > 0)                                                 \
			printf("\n# TEST SUMMARY: %d Errors detected\n\n", nErrors); \
		else                                                             \
			printf("\n# TEST SUMMARY: Tests passed!\n\n");               \
	} while (0);


// =========================================================
//           AUXILIARY FUNCTIONS
// =========================================================

/**
 * Sleep the program for a given period of time.
 * Note: the program may be interrupted by a SIGALARM,
 * but will resume its sleep after the interruption.
 * 
 * @param seconds		Time to sleep (in seconds).
 */
void sleepProgram(unsigned int seconds)
{
	struct timespec req = {seconds, 0};
	struct timespec rem = {0, 0};

	// Sleep the entire time
	while (nanosleep(&req, &rem) < 0)
	{
		// Resume sleeping
		req.tv_sec = rem.tv_sec;
		req.tv_nsec = rem.tv_nsec;
	}
}


// =========================================================
//           TESTS
// =========================================================

/**
 * Test - FAP Management Protocol.
 * 
 * @return		The number of errors detected.
 */
int runTest_fapManagementProtocol()
{
	PRINT_TEST_HEADER();

	int nErrors = 0;

	// Initialize FAP Management Protocol
	ASSERT_CONDITION(initializeFapManagementProtocol() == RETURN_VALUE_OK,
					 "Initializing FAP Management Protocol",
					 nErrors);
	

	// Move FAP and check if the FAP correctly updated its position
	sleepProgram(1); // Let some time pass

	GpsNedCoordinates fapGpsNedCoordinates;
	initializeGpsNedCoordinates(&fapGpsNedCoordinates,
								15, 20, 10, time(NULL));

	ASSERT_CONDITION(moveFapToGpsNedCoordinates(&fapGpsNedCoordinates) == RETURN_VALUE_OK,
					 "Moving FAP",
					 nErrors);

	GpsNedCoordinates newFapGpsNedCoordinates;
	ASSERT_CONDITION(getFapGpsNedCoordinates(&newFapGpsNedCoordinates) == RETURN_VALUE_OK,
					 "Getting FAP Coordinates",
					 nErrors);

	ASSERT_CONDITION(areGpsNedCoordinatesEqual(&fapGpsNedCoordinates, &newFapGpsNedCoordinates),
					 "FAP's coordinates were not updated",
					 nErrors);


	// Terminate FAP Management Protocol
	sleepProgram(1); // Let some time pass

	ASSERT_CONDITION(terminateFapManagementProtocol() == RETURN_VALUE_OK,
					 "Terminating the FAP Management Protocol",
					 nErrors);


	// Finish test
	PRINT_TEST_SUMMARY(nErrors);

	return nErrors;
}

/**
 * Test - Get all users GPS NED Coordinates.
 * 
 * @return		The number of errors detected.
 */
int runTest_getAllUsersGpsNedCoordinates()
{
	PRINT_TEST_HEADER();

	int nErrors = 0;


	// Initialize the FAP Management Protocol
	ASSERT_CONDITION(initializeFapManagementProtocol() == RETURN_VALUE_OK,
					 "Initializing the FAP Management Protocol",
					 nErrors);

	// Repeat this test 3 times
	for (int i = 0; i < 3; i++)
	{
		// Wait for the users to associate and send their GPS coordinates
		sleepProgram(GPS_COORDINATES_UPDATE_PERIOD_SECONDS);

		// Get all users coordinates
		GpsNedCoordinates usersGpsNedCoordinates[MAX_ASSOCIATED_USERS];
		int nUsers;

		ASSERT_CONDITION(getAllUsersGpsNedCoordinates(usersGpsNedCoordinates, &nUsers) == RETURN_VALUE_OK,
						 "Getting all users coordinates",
						 nErrors);

		TEST_PRINT("Users coordinates (%d users associated):", nUsers);
		
		for (int n = 0; n < nUsers; n++)
			PRINT_GPS_NED_COORDINATES(usersGpsNedCoordinates[n]);

		printf("\n");
	}

	// Terminate the FAP Management Protocol
	ASSERT_CONDITION(terminateFapManagementProtocol() == RETURN_VALUE_OK,
					 "Terminating the FAP Management Protocol",
					 nErrors);

	// Print test summary
	PRINT_TEST_SUMMARY(nErrors);

	return nErrors;
}

/**
 * Run all tests.
 */
void runTests()
{
	int nErrors = 0;

	// Run tests
	nErrors += runTest_fapManagementProtocol();
	nErrors += runTest_getAllUsersGpsNedCoordinates();
}

// =========================================================
//           MAIN
// =========================================================

/**
 * Main.
 */
int main()
{
	printf("=============================================\n"
		   "FAP MANAGEMENT PROTOCOL (SERVER) TEST\n"
		   "=============================================\n");

	runTests();

	return 0;
}