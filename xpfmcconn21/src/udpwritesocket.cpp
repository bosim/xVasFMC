#include "udpwritesocket.h"

#include <fstream>
#include <io.h>
#include <fcntl.h>

extern std::fstream m_logfile;

UDPWriteSocket::UDPWriteSocket()
{
#ifdef WIN32
    // Start the Winsocket API
    WSADATA wsa;

    // Load Winsock 2.2
    if ( WSAStartup (MAKEWORD(2, 2), &wsa) != 0 )
    {
        m_logfile << "WSAStartup failed" << std::endl;
        exit (1);
    }
#endif

    // Get a socket
    if ( (sockId = socket(PF_INET, SOCK_DGRAM, 0) ) < 0)
    {
        m_logfile << "PERROR: Socket failed" << std::endl;
        exit (1);
    }
}

UDPWriteSocket::~UDPWriteSocket()
{
    if(sockId > 0) {
        delete[] destinationAddr;
        // Close socket
        #ifdef WIN_32
        closesocket(sockId);
        WSACleanup();
        #else
        close (sockId);
        #endif
        sockId = -1;
    }
}

void UDPWriteSocket::configure(const std::string& host, int port)
{
    destinationAddr = new char [host.size()+1];
    strcpy(destinationAddr, host.c_str());
    destinationPort = port;
    m_logfile << "Host" << destinationAddr << ":" << destinationPort << std::endl;

    // Allow this socket to emit broadcasts.
    socketOption_t broadcast = 1;
    if ( setsockopt(sockId, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof (broadcast)) == -1 )
    {
        m_logfile << "PERROR: setsockopt BROADCAST" << std::endl;
        exit (1);
    }

    // Set the time to live/hop count.
    socketOption_t ttl = 4;
    if ( setsockopt(sockId, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof (ttl)) < 0 )
    {
        m_logfile << "PERROR setsockopt TTL" << std::endl;
        exit (1);
    }

    // Set up address and port of host.
    memset (&hostAddr, 0, sizeof(hostAddr));
    hostAddr.sin_family = AF_INET;
    hostAddr.sin_port = htons (destinationPort);
    hostAddr.sin_addr.s_addr = inet_addr (destinationAddr);
}

long UDPWriteSocket::write(const void* data, size_t size)
{
    m_logfile << "UDPReadSocket::write called" << std::endl;
    long bytes_written =
#ifdef WIN_32
            sendto (sockId, (const char*)data, size, 0, (struct sockaddr *)&hostAddr, sizeof(hostAddr));
#else
            sendto (sockId, data, size, 0, (struct sockaddr *)&hostAddr, sizeof(hostAddr));
#endif
    if ( bytes_written < 0 )
    {
        m_logfile << "Sendto2 failed" << std::endl;
        exit (1);
    }
    m_logfile << "Send" << bytes_written << std::endl;
    return bytes_written;
}
