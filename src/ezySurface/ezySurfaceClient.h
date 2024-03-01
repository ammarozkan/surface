#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

struct ezySurfaceClient {
	int socketfd;
};
typedef struct ezySurfaceClient* EzySurfaceClient;

struct ProgramFirstRequest {
#define EZYSURFACE_WINDOW_STANDARD 1 // for using standard surface op etc.
#define EZYSURFACE_WINDOW_FREE 2 // for using opengl etc.
	uint8_t windowtype;
	uint32_t windowsize_x, windowsize_y;
};


struct ProgramRequestBase {
#define PROGRAM_REQUEST_NONE 0
#define PROGRAM_REQUEST_KILL 1
#define PROGRAM_REQUEST_INFORMATION 2
#define PROGRAM_REQUEST_ERROR 1024
	uint32_t requestType;
};

struct ProgramRequest_kill {
#define PROGRAM_REQUEST_KILL_WAIT 1
#define PROGRAM_REQUEST_KILL_IMMEDIATELY 2
	uint32_t priorty;
};

struct ProgramRequest {
	struct ProgramRequestBase base;
	void* req;
};


// returns -1 if connection cannot be established
// or some other kind error.
EzySurfaceClient
ezySurfaceConnection(const char* place)
{
	int socketfd = socket(AF_UNIX, SOCK_STREAM, 0);
	if(socketfd == -1) goto directexit;
	struct sockaddr_un server_addr;
	server_addr.sun_family = AF_UNIX;
	strcpy(server_addr.sun_path,place);
	if(connect(socketfd,(struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) goto socketexit;
	
	struct ezySurfaceClient* result = 
		malloc(sizeof(struct ezySurfaceClient));
	result->socketfd = socketfd;
	return result;

socketexit:
	close(socketfd);
directexit:
	return NULL;
}

int 
ezySurfaceFirstRequest(EzySurfaceClient client,
		uint8_t window_type,
		uint32_t width, uint32_t height)
{
	struct ProgramFirstRequest request;
       	request.windowtype = window_type;
	request.windowsize_x = width;
	request.windowsize_y = height;
	return write(client->socketfd,&request,sizeof(request));
}

int
ezySurfaceRequestKill(EzySurfaceClient client)
{
	struct ProgramRequestBase req = {PROGRAM_REQUEST_KILL};
	struct ProgramRequest_kill req_kill 
		= {PROGRAM_REQUEST_KILL_IMMEDIATELY};

	if(write(client->socketfd,&req,sizeof(req)) == -1) return -1;
	if(write(client->socketfd,&req_kill,sizeof(req_kill)) == -1) return -1;
	return 0;
}

#define RECEIVE_STRUCT(client, thestruct, errstr) \
	{ thestruct* req = malloc(sizeof(thestruct)); \
	int ret = read(client->socketfd, req, sizeof(thestruct)); \
	if(ret == -1 && errno == EAGAIN) { \
		free(req); \
		return NULL; \
	} \
	else if(ret < sizeof(thestruct)) { \
		perror(errstr); \
		free(req); \
		return NULL; \
	} \
	return req; } \

struct ProgramRequestBase*
ezySurfaceReceiveRequestBase(EzySurfaceClient client)
{
	RECEIVE_STRUCT(client,struct ProgramRequestBase, 
			"Couldn't receive base request.");
}
