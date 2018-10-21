#include "serial_utils.h"
#include "ui_utils.h"

#include <stdio.h>


Ihandle *tabs = NULL;
int tabs_count = 0;

struct SerialPort *serialports[MAXIMUM_PORTS];
int serialcount = 0;


// Serial callback
int serial_loop(void);

int main(int argc, char **argv)
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

    // Main loop
    IupShow(IupGetHandle("main"));
    IupMainLoop();

    // Clean up
    IupDestroy(IupGetHandle("main"));
    IupClose();
    for (--serialcount; serialcount >= 0; --serialcount)
        CloseSerialPort(serialports[serialcount]);
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
        if (ReadSerialPort(serialports[i]) > 0)
        {
            int count = TEXT_SIZE; // maximum bytes to read

            // If \r is the last (or only) character available, skip reading the buffer
            // The purpose is to get both \r and \n together
            int last_index = serialports[i]->_InputBufferCount - 1;
            if (serialports[i]->_InputBuffer[last_index] == '\r')
                count = serialports[i]->_InputBufferCount - 1;

            // Only character available (or no caracter at all). Skip
            if (count != 0)
            {
                int read = ReadSerialBuffer(serialports[i], buffer, count);
                Ihandle *vbox = IupGetChild(tabs, i);
                Ihandle *text_read = IupGetChild(vbox, 1);
                IupSetAttribute(text_read, "APPEND", buffer);
                const char *caret = IupGetAttribute(text_read, "COUNT");
                IupSetAttribute(text_read, "CARETPOS", caret);
            }
        }

        // Write operation
        WriteSerialPort(serialports[i]);
    }

    return IUP_DEFAULT;
}