#ifndef UDPREADSOCKET_H
#define UDPREADSOCKET_H

#include <string>
#include <stdint.h>
#include "my_include.h"
#include "network_config.h"

typedef int socklen_t;

class UDPReadSocket
{
public:
    UDPReadSocket();
    virtual ~UDPReadSocket();
    void configure(const std::string& host, int port);
    long read(void* data, size_t maxsize, struct sockaddr_in& fromaddr, socklen_t& fromaddr_len);


private:
    int         sockId, charsReceived, port;
    uint32_t    address;
    char        msg[BUFF_LEN];
    struct      ip_mreq multicast;
    bool        m_multicastActive;
    struct sockaddr_in  myAddr;
};

#endif // UDPREADSOCKET_H
