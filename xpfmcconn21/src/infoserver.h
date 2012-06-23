#ifndef INFOSERVER_H
#define INFOSERVER_H

#include <windows.h>
#include <string>
#include <fstream>
#include <stdexcept>

using namespace std;

void SplitLine(string s, vector<string>& l, char delim = ' ') {
  string::size_type i = 0;
  string::size_type last = 0;

  bool ignore = false;

  while(i < s.size()) {
    if(s[i] == '"') {

      if(!ignore) {
    ignore = true;
    last = i + 1;
      }
      else {
    string new_str = s.substr(last, i-last);
    l.push_back(new_str);
    last = i + 2;
    i += 2;
    ignore = false;
      }
    }
    else if(s[i] == delim && !ignore) {
      string new_str = s.substr(last, i - last);
      l.push_back(new_str);
      last = i+1;
    }

    ++i;
  }
  if(s.size() - last > 0) {
    string new_str = s.substr(last, s.size() - last);
    l.push_back(new_str);
  }


}

class InfoServer {
    public:
        InfoServer(std::ostream& m_logfile) : logfile(m_logfile) {
        }

        string getFlightPlan() {
            int s;
            char buf[256];
            long read_bytes;

            string str;

            #ifdef WIN_32
            WSADATA wsadata;
            MYASSERT(WSAStartup(0x0202, &wsadata) == 0);
            #endif

            struct sockaddr_in target;
            target.sin_family = AF_INET;
            target.sin_port = htons (50019); //Port to connect on
            target.sin_addr.s_addr = inet_addr ("127.0.0.1"); //Target IP
            s = socket (AF_INET, SOCK_STREAM, IPPROTO_TCP); //Create socket
            if (s <= 0) {
                throw runtime_error("Cannot open socket");
            }
            if (connect(s, (SOCKADDR *)&target, sizeof(target)) == -1) {
                throw runtime_error("Cannot connect");
            }

            const char* get_request = "GET /persistence.dat\r\n";
            send(s, get_request, strlen(get_request), 0);
            logfile << "Send get request";
            while((read_bytes = recv(s, buf, sizeof(buf), 0)) == sizeof(buf)) {
              str += buf;
              logfile << "Got data! " << sizeof(buf) << std::endl;
            }

            if(read_bytes > 0) {
              str += buf;
            }

            logfile << "Flightplan" << str << std::endl;
            logfile.flush();

            if (s)
                closesocket(s);

            #if WIN_32
            WSACleanup(); //Clean up Winsock
            #endif

            return str;
       }

        void processFlightPlan(string input) {
            logfile << "processFlightPlan called" << endl;
            logfile.flush();

            std::vector<string> l;
            int i;

            for(i=0; i < 100; ++i) {
                XPLMClearFMSEntry(i);
            }

            i = 1;

            SplitLine(input, l, '\n');
            for(std::vector<string>::iterator it = l.begin(); it != l.end(); ++it) {
                std::vector<string> ll;
                SplitLine((*it), ll, '|');
                if(ll.size() > 7) {

                    string waypoint = ll[0];
                    float lat = atof(ll[1].c_str());
                    float lon = atof(ll[2].c_str());
                    string alt = ll[3];
                    string procedure = ll[4];
                    string proceduretype = ll[5];
                    string type = ll[7];

                    int xp_type;

                    if(type.find("WAYPOINT") != string::npos) {
                        xp_type = xplm_Nav_Fix;
                    }
                    else if(type.find("AIRPORT") != string::npos) {
                        xp_type = xplm_Nav_Airport;
                    }
                    else if(type.find("VOR") != string::npos) {
                        xp_type = xplm_Nav_VOR;
                    }
                    else if(type.find("NDB") != string::npos) {
                        xp_type = xplm_Nav_NDB;
                    }
                    else {
                        xp_type = xplm_Nav_Unknown;
                    }

                    logfile << "Processing " << waypoint << endl;
                    logfile.flush();

                    XPLMNavRef navaid = XPLMFindNavAid(NULL, waypoint.c_str(), &lat, &lon, NULL, xp_type);

                    if(navaid != XPLM_NAV_NOT_FOUND) {
                        logfile << "Got NavAid " << waypoint << endl;
                        logfile.flush();
                        XPLMSetFMSEntryInfo(i, navaid, 0);
                    }
                    else {
                        logfile << "Did not get NavAid " << waypoint << endl;
                        logfile.flush();
                        XPLMSetFMSEntryLatLon(i, lat, lon, 0);
                    }

                    ++i;
                }
            }
            XPLMSetDestinationFMSEntry(1);
            XPLMSetDisplayedFMSEntry(1);
        }

    private:
        ostream& logfile;
};

#endif // INFOSERVER_H
