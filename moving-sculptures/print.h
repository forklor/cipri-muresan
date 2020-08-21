#include "config.h"

#ifdef DEBUG

#define s_print(a) Serial.print(a)
#define s_println(a) Serial.println(a)

#endif


#ifndef DEBUG

#define s_print(a)
#define s_println(a)

#endif	