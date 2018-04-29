/******************************************************************************
*                         User-Aware Flying AP Project
*                       FAP Management Protocol (Client)
*******************************************************************************
*                        Comunicacoes Moveis 2017/2018
*                             FEUP | MIEEC / MIEIC
*******************************************************************************/

package test;

import FapManagementProtocolClient.*;


/**
 * FAP Management Protocol (Client) test.
 */
public class Test_FapManagementProtocol_Client
{
	/**
	 * Main.
	 */
	public static void main(String[] args)
	{
		System.out.println("=============================================\n" +
							"FAP MANAGEMENT PROTOCOL (CLIENT) TEST\n" +
		   					"=============================================\n");

		System.out.println("FAP Management Protocol (Client) Test not yet implemented!\n" +
							"Will be implemented by Eduardo Almeida.");




		/* ------------- IGNORE ------------- */

		FapManagementProtocol_Client fmpClient = new FapManagementProtocol_Client();


		System.out.println(fmpClient.requestUserAssociation() ? "RUA: OK" : "RUA: ERROR");
		System.out.println(fmpClient.requestUserDesassociation() ? "RUD: OK" : "RUD: ERROR");
		System.out.println(fmpClient.sendGpsCoordinatesToFap(null) ? "SGC: OK" : "SGC: ERROR");

		/* ------------- IGNORE ------------- */
	}

}