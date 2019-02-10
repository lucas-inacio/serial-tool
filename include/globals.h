#ifndef GLOBALS_H
#define GLOBALS_H

#include "SerialToolConfig.h"
#include <iup/iup.h>
#include <time.h>

#include "modbus_utils.h"
#include "serial_utils.h"

#ifdef MAIN_FILE
#define MODIFIER
#else
#define MODIFIER extern
#endif

#define MAXIMUM_PORTS 10
#define TEMP_BUFFER   512

// Tabs
MODIFIER Ihandle *tabs;
// extern struct SerialPort *serialports[MAXIMUM_PORTS];
MODIFIER int serialcount;
MODIFIER struct CommDescriptor serialports[MAXIMUM_PORTS];

struct ModbusQueue
{
    struct ModbusMessage *msg;
    struct ModbusMessage *next;
    clock_t time_since_request;
};

#endif // GLOBALS_H