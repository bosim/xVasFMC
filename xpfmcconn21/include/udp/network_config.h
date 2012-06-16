#ifdef WIN32
    #include <windows.h>
	#define sleep(x) Sleep((x)*1000)
    #define REUSE SO_REUSEADDR
	typedef char socketOption_t;

#else
	#ifdef __linux__
		#define REUSE SO_REUSEADDR
	#else
		#define REUSE SO_REUSEPORT
	#endif

	#include <sys/socket.h>
	#include <sys/types.h>
	#include <arpa/inet.h>
	#include <netinet/in.h>
	#include <netdb.h>		// gethostby*
	#define PERROR(x) perror(x)
	typedef int socketOption_t;
#endif
