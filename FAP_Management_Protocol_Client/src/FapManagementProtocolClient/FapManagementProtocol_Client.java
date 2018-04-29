/******************************************************************************
*                         User-Aware Flying AP Project
*                       FAP Management Protocol (Client)
*******************************************************************************
*                        Comunicacoes Moveis 2017/2018
*                             FEUP | MIEEC / MIEIC
*******************************************************************************/

package FapManagementProtocolClient;

import com.fasterxml.jackson.core.JsonProcessingException;
import com.fasterxml.jackson.databind.ObjectMapper;
import com.fasterxml.jackson.databind.PropertyNamingStrategy;
import com.fasterxml.jackson.databind.cfg.MapperConfig;
import com.fasterxml.jackson.databind.introspect.AnnotatedMethod;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;
import java.net.*;
import java.nio.charset.StandardCharsets;
import java.time.LocalDateTime;
import java.util.LinkedHashMap;


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
//	private static final String SERVER_IP_ADDRESS		= "10.0.0.254";
	private static final String SERVER_IP_ADDRESS		= "127.0.0.1";
	private static final int SERVER_PORT_NUMBER			= 40123;


	// =========================================================
	//           PUBLIC API
	// =========================================================

	//TODO: check if duplicated code can be reduced

	/**
	 * Constructor.
	 */
	public FapManagementProtocol_Client()
	{
		// TODO: Implement function
	}


	/**
	 * Request a user association to the FAP.
	 *
	 * @return		True / false if the request was accepted / rejected
	 * 				(considering USER_ASSOCIATION_TIMEOUT_SECONDS).
	 */
	public boolean requestUserAssociation() {

		/* Replace with getUserIdExternal() if server address isn't local */
		int userId = getUserIdLocal();
		if(userId < 0)
			return RETURN_VALUE_ERROR;


		/* Create JSON formatted String with data */
		LinkedHashMap<String, Object> data = new LinkedHashMap<>();
		data.put(PROTOCOL_PARAMETERS_USER_ID, userId);
		data.put(PROTOCOL_PARAMETERS_MSG_TYPE, ProtocolMsgType.USER_ASSOCIATION_REQUEST.getMsgTypeValue());

		ObjectMapper objectMapper = new ObjectMapper();
		String msg;
		try {
			msg = objectMapper.writeValueAsString(data);
		} catch (JsonProcessingException e) {
			return RETURN_VALUE_ERROR;
		}

		/* Open socket for communication with server */
		Socket socket = new Socket();
		if(!connectSocket(socket, USER_ASSOCIATION_TIMEOUT_SECONDS))
			return RETURN_VALUE_ERROR;


		/* Send JSON message through socket */
		if(!sendMsg(msg, socket))
			return closeSocket(socket, RETURN_VALUE_ERROR);


		/* Set the timeout value and read response from socket */
		try {
			socket.setSoTimeout(USER_ASSOCIATION_TIMEOUT_SECONDS*1000);
		} catch (SocketException e) {
			return closeSocket(socket, RETURN_VALUE_ERROR);
		}

		BufferedReader in;
		try {
			 in = new BufferedReader(new InputStreamReader(socket.getInputStream()));
		} catch (IOException e) {
			return closeSocket(socket, RETURN_VALUE_ERROR);
		}


		/* Parse response and check its values */
		LinkedHashMap response;
		try {
			response = objectMapper.readValue(in, LinkedHashMap.class);
		} catch (IOException e) {
			return closeSocket(socket, RETURN_VALUE_ERROR);
		}
		int responseId = Integer.parseInt(response.get(PROTOCOL_PARAMETERS_USER_ID).toString());
		int responseMsgType = Integer.parseInt(response.get(PROTOCOL_PARAMETERS_MSG_TYPE).toString());

		if(responseId != userId || responseMsgType != ProtocolMsgType.USER_ASSOCIATION_ACCEPTED.getMsgTypeValue()) {
			return closeSocket(socket, RETURN_VALUE_ERROR);
		}

		/* If the function reached this point, everything must be OK */
		return closeSocket(socket, RETURN_VALUE_OK);
	}


	/**
	 * Request a user dissociation from the FAP.
	 *
	 * @return		True / false if the request was / was not ACK by the server
	 * 				(considering USER_DESASSOCIATION_TIMEOUT_SECONDS).
	 */
	public boolean requestUserDesassociation() {

		/* Replace with getUserIdExternal() if server address isn't local */
		int userId = getUserIdLocal();
		if(userId < 0)
			return RETURN_VALUE_ERROR;


		/* Create JSON formatted String with data */
		LinkedHashMap<String, Object> data = new LinkedHashMap<>();
		data.put(PROTOCOL_PARAMETERS_USER_ID, userId);
		data.put(PROTOCOL_PARAMETERS_MSG_TYPE, ProtocolMsgType.USER_DESASSOCIATION_REQUEST.getMsgTypeValue());

		ObjectMapper objectMapper = new ObjectMapper();
		String msg;
		try {
			msg = objectMapper.writeValueAsString(data);
		} catch (JsonProcessingException e) {
			return RETURN_VALUE_ERROR;
		}


		/* Open socket for communication with server */
		Socket socket = new Socket();
		if(!connectSocket(socket, USER_DESASSOCIATION_TIMEOUT_SECONDS))
			return RETURN_VALUE_ERROR;


		/* Send JSON message through socket */
		if(!sendMsg(msg, socket))
			return closeSocket(socket, RETURN_VALUE_ERROR);


		/* Set the timeout value and read response from socket */
		try {
			socket.setSoTimeout(USER_DESASSOCIATION_TIMEOUT_SECONDS*1000);
		} catch (SocketException e) {
			return closeSocket(socket, RETURN_VALUE_ERROR);
		}

		BufferedReader in;
		try {
			in = new BufferedReader(new InputStreamReader(socket.getInputStream()));
		} catch (IOException e) {
			return closeSocket(socket, RETURN_VALUE_ERROR);
		}


		/* Parse response and check its values */
		LinkedHashMap response;
		try {
			response = objectMapper.readValue(in, LinkedHashMap.class);
		} catch (IOException e) {
			return closeSocket(socket, RETURN_VALUE_ERROR);
		}
		int responseId = Integer.parseInt(response.get(PROTOCOL_PARAMETERS_USER_ID).toString());
		int responseMsgType = Integer.parseInt(response.get(PROTOCOL_PARAMETERS_MSG_TYPE).toString());

		if(responseId != userId || responseMsgType != ProtocolMsgType.USER_DESASSOCIATION_ACK.getMsgTypeValue()) {
			return closeSocket(socket, RETURN_VALUE_ERROR);
		}

		return closeSocket(socket, RETURN_VALUE_OK);
	}


	/**
	 * Send the GPS Coordinates to the FAP.
	 *
	 * @param gpsCoordinates	GPS coordinates.
	 * @return					True / false if the GPS coordinates were / were not ACK by
	 * 							the server (considering GPS_COORDINATES_UPDATE_TIMEOUT_SECONDS).
	 */
	public boolean sendGpsCoordinatesToFap(GpsCoordinates gpsCoordinates) {
		if(gpsCoordinates == null)
			return RETURN_VALUE_ERROR;


		/* Replace with getUserIdExternal() if server address isn't local */
		int userId = getUserIdLocal();
		if(userId < 0)
			return RETURN_VALUE_ERROR;


		/* Create JSON formatted String with data */
		LinkedHashMap<Object, Object> data = new LinkedHashMap<>();
		data.put(PROTOCOL_PARAMETERS_USER_ID, userId);
		data.put(PROTOCOL_PARAMETERS_MSG_TYPE, ProtocolMsgType.GPS_COORDINATES_UPDATE.getMsgTypeValue());
		data.put(PROTOCOL_PARAMETERS_GPS_COORDINATES, gpsCoordinates);

		/* Standardize field names here since GpsCoordinates can't access these private constants
		 * in order to use @SerializedName(<name>) */
		PropertyNamingStrategy customStrategy = new PropertyNamingStrategy(){
			public String nameForGetterMethod(
					MapperConfig<?> config,
					AnnotatedMethod method,
					String defaultName)
			{
				if(defaultName.equals("latitude")) {
					return PROTOCOL_PARAMETERS_GPS_COORDINATES_LAT;
				}
				if(defaultName.equals("longitude")) {
					return PROTOCOL_PARAMETERS_GPS_COORDINATES_LON;
				}
				if(defaultName.equals("altitude")) {
					return PROTOCOL_PARAMETERS_GPS_COORDINATES_ALT;
				}
				return defaultName;
			}
		};

		ObjectMapper objectMapper = new ObjectMapper().setPropertyNamingStrategy(customStrategy);
		String msg;
		try {
			msg = objectMapper.writeValueAsString(data);
		} catch (JsonProcessingException e) {
			return RETURN_VALUE_ERROR;
		}


		/* Open socket for communication with server */
		Socket socket = new Socket();
		if(!connectSocket(socket, GPS_COORDINATES_UPDATE_TIMEOUT_SECONDS))
			return RETURN_VALUE_ERROR;


		/* Send JSON message through socket */
		if(!sendMsg(msg, socket))
			return closeSocket(socket, RETURN_VALUE_ERROR);


		/* Set the timeout value and read response from socket */
		try {
			socket.setSoTimeout(GPS_COORDINATES_UPDATE_TIMEOUT_SECONDS*1000);
		} catch (SocketException e) {
			return closeSocket(socket, RETURN_VALUE_ERROR);
		}

		BufferedReader in;
		try {
			in = new BufferedReader(new InputStreamReader(socket.getInputStream()));
		} catch (IOException e) {
			return closeSocket(socket, RETURN_VALUE_ERROR);
		}


		/* Parse response and check its values */
		LinkedHashMap response = null;
		try {
			response = objectMapper.readValue(in, LinkedHashMap.class);
		} catch (IOException e) {
			closeSocket(socket, RETURN_VALUE_ERROR);
		}

		if(response == null)
			closeSocket(socket, RETURN_VALUE_ERROR);

		int responseId = Integer.parseInt(response.get(PROTOCOL_PARAMETERS_USER_ID).toString());
		int responseMsgType = Integer.parseInt(response.get(PROTOCOL_PARAMETERS_MSG_TYPE).toString());
		String responseTimestamp = response.get(PROTOCOL_PARAMETERS_GPS_TIMESTAMP).toString();

		if(responseId != userId
				|| responseMsgType != ProtocolMsgType.GPS_COORDINATES_ACK.getMsgTypeValue()
				|| !responseTimestamp.equals(gpsCoordinates.toString())) {
			return closeSocket(socket, RETURN_VALUE_ERROR);
		}

		return closeSocket(socket, RETURN_VALUE_OK);
	}


	// =========================================================
	//           PRIVATE FUNCTIONS
	// =========================================================

	// Protocol (Client)

	/**
	 * Get user ID for message trading (LAN)
	 *
	 * @return 			Last octet of user's IP address in LAN
	 */
	private int getUserIdLocal() {
		String ip;

		//Use NetworkInterface.getNetworkInterfaces() and check addresses if more than one network interface

		try {
			ip = InetAddress.getLocalHost().toString();
		} catch (UnknownHostException e) {
			e.printStackTrace();
			return -1;
		}

		return Integer.parseInt(ip.substring(ip.lastIndexOf('.')+1, ip.length()));
	}

	/**
	 * Get user ID for message trading (Internet)
	 *
	 * @return 			Last octet of user's External IP address
	 */
	private int getUserIdExternal() {
		String ip;

		//TODO: add http://icanhazip.com/ and http://www.trackip.net/ip as backup
		URL myExternalIP = null;
		try {
			myExternalIP = new URL("http://checkip.amazonaws.com");
		} catch (MalformedURLException e) {
			e.printStackTrace();
		}
		try {
			if(myExternalIP == null)
				return -1;
			BufferedReader in = new BufferedReader(new InputStreamReader(myExternalIP.openStream()));
			ip = in.readLine();
		} catch (IOException e) {
			return -1;
		}

		return Integer.parseInt(ip.substring(ip.lastIndexOf('.')+1, ip.length()));
	}

	/**
	 * @param msg 		String message to send (pref JSON format)
	 * @param socket 	Socket to write the message into
	 * @return 			True/False in case of success/failure
	 */
	private boolean sendMsg(String msg, Socket socket) {

		try {
			OutputStreamWriter out = new OutputStreamWriter(socket.getOutputStream(), StandardCharsets.UTF_8);
			out.write(msg);
		} catch (IOException e) {
			return false;
		}

		return true;
	}

	/**
	 * @param socket 	Socket to be closed
	 */
	private boolean closeSocket(Socket socket, boolean retval) {
		if(socket != null && socket.isConnected()) {
			try {
				socket.close();
			} catch (IOException e) {
				//no need for catch, function is returning error already
			}
		}

		return retval;
	}

	/**
	 * @param socket 	Socket to connect to
	 * @param timeout 	Timeout value in seconds
	 * @return 			True/False in case of success/failure
	 */
	private boolean connectSocket(Socket socket, int timeout) {

		try {
			socket.connect(
				new InetSocketAddress(SERVER_IP_ADDRESS, SERVER_PORT_NUMBER),
				timeout*1000);
		} catch (IOException e) {
			return false;
		}

		return true;
	}
}