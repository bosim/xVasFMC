#ifndef _MYASSERT_H__
#define _MYASSERT_H__

void assertFailed(const char *file, const char* function, int line);

#define MYASSERT(expr) if (!(expr)) assertFailed(__FILE__, __FUNCTION__, __LINE__)

#endif // _MYASSERT_H__
