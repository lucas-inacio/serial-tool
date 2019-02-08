#ifndef GLOBALS_H
#define GLOBALS_H

#include "SerialToolConfig.h"
#include <iup/iup.h>

// Tabs
extern Ihandle *tabs;
// extern struct SerialPort *serialports[MAXIMUM_PORTS];
extern int serialcount;
extern struct CommDescriptor serialports[MAXIMUM_PORTS];

#endif // GLOBALS_H