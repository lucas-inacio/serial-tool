#include "serial_utils.h"


void show_port_names(struct sp_port **ports, int count)
{
    int i;
    for (i = 0; i < count; ++i)
        printf("%d - %s\n", i, ports[i]->name);
}

int detect_ports(struct sp_port ***ports)
{
    int count = 0;
    enum sp_return result = sp_list_ports(ports);
    if (result == SP_OK)
    {
        struct sp_port **port_array = *ports;
        while (*port_array != NULL)
        {
            ++count;
            ++port_array;
        }
    }

    return count;
}

void config_port(
    struct sp_port *port, int baud, int bits,
    enum sp_parity parity, int stopbits)
{
    struct sp_port_config *port_config;
    sp_new_config(&port_config);

    if (sp_get_config(port, port_config) != SP_OK) printf("Bad\n");
    port_config->baudrate = baud;
    port_config->bits = bits;
    port_config->parity = parity;
    port_config->stopbits = stopbits;
    sp_set_config(port, port_config);
}

struct sp_port *open_port(
    const char *name, int baud, int bits,
    enum sp_parity parity, int stopbits)
{
    struct sp_port* port = NULL;
    find_port_by_name(name, &port);
    sp_open(port, SP_MODE_READ_WRITE);
    config_port(port, baud, bits, parity, stopbits);
    return port;
}

void find_port_by_name(const char *name, struct sp_port **copy)
{
    struct sp_port** ports = NULL;
    int count = detect_ports(&ports);

    int i;
    for (i = 0; i < count; ++i)
    {
        if (strcmp(ports[i]->name, name) == 0)
        {
            sp_copy_port(ports[i], copy);
            break;
        }
    }

    sp_free_port_list(ports);
}

struct SerialPort *OpenSerialPort(
    const char* name, int baud, int bits,
    enum sp_parity parity, int stopbits,
    int inbuffersize, int outbuffersize)
{
    struct SerialPort *port = malloc(sizeof(struct SerialPort));
    port->_InputBuffer = calloc(inbuffersize, sizeof(char));
    port->_OutputBuffer = calloc(outbuffersize, sizeof(char));
    port->_InputBufferSize = inbuffersize;
    port->_OutputBufferSize = outbuffersize;
    port->_InputBufferCount = 0;
    port->_OutputBufferCount = 0;
    port->port = open_port(name, baud, bits, parity, stopbits);

    return port;
}

void CloseSerialPort(struct SerialPort *port)
{
    if (port->_InputBuffer != NULL) free(port->_InputBuffer);
    if (port->_OutputBuffer != NULL) free(port->_OutputBuffer);
    if (port->port != NULL)
    {
        sp_close(port->port);
        sp_free_port(port->port);
    }
    free(port);
}

int ReadSerialPort(struct SerialPort *port)
{
    int bytesread = 0;

    if (port->_InputBufferCount > 0 &&
        port->_InputBufferCount < port->_InputBufferSize)
    {
        bytesread = sp_nonblocking_read(
            port->port,
            &port->_InputBuffer[port->_InputBufferCount],
            port->_InputBufferSize - port->_InputBufferCount);
    }
    else
    {
        port->_InputBufferCount = 0;
        bytesread = sp_nonblocking_read(
            port->port, port->_InputBuffer, port->_InputBufferSize);
    }
    if (bytesread >= 0) port->_InputBufferCount += bytesread;

    return bytesread;
}

int WriteSerialPort(struct SerialPort *port)
{
    int byteswritten = 0;
    if (port->_OutputBufferCount > 0)
    {
        byteswritten = sp_nonblocking_write(
            port->port, port->_OutputBuffer, port->_OutputBufferCount);
        port->_OutputBufferCount -= byteswritten;

        if (port->_OutputBufferCount > 0)
        {
            if (byteswritten > 0)
            {
                memmove(port->_OutputBuffer,
                        &port->_OutputBuffer[byteswritten],
                        port->_OutputBufferCount);
            }
        }
    }

    return byteswritten;
}

int ReadSerialBuffer(struct SerialPort *port, char *buffer, int count)
{
    if (count > port->_InputBufferCount) count = port->_InputBufferCount;
    memcpy(buffer, port->_InputBuffer, count);
    int remaining = port->_InputBufferCount - count;

    if (remaining > 0)
        memmove(port->_InputBuffer, &port->_InputBuffer[count], remaining);

    port->_InputBufferCount = remaining;
    return count;
}

int WriteSerialBuffer(struct SerialPort *port, char *buffer, int count)
{
                      if (count > port->_OutputBufferSize) count = port->_OutputBufferSize;
    memcpy(port->_OutputBuffer, buffer, count);
    port->_OutputBufferCount = count;

    return count;
}