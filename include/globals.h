#ifndef GLOBALS_H
#define GLOBALS_H

#include "SerialToolConfig.h"
#include <iup/iup.h>

// Tabs
extern Ihandle *tabs;
extern int tabs_count;

extern struct SerialPort *serialports[MAXIMUM_PORTS];
extern int serialcount;

#endif // GLOBALS_H