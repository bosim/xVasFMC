#ifndef UDPWRITESOCKET_H
#define UDPWRITESOCKET_H

#include <string>
#include "my_include.h"
#include "network_config.h"
#include "paketwriter.h"

class UDPWriteSocket : public PaketWriter
{
public:
    UDPWriteSocket();
    virtual ~UDPWriteSocket();
    void configure(const std::string& host, int port);
    virtual long write(const void* data, size_t size);
private:
    int     sockId, destinationPort;
    char    msg[BUFF_LEN];
    char*   destinationAddr;
    struct sockaddr_in hostAddr;
};

#endif // UDPWRITESOCKET_H
