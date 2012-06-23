/*! \file    xplane.fsaccess.h
    \author  Alexander Wemmer, alex@wemmer.at
*/

#ifndef XPLANE_FSACCESS_H
#define XPLANE_FSACCESS_H

#include "fsaccess.h"
#include "canas.h"

#ifdef Q_OS_WIN32
#include <windows.h>
//#include <ws2tcpip.h>
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif


//! X-Plane Flightsim Access
class FSAccessXPlane : public FSAccess
{
    Q_OBJECT

public:

    //! Standard Constructor
    FSAccessXPlane(ConfigWidgetProvider* config_widget_provider,
                          const QString& cfg_file, FlightStatus* flightstatus);

    //! Destructor
    virtual ~FSAccessXPlane();

    virtual Config* config() { return &m_cfg; }

    //-----

    //! sets the NAV frequency, index 0 = NAV1, index 1 = NAV2
    virtual bool setNavFrequency(int freq, uint nav_index);
    //! sets the ADF frequency, index 0 = ADF1, index 1 = ADF2
    virtual bool setAdfFrequency(int freq, uint adf_index);
    //! sets the OBS angle, index 0 = NAV1, index 1 = NAV2
    virtual bool setNavOBS(int degrees, uint nav_index);

    virtual bool setAutothrustArm(bool armed);
    virtual bool setAutothrustSpeedHold(bool on);
    virtual bool setAutothrustMachHold(bool on);

    virtual bool setFDOnOff(bool on);
    virtual bool setAPOnOff(bool on);
    virtual bool setAPHeading(double heading);
    virtual bool setAPVs(int vs_ft_min);
    virtual bool setAPAlt(unsigned int alt);
    virtual bool setAPAirspeed(unsigned short speed_kts);
    virtual bool setAPMach(double mach);
    virtual bool setAPHeadingHold(bool on);
    virtual bool setAPAltHold(bool on);

    virtual bool setUTCTime(const QTime& utc_time);
    virtual bool setUTCDate(const QDate& utc_date);

    virtual bool freeThrottleAxes();
    virtual bool setThrottle(double percent);
    virtual bool setSBoxTransponder(bool) { return false; }
    virtual bool setSBoxIdent() { return false; }
    virtual bool setFlaps(uint);

    virtual bool setAileron(double percent);
    virtual bool setElevator(double percent);
    virtual bool setElevatorTrimPercent(double) { return false; }
    virtual bool setElevatorTrimDegrees(double) { return false; }

    virtual bool setSpoiler(double ) { return false; }

    virtual bool setNAV1Arm(bool on);
    virtual bool setAPPArm(bool on);

    virtual bool setAltimeterHpa(const double& hpa);
    virtual bool setReadFp();

protected slots:

    void slotSocketRead();

    void slotReadTimeout();

protected:

    //! XPlane fsaccess configuration
    Config m_cfg;

    QHostAddress m_read_hostaddress;
    QUdpSocket* m_read_socketdevice;
    QTimer m_read_timout_timer;

    QHostAddress m_write_hostaddress;
    unsigned int m_writeport;
    QUdpSocket* m_write_socketdevice;

    //FSTcasEntryValueList m_tcas_entry_list;

private:
    //! Hidden copy-constructor
    FSAccessXPlane(const FSAccessXPlane&);

    //! Hidden assignment operator
    const FSAccessXPlane& operator = (const FSAccessXPlane&);

    int     apstate;
    float   m_flaps_inc_per_notch;
    uint8_t m_message_code;
    void    inc_msgCode() { m_message_code = (m_message_code + 1)%256;}
    int     sendRequest(uint8_t service_code);
    bool    m_multicastActive;
    struct  ip_mreq m_multicastAddr;

    template <typename T> bool sendValue(int, T);
};

#endif /* XPLANE_FSACCESS_H */

// End of file
