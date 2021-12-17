#ifndef CONNECTION_H_CEYHUNCAKAR_2021_12_05
#define CONNECTION_H_CEYHUNCAKAR_2021_12_05

struct connection_s
{
	int sock;
};

int connection_init(struct connection_s* connection, char* ip, char* port);

int connection_send(struct connection_s* connection, char* buffer, int length);

int connection_receive(struct connection_s* connection, char* buffer, int length);

int connection_close(struct connection_s* connection);

#endif
