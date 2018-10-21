#ifndef UI_UTILS_H
#define UI_UTILS_H

#include <iup/iup.h>
#include <iup/iupcontrols.h>

#define TEXT_SIZE 512
#define CARRIAGE_RETURN 13


// UI callbacks (Main window)
int action_exit(Ihandle *self);
int action_new(Ihandle *self);

// UI callbacks (Comm configuration dialog)
int action_config_ok(Ihandle *self);
int action_config_cancel(Ihandle *self);
int action_tab_close(Ihandle *self, int pos);

// UI callbacks (Text)
int text_entered(Ihandle *self, int c, char *new_value);

// Helper functions
void create_tab(const char *title);
void open_tab(const char *title);
void close_tab(int index);

#endif // UI_UTILS_H