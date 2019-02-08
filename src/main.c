#include "modbus_utils.h"
#include "serial_utils.h"
#include "ui_utils.h"


Ihandle *tabs = NULL;
int tabs_count = 0;

// struct SerialPort *serialports[MAXIMUM_PORTS];
struct CommDescriptor serialports[MAXIMUM_PORTS];
int serialcount = 0;

// Serial callback
int serial_loop(void);
void main_loop(int argc, char **argv);
void print_registers(uint8_t* data, size_t size);


struct SerialPort *serial = NULL;
void teste_modbus(void);

int main(int argc, char **argv)
{
    main_loop(argc, argv);
    // teste_modbus();
    return EXIT_SUCCESS;
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

            // If \r is the last (or only) character available, skip reading the buffer
            // The purpose is to get both \r and \n together and avoid printing two new lines
            if (serialports[i].type == SERIAL)
            {
                int last_index = serialports[i].port->_InputBufferCount - 1;
                if (serialports[i].port->_InputBuffer[last_index] == '\r')
                    count = serialports[i].port->_InputBufferCount - 1;

                // Only character available (or no character at all). Skip
                if (count != 0)
                {
                    int read = ReadSerialBuffer(serialports[i].port, buffer, count);
                    Ihandle *vbox = IupGetChild(tabs, i);
                    Ihandle *text_read = IupGetChild(vbox, 1);
                    IupSetAttribute(text_read, "APPEND", buffer);
                    const char *caret = IupGetAttribute(text_read, "COUNT");
                    IupSetAttribute(text_read, "CARETPOS", caret);
                }
            }
        }

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

    // Main loop
    IupShow(IupGetHandle("main"));
    IupMainLoop();

    // Clean up
    IupDestroy(IupGetHandle("main"));
    IupClose();
    for (--serialcount; serialcount >= 0; --serialcount)
        CloseSerialPort(serialports[serialcount].port);
}

void teste_modbus(void)
{
    printf("Testando...\n");
    uint8_t buffer[512] = { 0 };
    uint8_t bufferFinal[512] = { 0 };
    struct ModbusMessage msg;
    msg.pdu.data = buffer;
    // Coil off
    BuildRequest(&msg, 1, WRITE_SINGLE_COIL, 0, COIL_OFF);
    size_t size = translateToASCIIStream(&msg, &bufferFinal[1]);
    bufferFinal[0] = ':';
    bufferFinal[size + 1] = '\r';
    bufferFinal[size + 2] = '\n';
    printf("Size: %d", size);
    printf("Message: %s\n", bufferFinal);

    // Send
    size_t bytesToSend = strlen(bufferFinal);
    size_t bytesSent = 0;
    serial = OpenSerialPort("COM3", 9600, 8, SP_PARITY_NONE, 2, 512, 512);
    printf("Bytes sent: %d\n", WriteSerialBuffer(serial, bufferFinal, bytesToSend));
    do
    {
        bytesSent += WriteSerialPort(serial);
    }
    while (bytesSent < bytesToSend);

    // Receive
    size_t bytesReceived = 0;
    int i;
    for (i = 0; i < 100000; ++i)
        bytesReceived += ReadSerialPort(serial);

    if (bytesReceived > 0)
    {
        // printf("Data size: %d\n", ReadSerialBuffer(serial, bufferFinal, bytesReceived));
        // printf("Data: %s\n", bufferFinal);
        // // Eliminate the ':' and the 'CR' and 'LF' bytes
        // translateFromASCIIStream(&bufferFinal[1], bytesReceived - 3, &msg);
        // printf("Address: %d\n", msg.address);
        // printf("Function code: %d\n", msg.pdu.functionCode);
        // printf("Checksum: %d\n", msg.checksum);
        // printf("Size: %d\n", msg.pdu.size);
        // print_registers(&msg.pdu.data[1], msg.pdu.size - 1);
    }
    scanf("Prosseguir?\n");
    CloseSerialPort(serial);
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