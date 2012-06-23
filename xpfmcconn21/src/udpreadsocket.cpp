#include "udpreadsocket.h"

#include <fstream>
#include <io.h>
#include <fcntl.h>

extern std::fstream m_logfile;

int ready_read(int sock)
{
    int             res;
    fd_set          sready;
    struct timeval  nowait;

    FD_ZERO(&sready);
    FD_SET((unsigned int)sock,&sready);
    nowait.tv_sec = 0;
    nowait.tv_usec = 0;

    res = select(sock+1,&sready,NULL,NULL,&nowait);
    if( FD_ISSET(sock,&sready) )
        res = 1;
    else
        res = 0;

    return(res);
}

UDPReadSocket::UDPReadSocket()
{
#ifdef WIN32
    // Start the Winsocket API
    WSADATA wsa;

    // Load Winsock 2.2
    if ( WSAStartup (MAKEWORD(2, 2), &wsa) != 0 )
    {
        m_logfile << " WSAStartup failed" << std::endl;
        exit (1);
    }
#endif

    // Create socket
    if ( (sockId = socket(PF_INET, SOCK_DGRAM, 0)) < 0 )
    {
        m_logfile << "PERROR Socket" << std::endl;
        exit (1);
    }
}

UDPReadSocket::~UDPReadSocket()
{
    if (sockId > 0) {
        if ( m_multicastActive )
        {
            #ifdef WIN_32
            setsockopt(sockId, IPPROTO_IP, IP_DROP_MEMBERSHIP, (const char *)&multicast, sizeof(multicast));
            #else
            setsockopt(sockId, IPPROTO_IP, IP_DROP_MEMBERSHIP, (const void *)&multicast, sizeof(multicast));
            #endif
        }

        #ifdef WIN_32
        closesocket(sockId);
        WSACleanup();
        #else
        close(sockId);
        #endif

        sockId = -1;
    }
}

void UDPReadSocket::configure(const std::string& host, int in_port)
{
    // If multiple processes should receive multicast, this must be enabled
    // a) on EACH of the processes and
    // b) BEFORE the call to bind!
    // Depending on OS the option enabling port reuse is called SO_REUSEPORT
    // or SO_REUSADDR.
    // The hints on the internet that SO_REUSEADDR behaves identically to
    // SO_REUSEPORT when a multicast address is bound to the socket proved
    // false. This has an inner logic, because REUSE must be set before bind.
    // Experimaentation has shown that a REUSE after bind is without effect.
    // So we use a generic REUSE re-defined for the different OS nneds.
    socketOption_t yes = 1;
    if ( setsockopt(sockId, SOL_SOCKET, REUSE, &yes, sizeof(yes)) == -1 ) {
        m_logfile << "setsockopt REUSE" << std::endl;
        exit (1);
    }


    // Start setting up our address.
    memset (&myAddr, 0, sizeof(myAddr));
    myAddr.sin_family = AF_INET;
    myAddr.sin_port = htons (in_port);
    myAddr.sin_addr.s_addr = htonl (INADDR_ANY);

    // Bind to it
    if ( bind(sockId, (struct sockaddr *) &myAddr, sizeof(myAddr)) < 0 )
    {
        m_logfile << "PERROR: Bind" << std::endl;
        exit (1);
    }

    address = inet_addr ( host.c_str() );

    // If the address given is a valid multicast group join it.
    // For Windos this must happen after the bind to the socket.
    if ( (ntohl(address) >= 0xe0000000) && (ntohl(address) <= 0xefffffff) )
    {
        // Always use the default interface.
        multicast.imr_interface.s_addr = htonl (INADDR_ANY);
        multicast.imr_multiaddr.s_addr = address;

        // Join the multicast group.
#ifdef WIN_32
        if ( setsockopt(sockId, IPPROTO_IP, IP_ADD_MEMBERSHIP, (const char *)&multicast, sizeof(multicast)) < 0 ) {
#else
        if ( setsockopt(sockId, IPPROTO_IP, IP_ADD_MEMBERSHIP, (const void *)&multicast, sizeof(multicast)) < 0 ) {
#endif
            m_logfile << "PERROR: setsockopt ADD_MEMBERSHIP" << std::endl;
            exit (1);
        } else
        {
            m_multicastActive = true;
        }
    }

    // Set the socket non blocking
    // otherwise it qould stop X-Plane when no messages imminent
    //int socketflags = fcntl (sockId, F_GETFL, 0);
    //fcntl (sockId, F_SETFL, socketflags | O_NONBLOCK);
}

long UDPReadSocket::read(void* data, size_t maxsize)
{
    fromAddr_len = sizeof (fromAddr);
    memset (&fromAddr, 0, fromAddr_len);
    m_logfile << "UDPReadSocket::read called" << std::endl;
    //memset (msg, 0, BUFF_LEN);

    if (ready_read(sockId) == 1) {
        long bytesReceived =
#ifdef WIN_32
            recvfrom (sockId, (char*)data, maxsize, 0, (struct sockaddr *)&fromAddr, (int*) &fromAddr_len);
#else
            recvfrom (sockId, data, maxsize, 0, (struct sockaddr *)&fromAddr, &fromAddr_len);
#endif
        if( bytesReceived < 0 )
        {
            m_logfile << "PERROR: recvfrom" << std::endl;
            exit (1);
        } else
            m_logfile << "Recv" << bytesReceived << std::endl;
            return bytesReceived;
    } else {
        m_logfile << "Recv" << 0 << std::endl;
        return 0;
    }
}
