#ifndef logichandler_h
#define logichandler_h

class ProtocolStreamer;

#include <vector>
#include <string>

    /**
     *  abstract class for all kinds of processing that takes values from XP, do calculations in the flightloop and publish the result in a custom Dataref
     *  @author Philipp Muenzel
     *  @version 0.1
     */
class LogicHandler {

 public:

    LogicHandler(std::ostream&) {}

    virtual ~LogicHandler(){}

    /**
     *  reimplement to register for the data needed for calculation
     */
    virtual bool registerDataRefs() = 0;


    /**
     *  reimplement to do the setups that have to be done once when data is acessible
     */
    virtual bool initState() = 0;


    /**
     *  reimplement to do the calculations that are done periodically (via the flightloop)
     */
    virtual bool processState() = 0;


    /**
     *  reimplement to process a signal from outside (e.g. interplugin message)
     */
    virtual bool processInput(long input) = 0;


    /**
     *  reimplement to register the acessors to the owned data to form a custom dataref(s)
     */
    virtual bool publishData() = 0;


    /**
     *  reimplement to unregister the custom dataref(s)
     */
    virtual bool unpublishData() = 0;


    /**
     *  reimplement to return the value for the next call of your processing- positive for seconds, negative for ticks
     */
    virtual float processingFrequency() = 0;

    /**
      * reimplement to write specially handeled Data to the binary Data Stream
      */
    virtual bool writeToStream(ProtocolStreamer*) = 0;

    /**
      * reimplement to tell the communicator if it has to call writeToStream on this handler instance
      */
    virtual bool needsSpecialWrite() = 0;

    virtual void suspend(bool yes) = 0;

    virtual bool isSuspended() = 0;

    virtual std::string name() = 0;

    friend float HandlerCallbackInit(float, float, int, void*);

    friend float HandlerCallbackProcess(float, float, int, void*);

    void hookMeIn();

    void hookMeOff();

};

#endif
