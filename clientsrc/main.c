#include "usurf/usurfClient.h"

#include <stdio.h>

int
main()
{
	int socketfd = -1;

	printf("Client Start!\n");
	
	while(socketfd == -1) {
		socketfd = usurfClientConnect("/surfacedesktop/regulardesktop-0");
	}
	printf("Client Connect %i!\n", socketfd);

	if(usurfClientSendEntry(socketfd, 300, 200)) printf("Client:Succes!\n");
	else printf("Client:Fail!\n");
	return 0;
}
