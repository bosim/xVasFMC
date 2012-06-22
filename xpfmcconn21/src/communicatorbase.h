#ifndef COMMUNICATORBASE_H
#define COMMUNICATORBASE_H

#include <vector>
#include <string>
#include "myassert.h"

class Settings;
class LogicHandler;

template <typename T>
class DataContainer;
template <typename T>
class SimData;

class CommunicatorBase
{
public:
    CommunicatorBase() {}

    virtual ~CommunicatorBase() {}


    virtual bool configure() = 0;

    virtual bool processOutput(DataContainer<int>&, DataContainer<float>&, DataContainer<double>&,
                               DataContainer<bool>&, DataContainer<std::vector<float> >&,
                               DataContainer<std::vector<int> >&, DataContainer<std::string>&,
                               std::vector<LogicHandler*>, int ticks= 0, double secs =0.0f, unsigned int maxDataItems=256) = 0;

    virtual bool processInput(DataContainer<int>&, DataContainer<float>&, DataContainer<double>&,
                               DataContainer<bool>&, DataContainer<std::vector<float> >&,
                               DataContainer<std::vector<int> >&, DataContainer<std::string>&,
                               std::vector<LogicHandler*>, int ticks=0, double secs=0.0f) = 0;
};

#endif // COMMUNICATORBASE_H
