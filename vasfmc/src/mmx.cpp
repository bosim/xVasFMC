// mmx.cpp

#include "mmx.h"

void MMXState::save()
{
#ifdef _X86_
    __asm__ __volatile__
    (
        "fsave      (%%eax)\n\t"

    // output registers
    : // none
    
    // input registers
    : "a" (state)

    // clobbers
    :
    );
#endif
}

void MMXState::restore()
{
#ifdef _X86_
    __asm__ __volatile__
    (
        "emms       \n\t"
        "frstor     (%%eax)\n\t"

    // output registers
    : // none
    
    // input registers
    : "a" (state)

    // clobbers
    :
    );
#endif
}
