#include "logichandler.h"
#include "XPLMProcessing.h"
#include <iostream>
#include <fstream>

extern std::fstream m_logfile;

float HandlerCallbackInit(float, float, int, void* inRefCon)
{
    LogicHandler* handler = static_cast<LogicHandler*>(inRefCon);
    handler->initState();
    return 0;
}

float HandlerCallbackProcess(float, float, int, void* inRefCon)
{
    LogicHandler* handler = static_cast<LogicHandler*>(inRefCon);
    if (!handler->isSuspended())
        handler->processState();
    else
        m_logfile << "Handler suspended " << handler->name() << std::endl;
    return handler->processingFrequency();
}

void LogicHandler::hookMeIn()
{
    if (!this->publishData()) {
        m_logfile << "Publishing Data failed for " << this->name() << std::endl;
    }
    this->suspend(false);
    XPLMRegisterFlightLoopCallback(HandlerCallbackInit,-1,this);
    XPLMRegisterFlightLoopCallback(HandlerCallbackProcess,-3,this);
}

void LogicHandler::hookMeOff()
{
    XPLMUnregisterFlightLoopCallback(HandlerCallbackInit, this);
    XPLMUnregisterFlightLoopCallback(HandlerCallbackProcess, this);
}
