#ifndef FBWJOYSTICKINTERFACE_H
#define FBWJOYSTICKINTERFACE_H

#include "logichandler.h"
#include <vector>

template <typename T>
class SimData;

template <typename T>
class OwnedData;

class FBWJoystickInterface : public LogicHandler
{
public:

    FBWJoystickInterface(std::ostream& logfile);

    virtual ~FBWJoystickInterface();

    /**
     *  reimplement to register for the data needed for calculation
     */
    virtual bool registerDataRefs();


    /**
     *  reimplement to do the setups that have to be done once when data is acessible
     */
    virtual bool initState();


    /**
     *  reimplement to do the calculations that are done periodically (via the flightloop)
     */
    virtual bool processState();


    /**
     *  reimplement to process a signal from outside (e.g. interplugin message)
     */
    virtual bool processInput(long) { return false; }


    /**
     *  reimplement to register the acessors to the owned data to form a custom dataref(s)
     */
    virtual bool publishData();


    /**
     *  reimplement to unregister the custom dataref(s)
     */
    virtual bool unpublishData();


    /**
     *  reimplement to return the value for the next call of your processing- positive for seconds, negative for ticks
     */
    virtual float processingFrequency();

    /**
      * reimplement to write specially handeled Data to the binary Data Stream
      */
    virtual bool writeToStream(ProtocolStreamer*) { return false; }

    virtual std::string name() { return "FBWJoystickInterface"; }

    virtual bool needsSpecialWrite() { return false; }

    virtual void suspend(bool yes) { m_suspended = yes; }

    virtual bool isSuspended() { return m_suspended; }

private:

    int m_throttle_index;

    int m_roll_index;

    int m_pitch_index;

    float m_throttleMin;

    float m_throttleMax;

    float m_rollMin;

    float m_rollMax;

    float m_pitchMin;

    float m_pitchMax;

    OwnedData<float>* m_main_throttle_input_percent;

    OwnedData<float>* m_roll_input_percent;

    OwnedData<float>* m_pitch_input_percent;

    SimData<std::vector<float> >* m_simJoystickDeflections;

    SimData<std::vector<int> >* m_simJoystickAssignments;

    SimData<std::vector<float> >* m_simJoystickMinimum;

    SimData<std::vector<float> >* m_simJoystickMaximum;

    std::ostream& m_logfile;

    bool m_suspended;
};

#endif // FBWJOYSTICKINTERFACE_H
