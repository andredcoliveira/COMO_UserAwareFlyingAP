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
import com.fasterxml.jackson.databind.SerializationFeature;

import java.io.*;
import java.net.*;
import java.time.ZoneOffset;
import java.util.LinkedHashMap;
import java.util.concurrent.TimeUnit;

import static com.fasterxml.jackson.core.JsonParser.Feature.AUTO_CLOSE_SOURCE;


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
	//           Class Parameters
	// =========================================================

	private Socket socket;
	private ObjectMapper objectMapper;
	private int userId;
	private BufferedReader in;
	private PrintWriter out;

	// =========================================================
	//           PUBLIC API
	// =========================================================

	/**
	 * Constructor.
	 */
	public FapManagementProtocol_Client() {
		/* Create an unconnected socket */
		this.socket = new Socket();

		/* JSON Object Mapper that keeps socket alive and pretty prints JSON */
		this.objectMapper = new ObjectMapper();
		objectMapper.configure(AUTO_CLOSE_SOURCE, false);
		objectMapper.configure(SerializationFeature.INDENT_OUTPUT, true);

		/* Replace with getUserIdExternal() if server address isn't local */
		this.userId = getUserIdLocal();

		if(this.userId < 0)
			prettyPrint(null, "Ready");
		else
			prettyPrint(null, "User ID " + this.userId + ": Ready");
	}


	/**
	 * Request a user association to the FAP.
	 *
	 * @return		True / false if the request was accepted / rejected
	 * 				(considering USER_ASSOCIATION_TIMEOUT_SECONDS).
	 */
	public boolean requestUserAssociation() {

		if(this.userId < 0) {
			if((this.userId = getUserIdLocal()) < 0)
				return RETURN_VALUE_ERROR;
		}
		if(this.objectMapper == null) {
			this.objectMapper = new ObjectMapper();
			objectMapper.configure(AUTO_CLOSE_SOURCE, false);
			objectMapper.configure(SerializationFeature.INDENT_OUTPUT, true);
		}


		/* Create JSON formatted String with data */
		LinkedHashMap<String, Object> data = new LinkedHashMap<>();
		data.put(PROTOCOL_PARAMETERS_USER_ID, this.userId);
		data.put(PROTOCOL_PARAMETERS_MSG_TYPE, ProtocolMsgType.USER_ASSOCIATION_REQUEST.getMsgTypeValue());

		String msg;
		try {
			msg = this.objectMapper.writeValueAsString(data);
		} catch (JsonProcessingException e) {
			return RETURN_VALUE_ERROR;
		}


		/* Open socket for communication with server */
		if(!connectSocket(this.socket, USER_ASSOCIATION_TIMEOUT_SECONDS)) {
			prettyPrint("requestUserAssociation", "Couldn't establish a connection to the Server");
			return RETURN_VALUE_ERROR;
		}

		prettyPrint("requestUserAssociation", "Socket ready");


		/* Create output stream */
		try {
			this.out = new PrintWriter(this.socket.getOutputStream(), true);
		} catch (IOException e) {
			return closeSocket(this.socket, RETURN_VALUE_ERROR);
		}

		prettyPrint("requestUserAssociation", "Trying to associate...");


		/* Send JSON message through socket */
		if(!sendMsg(msg))
			return closeSocket(this.socket, RETURN_VALUE_ERROR);


		/* Set the timeout value and read response from socket */
		try {
			this.socket.setSoTimeout(USER_ASSOCIATION_TIMEOUT_SECONDS*1000);
		} catch (SocketException e) {
			return closeSocket(this.socket, RETURN_VALUE_ERROR);
		}


		try {
			TimeUnit.NANOSECONDS.sleep(100);
		} catch (InterruptedException e) {
			closeSocket(this.socket, RETURN_VALUE_ERROR);
		}


		try {
			 this.in = new BufferedReader(new InputStreamReader(this.socket.getInputStream()));
		} catch (IOException e) {
			return closeSocket(this.socket, RETURN_VALUE_ERROR);
		}


		/* Parse response and check its values */
		LinkedHashMap response;
		try {
			response = this.objectMapper.readValue(this.in, LinkedHashMap.class);
		} catch (Exception e) {
			return closeSocket(this.socket, RETURN_VALUE_ERROR);
		}

		int responseId = Integer.parseInt(response.get(PROTOCOL_PARAMETERS_USER_ID).toString());
		int responseMsgType = Integer.parseInt(response.get(PROTOCOL_PARAMETERS_MSG_TYPE).toString());

		if(responseId != this.userId || responseMsgType != ProtocolMsgType.USER_ASSOCIATION_ACCEPTED.getMsgTypeValue()) {
			return closeSocket(this.socket, RETURN_VALUE_ERROR);
		}

		prettyPrint("requestUserAssociation", "Associated");


		/* If the function reached this point, everything must be OK */
		return RETURN_VALUE_OK;
	}


	/**
	 * Request a user dissociation from the FAP.
	 *
	 * @return		True / false if the request was / was not ACK by the server
	 * 				(considering USER_DESASSOCIATION_TIMEOUT_SECONDS).
	 */
	public boolean requestUserDesassociation() {
		if(this.userId < 0) {
			if((this.userId = getUserIdLocal()) < 0)
				return RETURN_VALUE_ERROR;
		}
		if(this.objectMapper == null) {
			this.objectMapper = new ObjectMapper();
			objectMapper.configure(AUTO_CLOSE_SOURCE, false);
			objectMapper.configure(SerializationFeature.INDENT_OUTPUT, true);
		}


		/* Create JSON formatted String with data */
		LinkedHashMap<String, Object> data = new LinkedHashMap<>();
		data.put(PROTOCOL_PARAMETERS_USER_ID, this.userId);
		data.put(PROTOCOL_PARAMETERS_MSG_TYPE, ProtocolMsgType.USER_DESASSOCIATION_REQUEST.getMsgTypeValue());

		String msg;
		try {
			msg = this.objectMapper.writeValueAsString(data);
		} catch (JsonProcessingException e) {
			return closeSocket(this.socket, RETURN_VALUE_ERROR);
		}

		prettyPrint("requestUserDesassociation", "Trying to disconnect...");

		/* Send JSON message through socket */
		if(!sendMsg(msg))
			return closeSocket(this.socket, RETURN_VALUE_ERROR);


		/* Set the timeout value and read response from socket */
		try {
			this.socket.setSoTimeout(USER_DESASSOCIATION_TIMEOUT_SECONDS*1000);
		} catch (SocketException e) {
			return closeSocket(this.socket, RETURN_VALUE_ERROR);
		}

		try {
			TimeUnit.NANOSECONDS.sleep(100);
		} catch (InterruptedException e) {
			closeSocket(this.socket, RETURN_VALUE_ERROR);
		}

		/* Parse response and check its values */
		LinkedHashMap response;
		try {
			response = this.objectMapper.readValue(this.in, LinkedHashMap.class);
		} catch (IOException e) {
			return closeSocket(this.socket, RETURN_VALUE_ERROR);
		}
		int responseId = Integer.parseInt(response.get(PROTOCOL_PARAMETERS_USER_ID).toString());
		int responseMsgType = Integer.parseInt(response.get(PROTOCOL_PARAMETERS_MSG_TYPE).toString());

		if(responseId != this.userId || responseMsgType != ProtocolMsgType.USER_DESASSOCIATION_ACK.getMsgTypeValue()) {
			return closeSocket(this.socket, RETURN_VALUE_ERROR);
		}

		prettyPrint("requestUserDesassociation", "Disconnected");

		return closeSocket(this.socket, RETURN_VALUE_OK);
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
			return closeSocket(this.socket, RETURN_VALUE_ERROR);
		if(this.userId < 0) {
			if((this.userId = getUserIdLocal()) < 0)
				return RETURN_VALUE_ERROR;
		}
		if(this.objectMapper == null) {
			this.objectMapper = new ObjectMapper();
			objectMapper.configure(AUTO_CLOSE_SOURCE, false);
			objectMapper.configure(SerializationFeature.INDENT_OUTPUT, true);
		}


		/* Create JSON formatted String with data */
		LinkedHashMap<Object, Object> data = new LinkedHashMap<>();
		data.put(PROTOCOL_PARAMETERS_USER_ID, this.userId);
		data.put(PROTOCOL_PARAMETERS_MSG_TYPE, ProtocolMsgType.GPS_COORDINATES_UPDATE.getMsgTypeValue());

		LinkedHashMap<Object, Object> gpsCoordinatesData = new LinkedHashMap<>();
		gpsCoordinatesData.put(PROTOCOL_PARAMETERS_GPS_COORDINATES_LAT, gpsCoordinates.getLatitude());
		gpsCoordinatesData.put(PROTOCOL_PARAMETERS_GPS_COORDINATES_LON, gpsCoordinates.getLongitude());
		gpsCoordinatesData.put(PROTOCOL_PARAMETERS_GPS_COORDINATES_ALT, gpsCoordinates.getAltitude());
		gpsCoordinatesData.put(
			PROTOCOL_PARAMETERS_GPS_COORDINATES_TIMESTAMP,
			gpsCoordinates.getTimestamp().withNano(0).atZone(ZoneOffset.UTC).toString()
		);

		data.put(PROTOCOL_PARAMETERS_GPS_COORDINATES, gpsCoordinatesData);


		String msg;
		try {
			msg = this.objectMapper.writeValueAsString(data);
		} catch (JsonProcessingException e) {
			return closeSocket(this.socket, RETURN_VALUE_ERROR);
		}

		prettyPrint("sendGpsCoordinatesToFap", "Sending coordinates update: \n" + msg);

		/* Send JSON message through socket */
		if(!sendMsg(msg))
			return closeSocket(this.socket, RETURN_VALUE_ERROR);


		/* Set the timeout value and read response from socket */
		try {
			this.socket.setSoTimeout(GPS_COORDINATES_UPDATE_TIMEOUT_SECONDS*1000);
		} catch (SocketException e) {
			return closeSocket(this.socket, RETURN_VALUE_ERROR);
		}


		try {
			TimeUnit.NANOSECONDS.sleep(100);
		} catch (InterruptedException e) {
			closeSocket(this.socket, RETURN_VALUE_ERROR);
		}

		/* Parse response and check its values */
		LinkedHashMap response = null;
		try {
			response = this.objectMapper.readValue(this.in, LinkedHashMap.class);
		} catch (IOException e) {
			return closeSocket(this.socket, RETURN_VALUE_ERROR);
		}


		if(response == null)
			return closeSocket(this.socket, RETURN_VALUE_ERROR);

		int responseId = Integer.parseInt(response.get(PROTOCOL_PARAMETERS_USER_ID).toString());
		int responseMsgType = Integer.parseInt(response.get(PROTOCOL_PARAMETERS_MSG_TYPE).toString());
		String responseTimestamp = response.get(PROTOCOL_PARAMETERS_GPS_TIMESTAMP).toString();


		if(responseId != this.userId
				|| responseMsgType != ProtocolMsgType.GPS_COORDINATES_ACK.getMsgTypeValue()
				|| !responseTimestamp.equals(gpsCoordinates.getTimestamp().withNano(0).atZone(ZoneOffset.UTC).toString())) {
			return closeSocket(this.socket, RETURN_VALUE_ERROR);
		}

		prettyPrint("sendGpsCoordinatesToFap", "Coordinates acknowledgement received");

		return RETURN_VALUE_OK;
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
			return -1;
		}

		return Integer.parseInt(ip.substring(ip.lastIndexOf('.') + 1, ip.length()));
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
			return -1;
		}
		try {
			BufferedReader in = new BufferedReader(new InputStreamReader(myExternalIP.openStream()));
			ip = in.readLine();
		} catch (IOException e) {
			return -1;
		}

		return Integer.parseInt(ip.substring(ip.lastIndexOf('.')+1, ip.length()));
	}

	/**
	 * Sends a message over the output stream
	 *
	 * @param msg 		String message to send (pref JSON format)
	 */
	private boolean sendMsg(String msg) {
		if(this.out == null) {
			return false;
		}

		this.out.println(msg);

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
				return RETURN_VALUE_ERROR;
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

	private void prettyPrint(String func, String status) {
		if(func == null)
			System.out.println(">> FapManagementProtocol_Client: " + status);
		else
			System.out.println(">> FapManagementProtocol_Client::" + func + "(): " + status);
	}
}