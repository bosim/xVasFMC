#include "myassert.h"
#include <iostream>
#include <fstream>
#include <cstdlib>

void assertFailed(const char *file, const char* function, int line)
{
    std::fstream assertfile("xpfmcconn.assert.txt", std::ios_base::out | std::ios_base::trunc);

    assertfile << "Assert failed in file " << file
            << "line " << line
            << "function " << function
            << std::endl;
    assertfile.close();
    exit(1);
}

