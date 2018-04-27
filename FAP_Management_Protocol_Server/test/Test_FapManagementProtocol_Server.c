/******************************************************************************
*                         User-Aware Flying AP Project
*                       FAP Management Protocol (Server)
*******************************************************************************
*                        Comunicacoes Moveis 2017/2018
*                             FEUP | MIEEC / MIEIC
*******************************************************************************/

#include <stdio.h>
#include <signal.h>
#include <pthread.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include "FapManagementProtocol_Server.h"
#include "MavlinkEmulator.h"
#include "GpsCoordinates.h"
int main()
{
	printf("=============================================\n"
		   "FAP MANAGEMENT PROTOCOL (SERVER) TEST\n"
		   "=============================================\n\n");

	printf("FAP Management Protocol (Server) Test not yet implemented!\n"
		   "Will be implemented by Eduardo Almeida.\n");
	GpsNedCoordinates user[10];	int n;
	initializeFapManagementProtocol();
    printf("WAIT FOR ENTER\n");
    getchar();
    getAllUsersGpsNedCoordinates(user, &n);
    printf("N: %d\n", n);
    printf("%f\n %f\n %f\n %s\n", user[0].x, user[0].y, user[0].z, user[0].timestamp);
    printf("WAIT FOR ENTER\n");
    getchar();
    printf("Closing FAP Management\n");

    terminateFapManagementProtocol();
	return 0;
}