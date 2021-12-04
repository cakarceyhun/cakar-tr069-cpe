#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN

#include <winsock2.h>
#endif

struct connection_s
{
#if defined(_WIN32)
	SOCKET socket;
#else
	int sock;
#endif
};

int connection_init(struct connection_s* connection, char* ip, char* port);

int connection_send(struct connection_s* connection, char* buffer, int length);

int connection_receive(struct connection_s* connection, char* buffer, int length);

int connection_close(struct connection_s* connection);
