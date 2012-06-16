// mmx.h

#ifndef MMX_H
#define MMX_H

#include <stdint.h>

class MMXState
{
public:
    void save();

    void restore();

private:
    uint8_t state[108];
};

#endif // MMX_H
