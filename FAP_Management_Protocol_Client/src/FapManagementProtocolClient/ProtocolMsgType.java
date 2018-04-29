/******************************************************************************
*                         User-Aware Flying AP Project
*                       FAP Management Protocol (Client)
*******************************************************************************
*                        Comunicacoes Moveis 2017/2018
*                             FEUP | MIEEC / MIEIC
*******************************************************************************/

package FapManagementProtocolClient;


/**
 * Enum with the protocol's msgType values.
 */
enum ProtocolMsgType
{
	// ----- MSG TYPE VALUES ----- //
	USER_ASSOCIATION_REQUEST		(1),
	USER_ASSOCIATION_ACCEPTED		(2),
	USER_ASSOCIATION_REJECTED		(3),
	USER_DESASSOCIATION_REQUEST		(4),
	USER_DESASSOCIATION_ACK			(5),
	GPS_COORDINATES_UPDATE			(6),
	GPS_COORDINATES_ACK				(7);

	// ----- CONSTRUCTOR ----- //
	private final int msgTypeValue;

	private ProtocolMsgType(int mtv)
	{
		this.msgTypeValue = mtv;
	}

	public int getMsgTypeValue() {
		return msgTypeValue;
	}
}