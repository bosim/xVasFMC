#include "fbwjoystickinterface.h"

#include "simdata.h"
#include "owneddata.h"
#include <vector>
#include <cmath>

FBWJoystickInterface::FBWJoystickInterface(std::ostream& logfile):
        LogicHandler(logfile),
        m_throttle_index(-1),
        m_roll_index(-1),
        m_pitch_index(-1),
        m_main_throttle_input_percent(0),
        m_roll_input_percent(0),
        m_pitch_input_percent(0),
        m_simJoystickDeflections(0),
        m_simJoystickAssignments(0),
        m_simJoystickMinimum(0),
        m_simJoystickMaximum(0),
        m_logfile(logfile)
{
}

FBWJoystickInterface::~FBWJoystickInterface()
{
    delete m_simJoystickDeflections;
    delete m_simJoystickAssignments;
    delete m_simJoystickMinimum,
    delete m_simJoystickMaximum;

    delete m_main_throttle_input_percent;
    delete m_roll_input_percent;
    delete m_pitch_input_percent;
}

bool FBWJoystickInterface::registerDataRefs()
{
    m_simJoystickAssignments = new SimData<std::vector<int> >("sim/joystick/joystick_axis_assignments","Assignments array of joystick axes",RWType::ReadOnly);
    m_simJoystickDeflections = new SimData<std::vector<float> >("sim/joystick/joystick_axis_values","Deflection values (raw) for joystick axes", RWType::ReadOnly);
    m_simJoystickMaximum = new SimData<std::vector<float> >("sim/joystick/joystick_axis_maximum","max seen raw deflection by axis", RWType::ReadOnly);
    m_simJoystickMinimum = new SimData<std::vector<float> >("sim/joystick/joystick_axis_minimum","min seen raw deflection by axis", RWType::ReadOnly);
    return true;
}

bool FBWJoystickInterface::publishData()
{
    m_main_throttle_input_percent = new OwnedData<float>("plugins/org/vasproject/xpfmcconn/joystick/throttle_input_percent");
    m_roll_input_percent = new OwnedData<float>("plugins/org/vasproject/xpfmcconn/joystick/roll_input_percent");
    m_pitch_input_percent = new OwnedData<float>("plugins/org/vasproject/xpfmcconn/joystick/pitch_input_percent");
    m_main_throttle_input_percent->registerRead();
    m_roll_input_percent->registerRead();
    m_pitch_input_percent->registerRead();
    return (m_main_throttle_input_percent->isRegistered() && m_roll_input_percent->isRegistered() &&
            m_pitch_input_percent->isRegistered());
}

bool FBWJoystickInterface::unpublishData()
{
    m_main_throttle_input_percent->unregister();
    m_roll_input_percent->unregister();
    m_pitch_input_percent->unregister();

    delete m_main_throttle_input_percent;
    m_main_throttle_input_percent = 0;
    delete m_roll_input_percent;
    m_roll_input_percent = 0;
    delete m_pitch_input_percent;
    m_pitch_input_percent = 0;

    return true;
}

bool FBWJoystickInterface::initState()
{
    m_simJoystickAssignments->poll();
    std::vector<int> joystick_axis = m_simJoystickAssignments->data();
    m_logfile << "Joystick axis array size: " << joystick_axis.size() << std::endl;
    for (uint i = 0 ; i < joystick_axis.size() ; i++)
    {
        //printf("Scanning %i to be %i\n",i,joystick_axis[i]);
        if(joystick_axis[i] == 4)
            m_throttle_index = i;
        if (joystick_axis[i] == 2)
            m_roll_index = i;
        if (joystick_axis[i] == 1)
            m_pitch_index = i;
    }
    m_logfile << "Found throttle at axis "<< m_throttle_index << std::endl;
    m_logfile << "Found roll at axis "<< m_roll_index << std::endl;
    m_logfile << "Found pitch at axis "<< m_pitch_index << std::endl;

    m_simJoystickMaximum->poll();
    m_simJoystickMinimum->poll();
    m_throttleMin = m_simJoystickMinimum->data()[m_throttle_index];
    m_throttleMax = m_simJoystickMaximum->data()[m_throttle_index];
    m_rollMin = m_simJoystickMinimum->data()[m_roll_index];
    m_rollMax = m_simJoystickMaximum->data()[m_roll_index];
    m_pitchMin = m_simJoystickMinimum->data()[m_pitch_index];
    m_pitchMax = m_simJoystickMaximum->data()[m_pitch_index];

    m_logfile << "Throttle max is: " << m_throttleMax << std::endl;
    m_logfile << "Throttle min is: " << m_throttleMin << std::endl;
    m_logfile << "Roll max is: " << m_rollMax << std::endl;
    m_logfile << "Roll min is: " << m_rollMin << std::endl;
    m_logfile << "Pitch max is: " << m_pitchMax << std::endl;
    m_logfile << "Pitch min is: " << m_pitchMin << std::endl;
    return (m_throttle_index > -1 && m_roll_index > -1 && m_pitch_index > -1);
}

bool FBWJoystickInterface::processState()
{
    m_simJoystickDeflections->poll();
    float throttle_input_percent = fabs( (m_simJoystickDeflections->data()[m_throttle_index] +
                                    m_throttleMin) * 100 / (m_throttleMax - m_throttleMin) );
    float roll_input_percent = fabs( (m_simJoystickDeflections->data()[m_roll_index] +
                                    m_rollMin) * 100 /(m_rollMax - m_rollMin ) );
    float pitch_input_percent = fabs( (m_simJoystickDeflections->data()[m_pitch_index] +
                                    m_pitchMin) * 100 / (m_pitchMax - m_pitchMin) );
    m_main_throttle_input_percent->set(throttle_input_percent);
    m_roll_input_percent->set(roll_input_percent);
    m_pitch_input_percent->set(pitch_input_percent);
    return true;
}

float FBWJoystickInterface::processingFrequency()
{
    return -3;
}
