#ifndef UI_UTILS_H
#define UI_UTILS_H

#include <iup/iup.h>
#include <iup/iupcontrols.h>

#define TEXT_SIZE 512
#define CARRIAGE_RETURN 13

#define CHOICE_MASCII 0
#define CHOICE_MRTU   1
#define CHOICE_SERIAL 2

#define SERIAL_CHOICE_STR ("serial")
#define ASCII_CHOICE_STR  ("modbus_ascii")
#define RTU_CHOICE_STR    ("modbus_rtu")


// UI callbacks (Main window)
int action_exit(Ihandle *self);
int action_new(Ihandle *self);
int action_about(Ihandle *self);
int poll_settings(Ihandle* self);

// UI callbacks (Comm configuration dialog)
int action_config_ok(Ihandle *self);
int action_config_cancel(Ihandle *self);
int action_tab_close(Ihandle *self, int pos);

// UI callbacks (Text)
int text_entered(Ihandle *self, int c, char *new_value);

// Helper functions
void create_tab(const char *title, int choice);
Ihandle *create_tab_serial();
Ihandle *create_tab_modbus();
int send_callback(Ihandle *ih);

void open_tab(const char *title);
void close_tab(int index);
int change_tab(Ihandle *self, Ihandle* new_tab, Ihandle* old_tab);

// Enables modbus menu when in a modbus tab.
void enable_modbus_menu();
void disable_modbus_menu();

int get_choice_radio();

#endif // UI_UTILS_H