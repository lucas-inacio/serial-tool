#include "modbus_utils.h"


size_t translateToASCIIStream(
    const struct ModbusMessage* message, uint8_t* buffer)
{
    uint8_t high = (message->address >> 4) & 0x0F;
    uint8_t low = message->address & 0x0F;
    buffer[0] = nibbleToASCII(high);
    buffer[1] = nibbleToASCII(low);

    high = (message->pdu.functionCode >> 4) & 0x0F;
    low = message->pdu.functionCode & 0x0F;
    buffer[2] = nibbleToASCII(high);
    buffer[3] = nibbleToASCII(low);

    size_t i, j;
    for (i = 0, j = 4; i < message->pdu.size; ++i, j += 2)
    {
        buffer[j] = nibbleToASCII((message->pdu.data[i] >> 4) & 0x0F);
        buffer[j + 1] = nibbleToASCII(message->pdu.data[i] & 0x0F);
    }

    high = (message->checksum >> 4) & 0x000F;
    low = message->checksum & 0x000F;
    buffer[j] = nibbleToASCII(high);
    buffer[j + 1] = nibbleToASCII(low);

    return j + 2;
}

void translateFromASCIIStream(
    const uint8_t* message, size_t size, struct ModbusMessage* outMessage)
{
    outMessage->address = ASCIIToByte(message[0], message[1]);
    outMessage->pdu.functionCode = ASCIIToByte(message[2], message[3]);
    outMessage->checksum = ASCIIToByte(message[size - 2], message[size - 1]);

    int lastIndex = size - 2;
    int i, j;
    for (i = 4, j = 0; i < lastIndex; i += 2, ++j)
        outMessage->pdu.data[j] = ASCIIToByte(message[i], message[i + 1]);
    outMessage->pdu.size = (size - 6) / 2;
}

uint8_t nibbleToASCII(uint8_t nibble)
{
    if (nibble >= 0 && nibble <= 9)
        return nibble + 0x30;
    else if (nibble >= 10 && nibble <= 15)
        return nibble + 0x37;
    else
        return 0;
}

uint8_t ASCIIToByte(uint8_t high, uint8_t low)
{
    uint8_t value = 0;
    if (high >= 0x30 && high <= 0x39)
        value = ((high - 0x30) << 4) & 0xF0;
    else
        value = ((high - 0x37) << 4) & 0xF0;

    if (low >= 0x30 && low <= 0x39)
        value |= (low - 0x30) & 0x0F;
    else
        value |= (low - 0x37) & 0x0F;

    return value;
}

void BuildRequest(
    struct ModbusMessage* msg,
    uint8_t address,
    enum ModbusFunction functionCode,
    uint16_t startingAddress,
    uint16_t value)
{
    msg->address = address;
    msg->pdu.functionCode = functionCode;
    msg->pdu.data[0] = (startingAddress >> 8) & 0x00FF;
    msg->pdu.data[1] = startingAddress & 0x00FF;
    msg->pdu.data[2] = (value >> 8) & 0x00FF;
    msg->pdu.data[3] = value & 0x00FF;
    msg->pdu.size = 4;
    msg->checksum = LRC(msg); 
}

uint8_t LRC(struct ModbusMessage* message)
{
    uint8_t lrc = 0;
    lrc = message->address + message->pdu.functionCode;
    size_t i;
    for (i = 0; i < message->pdu.size; ++i)
        lrc += message->pdu.data[i];

    return (lrc ^ 0xFF) + 1;
}

uint16_t CRC16(uint8_t* data, size_t size)
{
	static const uint16_t CRC_POLY = 0xa001;
	uint16_t crc = 0xffff;
	uint16_t dataArrayIndex = 0;

	for (; dataArrayIndex < size; ++dataArrayIndex)
	{
		crc ^= data[dataArrayIndex];
		uint8_t bitCount = 0;
		for (; bitCount < 8; ++bitCount)
		{
			if (crc & 1)
			{
				crc >>= 1;
				crc ^= CRC_POLY;
			}
			else
				crc >>= 1;
		}
	}
	return crc;
}