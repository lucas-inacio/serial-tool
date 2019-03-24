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

int poll_settings(Ihandle* self)
{
    IupPopup(IupGetHandle("settings_dialog"), IUP_CENTERPARENT, IUP_CENTERPARENT);
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
    Ihandle *hbox = NULL;
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
            hbox = create_tab_modbus();
        }
        break;
    case CHOICE_SERIAL:
        {
            hbox = create_tab_serial();
        }
        break;
    }

    IupSetAttribute(hbox, "TABTITLE", title);
    IupSetAttribute(hbox, "GAP", "5");
    // Inserts the new widgets on the Tabs
    IupAppend(tabs, hbox);
    IupMap(hbox);

    // Updates the main window
    IupRefresh(IupGetHandle("main"));
}

Ihandle *create_tab_serial()
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
    return IupVbox(text_write, text_read, NULL);
}

Ihandle *create_tab_modbus()
{
    Ihandle *label_msg = IupLabel("Message");
    Ihandle *label_addr = IupLabel("Address");
    Ihandle *label_start = IupLabel("Start");
    Ihandle *label_quantity = IupLabel("Quantity");
    Ihandle *label_data = IupLabel("Value");

    Ihandle *type = IupList(NULL);
    IupSetAttribute(type, "DROPDOWN", "YES");
    IupSetAttribute(type, "SIZE", "100x10");
    IupSetAttribute(type, "1", "Write Single Coil");
    IupSetAttribute(type, "2", "Write Single Register");
    IupSetAttribute(type, "3", "Write Multiple Coils");
    IupSetAttribute(type, "4", "Write Multiple Registers");

    Ihandle *addr = IupText(NULL);
    Ihandle *start = IupText(NULL);
    Ihandle *quantity = IupText(NULL);
    Ihandle *data = IupText(NULL);
    IupSetAttribute(addr, "SPIN", "YES");
    IupSetAttribute(start, "SPIN", "YES");
    IupSetAttribute(quantity, "SPIN", "YES");
    IupSetAttribute(data, "SPIN", "YES");

    // Give names to handles so we can refer to them later
    IupSetHandle("msg_type", type);
    IupSetHandle("addr_spin", addr);
    IupSetHandle("start_spin", start);
    IupSetHandle("quantity_spin", quantity);
    IupSetHandle("data_spin", data);    
    Ihandle *send_button = IupButton("Send", NULL);
    IupSetCallback(send_button, "ACTION", (Icallback)send_callback);

    Ihandle *functions = IupVbox(
        label_msg, type,
        label_addr, addr,
        label_start, start,
        label_quantity, quantity,
        label_data, data,
        send_button, NULL);
    IupSetAttribute(functions, "MINSIZE", "256x400");

    Ihandle *matrix = IupMatrix(NULL);
    return IupHbox(matrix, functions, NULL);
}

int send_callback(Ihandle *ih)
{
    int choice = atoi(IupGetAttribute(IupGetHandle("msg_type"), "VALUE"));
    int address = atoi(IupGetAttribute(IupGetHandle("addr_spin"), "VALUE"));
    int start = atoi(IupGetAttribute(IupGetHandle("start_spin"), "VALUE"));
    int quantity = atoi(IupGetAttribute(IupGetHandle("quantity_spin"), "VALUE"));
    int data = atoi(IupGetAttribute(IupGetHandle("data_spin"), "VALUE"));
    int index = atoi(IupGetAttribute(tabs, "VALUEPOS"));

    switch (choice)
    {
    case 1:
        QueueRequest(
            &modbus_queue, WRITE_SINGLE_COIL, serialports[index].port,
            address, start, (data != 0) ? COIL_ON : COIL_OFF);
        break;
    case 2:
        QueueRequest(
            &modbus_queue, WRITE_SINGLE_REGISTER, serialports[index].port,
            address, start, data);
        break;
    case 3:
        break;
    case 4:
        break;
    }

    return IUP_DEFAULT;
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