#ifndef MODBUS_UTILS_H
#define MODBUS_UTILS_H

#include <stdint.h>

#define COIL_ON  0xFF00
#define COIL_OFF 0x0000

enum ModbusFunction
{
    READ_COILS = 1,
    READ_DISCRETE_INPUTS,
    READ_HOLDING_REGISTERS,
    READ_INPUT_REGISTERS,
    WRITE_SINGLE_COIL,
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

// Translates a ModbusMessage to a stream of bytes already in the ASCII format
size_t translateToASCIIStream(
    const struct ModbusMessage* message, uint8_t* buffer);
// Translates a stream of bytes to a ModbusMessage
// Note: the message must not contain the ':' and the termination 'CR' and 'LF' bytes
void translateFromASCIIStream(
    const uint8_t* message, size_t size, struct ModbusMessage* outMessage);

// General utilities
uint8_t nibbleToASCII(uint8_t nibble);
uint8_t ASCIIToByte(uint8_t high, uint8_t low);
uint8_t LRC(struct ModbusMessage* message);

// Many modbus functions have the format
// | funcionCode (1 byte) | startAddress (2 bytes) | quantityOfRegisters (2 bytes) |
void BuildRequest(
    struct ModbusMessage* msg,
    uint8_t address,
    enum ModbusFunction functionCode,
    uint16_t startingAddress,
    uint16_t value);


#endif // MODBUS_UTILS_H