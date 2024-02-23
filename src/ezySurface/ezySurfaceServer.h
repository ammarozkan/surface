// UNIX file system server implementation for connection to other programs.
//  
//
//
//
// They need to send information about what they want to be in the screen, window etc.
//
// Maybe a authentication work is needed in procces id wise. Like, is this a program
// that executed "from" here?
//
// Struct definitions is needed for writing.

// Lets name this standard as ezySurface

#include <sys/socket.h>

#include "ezySurfaceClient.h" // generic ones are there. 

int
ezySurfaceCreateUnixServer(const char* place)
{
	int server_socket;
	struct sockaddr_un server_addr;
	
	if( (server_socket = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
		perror("socket() error");
		return -1;
	}
	
	server_addr.sun_family = AF_UNIX;
	strcpy(server_addr.sun_path, place);
	if(bind(server_socket, (struct sockaddr*) &server_addr, 
			sizeof(server_addr)) == -1) { 
		perror("bind() error");
		return -1;
	}
	
	if(listen(server_socket, 3) == -1) {
		perror("listen() error");
		return -1;
	}

	int flags = fcntl(server_socket, F_GETFL,0);
	fcntl(server_socket, F_SETFL, flags|O_NONBLOCK);

	return server_socket;
}

struct ezySurfaceClient*
ezySurfaceLookUpClients(int server_socket)
{
	int client_socket; int clen = sizeof(struct sockaddr_un); struct sockaddr_un client_addr;
	if((client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &clen)) == -1) {
		if(errno == EAGAIN) {
			return NULL;
		} else {
			perror("accept new client in ezySurfaceLookUp");
			return NULL;
		}
	}
	int flags = fcntl(client_socket, F_GETFL,0);
	fcntl(client_socket, F_SETFL, flags|O_NONBLOCK);
	
	struct ezySurfaceClient* result = malloc(sizeof(struct ezySurfaceClient));
	result->socketfd = client_socket;
	return result;
}

struct ProgramFirstRequest*
ezySurfaceFirstRequestReceive(struct ezySurfaceClient* client)
{
	struct ProgramFirstRequest* firstrequest = malloc(sizeof(struct ProgramFirstRequest));
	int ret = read(client->socketfd,firstrequest,sizeof(struct ProgramFirstRequest));
	if(ret == -1) {
		free(firstrequest); return NULL;
	} else {
		return firstrequest;
	}
}

uint32_t ezySurfaceLookupRequest(struct ezySurfaceClient* client)
{
	uint32_t result;
	int ret = read(client->socketfd, &result, sizeof(result));
	if(ret == -1) {
		if(errno == EAGAIN) return PROGRAM_REQUEST_NONE;
		else return PROGRAM_REQUEST_ERROR; 	
	} else {
		return result;
	}
}


struct ProgramRequest_kill*
ezySurfaceReceiveKillRequest(struct ezySurfaceClient* client)
{
	RECEIVE_STRUCT(client, struct ProgramRequest_kill,
			"Couldn't receive kill request.");
}
