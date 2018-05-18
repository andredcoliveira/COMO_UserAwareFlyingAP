/******************************************************************************
 *                         User-Aware Flying AP Project
 *                       FAP Management Protocol (Client)
 *******************************************************************************
 *                        Comunicacoes Moveis 2017/2018
 *                             FEUP | MIEEC / MIEIC
 *******************************************************************************/

package test;

import java.time.LocalDateTime;
import java.util.Random;

import FapManagementProtocolClient.*;
import java.time.ZoneId;


/**
 * FAP Management Protocol (Client) test.
 */
public class Test_FapManagementProtocol_Client
{
	// =========================================================
	//           CONSTANTS
	// =========================================================

	// Limits of the users GPS coordinates
	private static final float USERS_LATITUDE_MIN	= 41.175590f;
	private static final float USERS_LATITUDE_MAX	= 41.180524f;

	private static final float USERS_LONGITUDE_MIN	= -8.601089f;
	private static final float USERS_LONGITUDE_MAX	= -8.594566f;


	// =========================================================
	//           MAIN
	// =========================================================

	/**
	 * Main.
	 */
	public static void main(String[] args)
	{
		System.out.println("=============================================\n" +
			"FAP MANAGEMENT PROTOCOL (CLIENT) TEST\n" +
			"=============================================\n");

		runTests();
	}


	// =========================================================
	//           TESTS
	// =========================================================

	/**
	 * Run tests.
	 */
	private static void runTests()
	{
		int nErrors = 0;

		// Run tests
		nErrors += runTest_fapManagementProtocol();

		if(nErrors == 0) {
			System.out.println("\n# TEST SUMMARY: Tests passed!");
		} else {
			System.out.print("\n# TEST SUMMARY: " + nErrors + " errors.");
			if(nErrors == 2 || nErrors == 3)
				System.out.println(" Check the server log. Coordinates could be out of bounds.");
		}
	}

	/**
	 * Test - FAP Management Protocol.
	 *
	 * @return	Number of errors detected.
	 */
	private static int runTest_fapManagementProtocol()
	{
		System.out.println("==========");
		System.out.println("TEST: FAP Management Protocol (Client)");
		System.out.println("==========");

		int nErrors = 0;

		// Initialize the FAP Management Protocol
		FapManagementProtocol_Client fmp = new FapManagementProtocol_Client();

		// Request user association
		nErrors += assertCondition(fmp.requestUserAssociation() == FapManagementProtocol_Client.RETURN_VALUE_OK,
			"Requesting user association");

		// Send coordinates (repeat 2 times)
		Random random = new Random();

		for (int i = 0; i < 2; i++)
		{
			// Generate random GPS coordinates (within the specified limit)
			float latitude = USERS_LATITUDE_MIN + random.nextFloat() * (USERS_LATITUDE_MAX - USERS_LATITUDE_MIN);
			float longitude = USERS_LONGITUDE_MIN + random.nextFloat() * (USERS_LONGITUDE_MAX - USERS_LONGITUDE_MIN);

			GpsCoordinates gpsCoordinates = new GpsCoordinates(latitude, longitude, 0, LocalDateTime.now(ZoneId.of("Z")));

			// Send GPS coordinates
			nErrors += assertCondition(fmp.sendGpsCoordinatesToFap(gpsCoordinates) == FapManagementProtocol_Client.RETURN_VALUE_OK,
				"Sending GPS Coordinates");
		}


		// Terminate the FAP Management Protocol
		nErrors += assertCondition(fmp.requestUserDesassociation() == FapManagementProtocol_Client.RETURN_VALUE_OK,
			"Requesting user desassociation");

		return nErrors;
	}


	// =========================================================
	//           AUXILIARY FUNCTIONS
	// =========================================================

	/**
	 * Assert if a condition is true, printing a message if it is not.
	 *
	 * @param condition		Condition to be tested.
	 * @param errorMsg		Message to be displayed if the condition is not valid.
	 *
	 * @return				Number of errors detected.
	 */
	private static int assertCondition(boolean condition, String errorMsg)
	{
		if (!condition)
		{
			System.out.println("ERROR: " + errorMsg);
			return 1;
		}
		else
		{
			return 0;
		}
	}
}