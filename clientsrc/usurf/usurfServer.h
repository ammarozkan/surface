#include <stdint.h>
#include <sys/socket.h>
#include <sys/un.h>
#include "usurfTypes.h"
	
int
usurfServerCreateUnixServer(const char* place)
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

	int flags = fcntl(server_socket, F_GETFL, 0);
	fcntl(server_socket, F_SETFL, flags|O_NONBLOCK);

	return server_socket;
}

int
usurfServerLookupClients(int server_socket)
{
	int client_socket;
	struct sockaddr_un client_addr;
	int clen = sizeof(struct sockaddr_un);

	client_socket = accept(server_socket, 
			(struct sockaddr*)&client_addr, &clen);
	if (client_socket == -1) {
		if (errno == EAGAIN) {
			return -1;
		} else {
			perror("accept new client in usurfLookup");
			return -2;
		}
	}

	int flags = fcntl(client_socket, F_GETFL, 0);
	fcntl(client_socket, F_SETFL, flags|O_NONBLOCK);

	return client_socket;
}

struct usurfClientEntry*
usurfServerGetEntry(int socket)
{
	struct usurfClientEntry* entry = 
		malloc(sizeof(struct usurfClientEntry));

	int ret = read(socket, entry, sizeof(*entry));
	if(ret == -1) {
		free(entry);
		return NULL;
	}

	return entry;
}

void
usurfServerRecvAddText(struct usurfAddText* msg,
		struct usurfServerInterface* interface)
{
	char* text = msg + sizeof(struct usurfAddText);
	size_t textlength = msg->header.size - sizeof(struct usurfAddText);
	interface->addText(interface->window, 
			msg->panel, msg->x, msg->y, text);
}

void
usurfServerRecvAddButton(struct usurfAddButton* msg,
		struct usurfServerInterface* interface)
{
	char* text = msg + sizeof(struct usurfAddButton);
	size_t textlength = msg->header.size - sizeof(struct usurfAddText);
	interface->addButton(interface->window, msg->buttonid, 
			msg->panel, msg->x, msg->y,
			msg->width, msg->height, text);
}

void
usurfServerRecvMsg(int socket, struct usurfServerInterface* interface)
{
	struct usurfHeader* header = malloc(1024);
	int size = read(socket, header, 1024);
	if(size == header->size) {
		if(header->type == USURF_ADDTEXT)
			usurfServerRecvAddText(header, interface);
		else if(header->type == USURF_ADDBUTTON)
			usurfServerRecvAddButton(header, interface);
	}
}
