// size is 0-255 -> 0-100
struct usurfClientEntry {
	uint32_t zero1, zero2; // skipping wayland
#define USURF_SURFACEAPI 0x01
	uint8_t type;
	uint32_t width, height;
#define USURF_PANEL_HORIZONTAL 0x01
#define USURF_PANEL_VERTICAL 0x02
	uint8_t panelType;
	uint8_t phaseCount;
	uint8_t phase_panelCount[4]; uint8_t phase_size[4];
	uint8_t panelSizes[4][4]; // [phaseid][panelid]
};

int
usurfCreateUnixServer(const char* place)
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
usurfLookupClients(int server_socket)
{
	int client_socket;
	struct sockaddr_un client_addr;
	int clen = sizeof(struct sockaddr_un);

	client_socket = accept(server_socket, (struct sockaddr*)&client_addr, 
			&clen);
	if (client_socket == -1) {
		if (errno == EAGAIN) {
			return NULL;
		} else {
			perror("accept new client in usurfLookup");
			return NULL;
		}
	}

	int flags = fcntl(client_socket, F_GETFL, 0);
	fcntl(client_socket, F_SETFL, flags|O_NONBLOCK);

	return client_socket;
}


