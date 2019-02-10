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
    struct ModbusQueue *next;
    clock_t time_since_request;
    size_t remaining, total;
    struct SerialPort *port;
};

// ModbusQueue functions
// Includes an item at the end of the queue
MODIFIER void add_item(struct ModbusQueue **queue, struct ModbusMessage *msg);
// Removes the first item and returns the new starting element
MODIFIER void remove_item(struct ModbusQueue **queue);

// These functions build and add elements in the
// ModbusQueue (they don't actually send anything)
MODIFIER void ReadRequest(
    struct ModbusQueue **queue,
    enum ModbusFunction function,
    uint8_t id, uint16_t start, uint16_t quantity);

MODIFIER void SendModbusMessage(struct ModbusQueue **queue);

MODIFIER void DestroyRequest(struct ModbusQueue **queue);

// MODIFIER WriteSingleCoil(
//     struct ModbusQueue *queue, uint8_t id, uint16_t address, uint16_t value);
// MODIFIER WriteSingleRegister(
//     struct ModbusQueue *queue, uint8_t id, uint16_t start, uint16_t quantity);
// MODIFIER WriteMultipleCoils(
//     struct ModbusQueue *queue, uint8_t id, uint16_t start, uint16_t quantity);
// MODIFIER WriteMultipleRegisters(
//     struct ModbusQueue *queue, uint8_t id, uint16_t start, uint16_t quantity);

#endif // GLOBALS_H