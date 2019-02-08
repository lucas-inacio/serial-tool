#include "ui_utils.h"
#include "serial_utils.h"
#include "string_utils.h"

#include <libserialport.h>
#include <libserialport_internal.h>

#include "globals.h"

// UI Callbacks (Main window)
int action_exit(Ihandle* self)
{
    return IUP_CLOSE;
}

int action_new(Ihandle* self)
{
    if (serialcount >= MAXIMUM_PORTS)
        return IUP_IGNORE;

    char index_str[4] = { 0 };
    struct sp_port** ports;
    int count = detect_ports(&ports);

    if (count > 0)
    {
        Ihandle* list = IupGetHandle("comm_port_list");
        IupSetAttribute(list, "1", NULL); // Removes previous contents

        int i;
        for (i = 0; i < count; ++i)
        {
            number_to_string(index_str, 3, i + 1);
            IupSetAttribute(list, index_str, ports[i]->name);
        }
        IupSetAttribute(list, "VALUE", "1"); // Selects first item
        IupPopup(IupGetHandle("comm_dialog"), IUP_CENTERPARENT, IUP_CENTERPARENT);
        sp_free_port_list(ports);
    }
    else
    {
        IupMessage("Info", "No ports available");
    }

    return IUP_IGNORE;
}

int action_about(Ihandle *self)
{
    char buffer[20] = { 0 };
    sprintf(buffer, "Version %d.%d",
            SerialTool_VERSION_MAJOR,
            SerialTool_VERSION_MINOR);
    IupMessage("SerialTool", buffer);
    return IUP_DEFAULT;
}

// UI callbacks (Comm configuration dialog)
int action_config_ok(Ihandle* self)
{
    if (serialcount < sizeof(serialports))
    {
        Ihandle* port_list = IupGetHandle("comm_port_list");
        const char* port_index = IupGetAttribute(port_list, "VALUE");
        const char* port_name = IupGetAttribute(port_list, port_index);
        // Checks if the port is already in use
        int i = 0;
        for (; i < serialcount; ++i)
        {
            // TODO: Free memory here
            char *name = sp_get_port_name(serialports[i].port->port);
            if (strcmp(port_name, name) == 0) // Port already in use
            {
                return IUP_CLOSE;
            }
        }
        open_tab(port_name);
    }

    return IUP_CLOSE;
}

int action_config_cancel(Ihandle *self)
{
    return IUP_CLOSE;
}

int action_tab_close(Ihandle *self, int pos)
{
    if (serialcount > 1)
    {
        int prev = (pos > 0) ? pos - 1 : 1;
        char number[3] = { 0 };
        number_to_string(number, 2, prev);
        IupSetAttribute(tabs, "VALUEPOS", number);
        if (serialports[prev].type != SERIAL)
            enable_modbus_menu();
        else
            disable_modbus_menu();
    }
    else
        disable_modbus_menu();
        
    IupDestroy(IupGetChild(tabs, pos));
    close_tab(pos);
    return IUP_DEFAULT;
}

void create_tab(const char* title, int choice)
{
    // Creates a Tabs in case there isn't one yet
    Ihandle* main_area = IupGetHandle("main_area");
    Ihandle *vbox = NULL;
    if (tabs == NULL)
    {
        tabs = IupTabs(NULL);
        IupSetCallback(tabs, "TABCHANGE_CB", (Icallback)change_tab);
        IupSetCallback(tabs, "RIGHTCLICK_CB", (Icallback)action_tab_close);
        IupSetAttribute(tabs, "MARGIN", "3x5");
        IupAppend(main_area, tabs);
        IupMap(tabs);
    }

    switch (choice)
    {
    case CHOICE_MASCII:
    case CHOICE_MRTU:
        {
            Ihandle *matrix = IupMatrix(NULL);
            vbox = IupVbox(matrix, NULL);
        }
        break;
    case CHOICE_SERIAL:
        {
            Ihandle *text_read = IupText(NULL);
            IupSetAttribute(text_read, "MULTILINE", "YES");
            IupSetAttribute(text_read, "EXPAND", "YES");
            IupSetAttribute(text_read, "READONLY", "YES");
            IupSetAttribute(text_read, "APPENDNEWLINE", "NO");

            Ihandle *text_write = IupText(NULL);
            IupSetAttribute(text_write, "EXPAND", "HORIZONTAL");
            char str[4] = { 0 };
            number_to_string(str, 3, TEXT_SIZE);
            IupSetAttribute(text_write, "NC", str);
            IupSetCallback(text_write, "K_ANY", (Icallback)text_entered);
            vbox = IupVbox(text_write, text_read, NULL);
        }
        break;
    }

    IupSetAttribute(vbox, "TABTITLE", title);
    IupSetAttribute(vbox, "GAP", "5");
    // Inserts the new widgets on the Tabs
    IupAppend(tabs, vbox);
    IupMap(vbox);

    // Updates the main window
    IupRefresh(IupGetHandle("main"));
}

void open_tab(const char *title)
{
    // Get user's choices
    Ihandle *baud_list = IupGetHandle("comm_baud_list");
    Ihandle *parity_list = IupGetHandle("comm_parity_list");
    Ihandle *data_list = IupGetHandle("comm_data_list");
    Ihandle *stop_list = IupGetHandle("comm_stop_list");
    Ihandle *radio = IupGetHandle("type_radio");

    const char *baud_index = IupGetAttribute(baud_list, "VALUE");
    const char *parity_index = IupGetAttribute(parity_list, "VALUE");
    const char *data_index = IupGetAttribute(data_list, "VALUE");
    const char *stop_index = IupGetAttribute(stop_list, "VALUE");
    const char *type = IupGetAttribute(radio, "VALUE");

    int baudrate = atoi(IupGetAttribute(baud_list, baud_index));
    int bits = atoi(IupGetAttribute(data_list, data_index));
    int stopbits = atoi(IupGetAttribute(stop_list, stop_index));
    // ATTENTION: code assumes the specific order NONE = 0, ODD and EVEN
    enum sp_parity parity = atoi(parity_index) - 1;

    // The order is important here (serialcount)
    if (strcmp(type, SERIAL_CHOICE_STR) == 0)
        serialports[serialcount].type = SERIAL;
    else if (strcmp(type, ASCII_CHOICE_STR) == 0)
        serialports[serialcount].type = MODBUS_ASCII;
    else if (strcmp(type, RTU_CHOICE_STR) == 0)
        serialports[serialcount].type = MODBUS_RTU;

    serialports[serialcount++].port =
        OpenSerialPort(title, baudrate, bits, parity, stopbits, TEXT_SIZE, TEXT_SIZE);

    if (serialcount == 1 && serialports[0].type != SERIAL)
        enable_modbus_menu();    

    // Deal with error else;
    create_tab(title, get_choice_radio());
}

void close_tab(int index)
{
    if (index < 0 || index >= sizeof(serialports)) return;

    CloseSerialPort(serialports[index].port);
    --serialcount;
    if (index < (sizeof(serialports) - 1))
        memmove(&serialports[index], &serialports[index + 1],
                sizeof(serialports) - index);
}

int text_entered(Ihandle *self, int c, char *new_value)
{
    if (c == CARRIAGE_RETURN) // Enter
    {
        // value does not need to be freed. IUP will take care of it
        char *value = IupGetAttribute(self, "VALUE");
        if (value != NULL)
        {
            int index = atoi(IupGetAttribute(tabs, "VALUEPOS"));
            if (index >= 0)
            {
                WriteSerialBuffer(
                    serialports[index].port, value, strlen(value));
                IupSetAttribute(self, "VALUE", "");
            }
        }
    }
    return IUP_DEFAULT;
}

int change_tab(Ihandle *self, Ihandle* new_tab, Ihandle* old_tab)
{
    int index = atoi(IupGetAttribute(tabs, "VALUEPOS"));
    if ((index >= 0 && index < serialcount) &&
        (serialports[index].type != SERIAL))
        enable_modbus_menu();
    else
        disable_modbus_menu();
    
    return IUP_DEFAULT;
}

void enable_modbus_menu()
{
    Ihandle *modbus_menu = IupGetHandle("modbus");
    IupSetAttribute(modbus_menu, "ACTIVE", "YES");
}

void disable_modbus_menu()
{
    Ihandle *modbus_menu = IupGetHandle("modbus");
    IupSetAttribute(modbus_menu, "ACTIVE", "NO");
}

int get_choice_radio()
{
    int result = CHOICE_MASCII;
    Ihandle *radio = IupGetHandle("type_radio");
    Ihandle *mascii = IupGetHandle("modbus_ascii");
    Ihandle *mrtu = IupGetHandle("modbus_rtu");
    Ihandle *serial = IupGetHandle("serial");
    Ihandle *value = (Ihandle *)IupGetAttribute(radio, "VALUE_HANDLE");

    if (value == mascii)
        result = CHOICE_MASCII;
    else if (value == mrtu)
        result = CHOICE_MRTU;
    else if (value == serial)
        result = CHOICE_SERIAL;

    return result;
}