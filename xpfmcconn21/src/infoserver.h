#ifndef INFOSERVER_H
#define INFOSERVER_H

#include <windows.h>
#include <string>
#include <fstream>

#include <XPLMDataAccess.h>
#include <XPLMUtilities.h>
#include <XPLMNavigation.h>

using namespace std;

#define MAX_WAYPOINTS 100


class InfoServer {
    public:
        InfoServer() {
        }

        /* Routine for splitting a line into a vector */
        void SplitLine(string s, vector<string>& l, char delim = ' ') {
            string::size_type i = 0;
            string::size_type last = 0;

            while(i < s.size()) {
                if(s[i] == delim) {
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

        string getFlightPlan(struct sockaddr_in const& fromaddr) {
            int s;
            char buf[256];
            long read_bytes;
            string str;

            struct sockaddr_in target;
            target.sin_family = AF_INET;
            target.sin_port = htons (50019); //Port to connect on
            target.sin_addr.s_addr = fromaddr.sin_addr.s_addr;

            s = socket (AF_INET, SOCK_STREAM, IPPROTO_TCP); //Create socket
            if (s <= 0) {
                return str;
            }
            if (connect(s, (SOCKADDR *)&target, sizeof(target)) == -1) {
                return str;
            }

            const char* get_request = "GET /persistence.dat\r\n";
            send(s, get_request, strlen(get_request), 0);

            while((read_bytes = recv(s, buf, sizeof(buf), 0)) == sizeof(buf)) {
              str += buf;
            }

            if(read_bytes > 0) {
              str += buf;
            }


            if (s) {
                closesocket(s);
            }

            return str;
       }

        void processFlightPlan(string input) {

            std::vector<string> lines;
            int i = 1;

            SplitLine(input, lines, '\n');
            for(std::vector<string>::iterator it = lines.begin();
                                              it != lines.end();
                                              ++it) {

                std::vector<string> elements;

                SplitLine((*it), elements, '|');

                if(elements.size() < 8) {
                    continue;
                }

                string waypoint = elements[0];
                float lat = atof(elements[1].c_str());
                float lon = atof(elements[2].c_str());
                string alt = elements[3];
                string type = elements[7];

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

                XPLMNavRef navaid = XPLMFindNavAid(NULL, waypoint.c_str(), &lat, &lon, NULL, xp_type);

                if(navaid != XPLM_NAV_NOT_FOUND) {
                    char navaid_id[32];
                    XPLMGetNavAidInfo(navaid, NULL, NULL, NULL, NULL, NULL, NULL, navaid_id, NULL, NULL);
                    /* We cannot be sure that X-Plane finds the right waypoint, e.g. TRS27 is found instead
                       of TRS near Stockholm */
                    if(!strncmp(navaid_id, waypoint.c_str(), strlen(navaid_id))) {
                        XPLMSetFMSEntryInfo(i, navaid, 0);
                    }
                    else {
                        XPLMSetFMSEntryLatLon(i, lat, lon, 0);
                    }
                }
                else {
                    XPLMSetFMSEntryLatLon(i, lat, lon, 0);
                }

                ++i;

                /* We will just load the first MAX_WAYPOINTS waypoints into x-plane */
                if(i >= MAX_WAYPOINTS) {
                    break;
                }

            }

            /* First waypoint in VASFMC route is _always_ previous waypoint */

            if(i > 1) {
                XPLMSetDestinationFMSEntry(2);
                XPLMSetDisplayedFMSEntry(2);
            }

            while(i < MAX_WAYPOINTS) {
                XPLMClearFMSEntry(i);
                ++i;
            }

        }
};

#endif // INFOSERVER_H
