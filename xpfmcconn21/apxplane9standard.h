#ifndef apxplane9standard_h
#define apxplane9standard_h

#include "logichandler.h"
#include "owneddata.h"
#include "simdata.h"

template <typename T>
inline const T &qMin(const T &a, const T &b) { if (a < b) return a; return b; }
template <typename T>
inline const T &qMax(const T &a, const T &b) { if (a < b) return b; return a; }
template <typename T>
inline const T &qBound(const T &min, const T &val, const T &max)
{ return qMax(min, qMin(max, val)); }

class APXPlane9Standard : public LogicHandler {
 public:

    APXPlane9Standard(std::ostream& logfile);

    virtual ~APXPlane9Standard();

    /**
     *  registers autopilot datarefs to access the xplane internal autopilot logic
     */
    virtual bool registerDataRefs();


    /**
     *  syncs internal hdg and vvi to the current value and initializes the internal ap state
     */
    virtual bool initState();


    /**
     * interprets xplane's ap state for the internal autopilot
     */
    virtual bool processState();


    /**
     * receives commands and sets internal and xp autopilot accordingly
     */
    virtual bool processInput(long input);


    /**
     * publishes internal state, hdg and vvi
     */
    virtual bool publishData();


    /**
     * unpublishes internal state, hdg and vvi
     */
    virtual bool unpublishData();


    /**
     * for the time being, every seventh flightloop is enough
     */
    virtual float processingFrequency();

    /**
      * identify handler
      */
    virtual std::string name() { return "APXPlane9Standard"; }

    /**
      * we have no special writes
      */
    virtual bool writeToStream(ProtocolStreamer*) { return false; }

    /**
      * we have no special writes
      */
    virtual bool needsSpecialWrite() { return false; }

    virtual void suspend(bool yes) { m_suspended = yes; }

    virtual bool isSuspended() { return m_suspended; }

  private:

     SimData<int>* m_simAPState;

    SimData<int>* m_simFDMode;

    SimData<float>* m_simVVI;

    SimData<float>* m_simHDG;

    SimData<bool>* m_simAPSPDisMach;

    SimData<float>* m_simBarAlt;

    SimData<float>* m_simAltWindow;

    SimData<bool>* m_simnavOVR;


    /*SimData<float>* m_joyRollInput;

    SimData<float>* m_joyPitchInput;*/

    OwnedData<int>* m_internalAPState;

    OwnedData<float>* m_internalVVI;

    OwnedData<float>* m_internalHDG;


    float m_last_received_vs;

    float m_last_received_hdg;

    static const int i_avail     = 1;
    static const int i_enabled   = 2;
    static const int i_hdg       = 4;
    static const int i_alt       = 8;
    static const int i_spd       = 16;
    static const int i_mach      = 32;
    static const int i_vs        = 64;
    static const int i_nav1      = 128;
    static const int i_gs        = 256;
    static const int i_app       = 512;
    static const int i_appbc     = 1024;
    static const int i_at_toga   = 2048;
    static const int i_at_arm    = 4096;
    static const int i_gps_cpl   = 8192;

    static const int x_atr = 1;
    static const int x_hdg = 2;
    static const int x_lvl = 4;
    static const int x_ash = 8;
    static const int x_vs = 16;
    static const int x_alt_arm = 32;
    static const int x_flch = 64;
    static const int x_autosync = 128;
    static const int x_hnav_arm = 256;
    static const int x_hnav_engage = 512;
    static const int x_gs_arm = 1024;
    static const int x_gs_engage = 2048;
    static const int x_fms_vnav_arm = 4096;
    static const int x_fms_vnav_enageg = 8192;
    static const int x_alt_engage = 16384;

    static const int alt_hold_engage_thresh_ft = 200;

    std::ostream& m_logfile;

    bool m_suspended;
};

#endif
