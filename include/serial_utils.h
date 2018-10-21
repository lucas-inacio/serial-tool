#ifndef SERIAL_UTILS_H
#define SERIAL_UTILS_H

#include <libserialport.h>
#include <libserialport_internal.h>

#define MAXIMUM_PORTS 10
#define TEMP_BUFFER   512

struct SerialPort
{
    char *_InputBuffer;
    int _InputBufferSize;
    int _InputBufferCount;

    char *_OutputBuffer;
    int _OutputBufferSize;
    int _OutputBufferCount;

    struct sp_port *port;
};

// Fills up ports with the available ports and returns
// the number of detected ports
int detect_ports(struct sp_port ***ports);

// Displays a numbered list of ports
void show_port_names(struct sp_port **ports, int count);

// Configures the port with the given parameters
void config_port(
    struct sp_port *port, int baud, int bits,
    enum sp_parity parity, int stopbits);

struct sp_port *open_port(
    const char *name, int baud, int bits,
    enum sp_parity parity, int stopbits);

void find_port_by_name(const char *name, struct sp_port **copy);

struct SerialPort *OpenSerialPort(
    const char *name, int baud, int bits,
    enum sp_parity parity, int stopbits,
    int inbuffersize, int outbuffersize);

void CloseSerialPort(struct SerialPort *port);

int ReadSerialPort(struct SerialPort *port);
int WriteSerialPort(struct SerialPort *port);
int ReadSerialBuffer(struct SerialPort *port, char *buffer, int count);
int WriteSerialBuffer(struct SerialPort *port, char *buffer, int count);

#endif // SERIAL_UTILS_H