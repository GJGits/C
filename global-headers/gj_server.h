int startTcpServer(in_addr_t addr, const char* port);
void runTcpInstance(int passiveSock);
void doTcpJob(int connSock);
int doTcpReceive(int connSock, char *request);
void doTcpSend(int connSock, char *request);
