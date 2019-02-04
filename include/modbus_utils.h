#ifndef MODBUS_UTILS_H
#define MODBUS_UTILS_H

#include <stdint.h>

enum ModbusFunction
{
    READ_COILS = 1,
    READ_DISCRETE_INPUTS,
    READ_HOLDING_REGISTERS,
    READ_INPUT_REGISTERS,
    WRITE_SINGLE_COILS,
    WRITE_SINGLE_REGISTER,
    WRITE_MULTIPLE_COILS = 15,
    WRITE_MULTIPLE_REGISTERS
};

struct ModbusMessage
{
    uint8_t address;
    struct ModbusPDU
    {
        enum ModbusFunction functionCode;
        uint8_t* data;
        size_t size;
    } pdu;
    uint16_t checksum;
};

size_t translateToASCIIStream(
    const struct ModbusMessage* message, uint8_t* buffer);
void translateFromASCIIStream(
    const uint8_t* message, size_t size, struct ModbusMessage* outMessage);

// General utilities
uint8_t nibbleToASCII(uint8_t nibble);
uint8_t ASCIIToByte(uint8_t high, uint8_t low);
uint8_t LRC(struct ModbusMessage* message);

#endif // MODBUS_UTILS_H