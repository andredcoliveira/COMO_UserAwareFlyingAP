/******************************************************************************
*                         User-Aware Flying AP Project
*                       FAP Management Protocol (Client)
*******************************************************************************
*                        Comunicacoes Moveis 2017/2018
*                             FEUP | MIEEC / MIEIC
*******************************************************************************/

package FapManagementProtocolClient;


import java.time.LocalDateTime;



/**
 * GPS Coordinates in RAW format (lat, lon, alt).
 */
public class GpsCoordinates
{
	// =========================================================
	//           MEMBERS
	// =========================================================
	private float latitude;				// Latitude (in degrees)
	private float longitude;			// Longitude (in degrees)
	private float altitude;				// Altitude (in meters)
	private LocalDateTime timestamp;	// Timestamp in ISO8601 format



	// =========================================================
	//           FUNCTIONS
	// =========================================================

	/**
	 * Constructor of the GPS coordinates.
	 *
	 * @param latitude		Latitude (in degrees).
	 * @param longitude		Longitude (in degrees).
	 * @param altitude		Altitude (in meters).
	 * @param timestamp		Timestamp (ISO 8601).
	 */
	public GpsCoordinates(float latitude, float longitude, float altitude, LocalDateTime timestamp)
	{
		setLatitude(latitude);
		setLongitude(longitude);
		setAltitude(altitude);

		setTimestamp(timestamp);
	}


	/**
	 * Override of the equals method.
	 *
	 * @param objectToCompare	Object to compare.
	 * @return					True / false if the object to compare is equal / not equal.
	 */
	@Override
	public boolean equals(Object objectToCompare)
	{
		if (objectToCompare == null)
		{
			return false;
		}

		if (getClass() != objectToCompare.getClass())
		{
			return false;
		}


		final GpsCoordinates other = (GpsCoordinates) objectToCompare;

		return ((this.latitude == other.latitude) &&
				(this.longitude == other.longitude) &&
				(this.altitude == other.altitude) &&
				(this.timestamp.equals(other.timestamp)));
	}


	/**
	 * Get the String representation of the GPS coordinates.
	 *
	 * @return		String representation of the GPS coordinates.
	 */
	@Override
	public String toString()
	{
		return	"Lat: " + latitude + "deg" + "," +
				"Lon: " + longitude + "deg" + "," +
				"Alt: " + altitude + "m" + "," +
				"Timestamp: " + timestamp.toString();
	}


	// =========================================================
	//           GETTERS / SETTERS
	// =========================================================

	/**
	 * Get latitude (in degrees).
	 *
	 * @return	Latitude (in degrees).
	 */
	public float getLatitude()
	{
		return this.latitude;
	}

	/**
	 * Get longitude (in degrees).
	 *
	 * @return	Longitude (in degrees).
	 */
	public float getLongitude()
	{
		return this.longitude;
	}

	/**
	 * Get altitude (in meters).
	 *
	 * @return	Altitude (in meters).
	 */
	public float getAltitude()
	{
		return this.altitude;
	}

	/**
	 * Get timestamp.
	 *
	 * @return	Timestamp.
	 */
	public LocalDateTime getTimestamp()
	{
		return this.timestamp;
	}


	/**
	 * Set latitude (in degrees).
	 *
	 * @param latitude		Latitude (in degrees).
	 * @throws				IllegalArgumentException if the latitude is invalid.
	 */
	public void setLatitude(float latitude) throws IllegalArgumentException
	{
		if (latitude < -90 || latitude > 90)
			throw new IllegalArgumentException("Invalid latitude");

		this.latitude = latitude;
	}

	/**
	 * Set longitude (in degrees).
	 *
	 * @param longitude		Longitude (in degrees).
	 * @throws				IllegalArgumentException if the longitude is invalid.
	 */
	public void setLongitude(float longitude) throws IllegalArgumentException
	{
		if (longitude < -180 || longitude > 180)
			throw new IllegalArgumentException("Invalid longitude");

		this.longitude = longitude;
	}

	/**
	 * Set altitude (in meters).
	 *
	 * @param altitude		Altitude (in meters).
	 */
	public void setAltitude(float altitude)
	{
		this.altitude = altitude;
	}

	/**
	 * Set timestamp.
	 *
	 * @param timestamp		Timestamp.
	 */
	public void setTimestamp(LocalDateTime timestamp)
	{
		this.timestamp = timestamp;
	}
}