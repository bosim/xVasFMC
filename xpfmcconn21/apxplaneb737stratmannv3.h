#ifndef apxplaneb737stratmannv3_h
#define apxplaneb737stratmannv3_h

#include "logichandler.h"


class APXPlaneB737StratmannV3 : public LogicHandler {
public:
    APXPlaneB737StratmannV3(std::ostream& logfile):
    LogicHandler(logfile){}
    ~APXPlaneB737StratmannV3() {}
};

#endif
