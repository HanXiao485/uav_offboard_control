#include <termios.h>
#include <unistd.h>
#include <fcntl.h>

class KeyboardHandler {
public:
    KeyboardHandler();
    ~KeyboardHandler();
    char getKey();

private:
    struct termios initial_settings;
    int initKeyboard();
    void closeKeyboard();
    int kbhit();
};