/******************************************************************************
*                         User-Aware Flying AP Project
*                       FAP Management Protocol (Client)
*******************************************************************************
*                        Comunicacoes Moveis 2017/2018
*                             FEUP | MIEEC / MIEIC
*******************************************************************************/

package FapManagementProtocolClient;

import FapManagementProtocolClient.GpsCoordinates;
import FapManagementProtocolClient.ProtocolMsgType;


/**
 * FAP Management Protocol (Client).
 */
public class FapManagementProtocol_Client
{
	// =========================================================
	//           CONSTANTS
	// =========================================================

	// ----- RETURN CODES ----- //
	public static final boolean RETURN_VALUE_OK			= true;
	public static final boolean RETURN_VALUE_ERROR		= false;


	// ----- FAP MANAGEMENT PROTOCOL - MESSAGES ----- //

	// Protocol parameters
	private static final String PROTOCOL_PARAMETERS_USER_ID						= "userId";
	private static final String PROTOCOL_PARAMETERS_MSG_TYPE					= "msgType";
	private static final String PROTOCOL_PARAMETERS_GPS_COORDINATES				= "gpsCoordinates";
	private static final String PROTOCOL_PARAMETERS_GPS_COORDINATES_LAT			= "lat";
	private static final String PROTOCOL_PARAMETERS_GPS_COORDINATES_LON			= "lon";
	private static final String PROTOCOL_PARAMETERS_GPS_COORDINATES_ALT			= "alt";
	private static final String PROTOCOL_PARAMETERS_GPS_COORDINATES_TIMESTAMP	= "timestamp";
	private static final String PROTOCOL_PARAMETERS_GPS_TIMESTAMP				= "gpsTimestamp";

	// User (des)association timeouts (in seconds)
	private static final int USER_ASSOCIATION_TIMEOUT_SECONDS					= 2;
	private static final int USER_DESASSOCIATION_TIMEOUT_SECONDS				= 2;

	// GPS coordinates update period (in seconds)
	private static final int GPS_COORDINATES_UPDATE_PERIOD_SECONDS				= 10;
	private static final int GPS_COORDINATES_UPDATE_TIMEOUT_SECONDS				= (2 * GPS_COORDINATES_UPDATE_PERIOD_SECONDS);


	// ----- FAP MANAGEMENT PROTOCOL - SERVER ADDRESS ----- //
	private static final String SERVER_IP_ADDRESS		= "10.0.0.254";
	private static final int SERVER_PORT_NUMBER			= 40123;


	// =========================================================
	//           PUBLIC API
	// =========================================================

	/**
	 * Request a user association to the FAP.
	 *
	 * @return		True / false if the request was accepted / rejected
	 * 				(considering USER_ASSOCIATION_TIMEOUT_SECONDS).
	 */
	public boolean requestUserAssociation()
	{
		// TODO: Implement function

		return RETURN_VALUE_OK;
	}


	/**
	 * Request a user desassociation from the FAP.
	 *
	 * @return		True / false if the request was / was not ACK by the server
	 * 				(considering USER_DESASSOCIATION_TIMEOUT_SECONDS).
	 */
	public boolean requestUserDesassociation()
	{
		// TODO: Implement function

		return RETURN_VALUE_OK;
	}


	/**
	 * Send the GPS Coordinates to the FAP.
	 *
	 * @param gpsCoordinates	GPS coordinates.
	 * @return					True / false if the GPS coordinates were / were not ACK by
	 * 							the server (considering GPS_COORDINATES_UPDATE_TIMEOUT_SECONDS).
	 */
	public boolean sendGpsCoordinatesToFap(GpsCoordinates gpsCoordinates)
	{
		// TODO: Implement function

		return RETURN_VALUE_OK;
	}


	// =========================================================
	//           PRIVATE FUNCTIONS
	// =========================================================

	// TODO: Develop the required functions to implement the FAP Management
	// Protocol (Client)
}