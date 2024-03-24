#include "usurfTypes.h"
#include <sys/socket.h>
#include <sys/un.h>

int
usurfClientConnect(const char* place)
{
	int socketfd = socket(AF_UNIX, SOCK_STREAM, 0);
	if(socketfd == -1) goto directexit;
	struct sockaddr_un server_addr;
	server_addr.sun_family = AF_UNIX;
	strcpy(server_addr.sun_path,place);
	if(connect(socketfd,(struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) goto socketexit;
	
	return socketfd;

socketexit:
	close(socketfd);
directexit:
	return -1;
}

int
usurfClientSendEntry(int socket, uint32_t w, uint32_t h)
{
	struct usurfClientEntry* entry = 
		malloc(sizeof(struct usurfClientEntry));

	entry->zero1 = entry->zero2 = 0;
	entry->type = USURF_SURFACEAPI;
	entry->width = w; 
	entry->height = h;
	int ret = write(socket, entry, sizeof(*entry));
	
	if(ret == sizeof(entry)) return 1;
	return 0;
}

int
usurfClientSendAddText(int socket, uint8_t panel, uint32_t x, uint32_t y,
		char* text)
{
	size_t textsize = strlen(text) + 1;
	struct usurfAddText* msg = 
		malloc(sizeof(struct usurfAddText) + textsize);
	msg->header.type = USURF_ADDTEXT;
	msg->header.size = sizeof(*msg)+textsize;
	msg->panel = panel;
	msg->x = x;
	msg->y = y;
	char* ntext = msg + sizeof(struct usurfAddText);
	memcpy(ntext, text, textsize);
	int ret = write(socket, msg, sizeof(*msg)+textsize);
}

int
usurfClientSendAddButton(int socket, uint8_t panel, uint32_t x, uint32_t y,
		uint32_t w, uint32_t h, uint32_t buttonid, char* text)
{
	size_t textsize = strlen(text) + 1;
	struct usurfAddButton* msg = 
		malloc(sizeof(struct usurfAddButton) + textsize);
	msg->header.type = USURF_ADDBUTTON;
	msg->header.size = sizeof(*msg)+textsize;
	msg->panel = panel;
	msg->x = x;
	msg->y = y;
	msg->buttonid = buttonid;
	char* ntext = msg + sizeof(*msg);
	memcpy(ntext, text, textsize);
	int ret = write(socket, msg, sizeof(*msg)+textsize);
}
