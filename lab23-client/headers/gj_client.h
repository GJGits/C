void setAddress(const char *ip, struct sockaddr_in *address);
int connectTcpClient(const char *address, const char *port);
void doClient(int connSock, char **requests);
void clientSend(int connSock, char *request);
void clientReceive(int connSock);
