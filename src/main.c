#define MAIN_FILE
#include "globals.h"
#include "ui_utils.h"

#include <stdlib.h>


Ihandle *tabs = NULL;
struct CommDescriptor serialports[MAXIMUM_PORTS];
int serialcount = 0;

// Time in seconds
double delta();
void main_loop(int argc, char **argv);
// Serial callback
int serial_loop(void);
void print_registers(uint8_t* data, size_t size);

int main(int argc, char **argv)
{
    modbus_queue = NULL;
    main_loop(argc, argv);
    // teste_modbus();
    return EXIT_SUCCESS;
}

double delta()
{
    static clock_t last = 0;
    static int first = 1;

    double diff = 0.0;
    if (!first)
    {
        clock_t now = clock();
        diff = (double)(now - last) / CLOCKS_PER_SEC;
        last = now;
    }
    else
    {
        last = clock();
        first = 0;
    }
    return diff;
}

void read_serial(int index, uint8_t *buffer, size_t size)
{
    // If \r is the last (or only) character available, skip reading the buffer
    // The purpose is to get both \r and \n together and avoid printing two new lines
    int count = size;
    int last_index = serialports[index].port->_InputBufferCount - 1;
    if (serialports[index].port->_InputBuffer[last_index] == '\r')
        count = serialports[index].port->_InputBufferCount - 1;

    // Only character available (or no character at all). Skip
    if (count != 0)
    {
        int read = ReadSerialBuffer(serialports[index].port, buffer, count);
        Ihandle *vbox = IupGetChild(tabs, index);
        Ihandle *text_read = IupGetChild(vbox, 1);
        IupSetAttribute(text_read, "APPEND", buffer);
        const char *caret = IupGetAttribute(text_read, "COUNT");
        IupSetAttribute(text_read, "CARETPOS", caret);
    }
}

void read_modbus_ascii(int index, uint8_t *buffer, size_t size)
{
    // TODO: support read in chunks
    int count = size;
    int offset = serialports[index].port->_InputBufferCount - 2;
    const uint8_t *data = &serialports[index].port->_InputBuffer[offset];
    if (*data == '\r' && *(data + 1) == '\n')
    {
        struct ModbusMessage msg;
        int bytesReceived = ReadSerialBuffer(serialports[index].port, buffer, count);
        translateFromASCIIStream(&buffer[1], bytesReceived - 3, &msg);
        print_registers(&msg.pdu.data[1], msg.pdu.size - 1);
        printf("Row data: %s\n", buffer);
        printf("Data size: %d\n", bytesReceived);
    }
}

// Add limit to the number of serial ports checked in a single call
int serial_loop(void)
{
    int i = 0;
    for (; i < serialcount; ++i)
    {
        // Read operation
        char buffer[TEXT_SIZE + 1] = { 0 };
        if (ReadSerialPort(serialports[i].port) > 0)
        {
            int count = TEXT_SIZE; // maximum bytes to read
            if (serialports[i].type == SERIAL)
                read_serial(i, buffer, TEXT_SIZE);
            else if (serialports[i].type == MODBUS_ASCII)
                read_modbus_ascii(i, buffer, TEXT_SIZE);
        }
        if (modbus_queue) SendModbusMessage(&modbus_queue);
        // Write operation
        WriteSerialPort(serialports[i].port);
    }

    return IUP_DEFAULT;
}

void main_loop(int argc, char** argv)
{
    const char* error = NULL;
    IupOpen(&argc, &argv);
    IupControlsOpen();
    if ((error = IupLoad("ui.led")) != NULL)
    {
        IupMessage("Error", error);
        exit(1);
    }

    // Callbacks
    IupSetFunction("action_exit", (Icallback)action_exit);
    IupSetFunction("action_new", (Icallback)action_new);
    IupSetFunction("comm_config_ok", (Icallback)action_config_ok);
    IupSetFunction("comm_config_cancel", (Icallback)action_config_cancel);
    IupSetFunction("IDLE_ACTION", (Icallback)serial_loop);
    IupSetFunction("action_about", (Icallback)action_about);
    IupSetFunction("poll_settings", (Icallback)poll_settings);

    // Main loop
    IupShow(IupGetHandle("main"));
    IupMainLoop();

    // Clean up
    IupDestroy(IupGetHandle("main"));
    IupClose();
    for (--serialcount; serialcount >= 0; --serialcount)
        CloseSerialPort(serialports[serialcount].port);
}

void print_registers(uint8_t* data, size_t size)
{
    size_t i;
    for (i = 0; i < (size - 1); i += 2)
    {
        uint16_t number;
        number = ((data[i] << 8) & 0xFF00) | (data[i + 1] & 0x00FF);
        printf("Data: %d\n", number);
    }
}

struct ModbusQueue *add_item(struct ModbusQueue **queue, struct ModbusMessage *msg)
{
    // Go to the end
    if (!(*queue))
    {
        (*queue) = malloc(sizeof(struct ModbusQueue));
        (*queue)->next = NULL;
        (*queue)->msg = msg;
        (*queue)->time_since_request = 0;
        (*queue)->total = 0;
        (*queue)->remaining = 0;
        return *queue;
    }
    else
    {  
        struct ModbusQueue *first = *queue;
        while (first->next) first = first->next;
        // Creates a new item and adds it to the end
        struct ModbusQueue *next = malloc(sizeof(struct ModbusQueue));
        next->msg = msg;
        next->time_since_request = 0;
        next->next = NULL;
        next->total = 0;
        next->remaining = 0;
        first->next = next;
        return next;
    }
}

void remove_item(struct ModbusQueue **queue)
{
    struct ModbusQueue *next = (*queue)->next;
    free(*queue);
    *queue = next;
}

void QueueRequest(
    struct ModbusQueue **queue,
    enum ModbusFunction function,
    struct SerialPort *port,
    uint8_t id, uint16_t start, uint16_t quantity)
{
    struct ModbusMessage *msg = malloc(sizeof(struct ModbusMessage));
    BuildRequest(msg, id, function, start, quantity);
    struct ModbusQueue *next = add_item(queue, msg);
    next->port = port;
}

MODIFIER void SendModbusMessage(struct ModbusQueue **queue)
{
    static uint8_t _buffer[64] = { 0 };

    if (*queue)
    {
        if ((*queue)->total == 0)
        {
            size_t size = translateToASCIIStream((*queue)->msg, &_buffer[1]);
            _buffer[0] = ':';
            _buffer[size + 1] = '\r';
            _buffer[size + 2] = '\n';
            (*queue)->total = size + 3;
            (*queue)->remaining = size + 3;
        }

        size_t total = (*queue)->total;
        size_t remaining = (*queue)->remaining;
        (*queue)->remaining -=
            WriteSerialBuffer((*queue)->port, &_buffer[total - remaining], remaining);

        if ((*queue)->remaining == 0)
            DestroyRequest(queue);
    }
}

void DestroyRequest(struct ModbusQueue **queue)
{
    free((*queue)->msg);
    remove_item(queue);
}