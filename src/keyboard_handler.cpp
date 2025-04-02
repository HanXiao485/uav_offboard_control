#include "uav_keyboard_control/keyboard_handler.h"
#include <sys/time.h>

KeyboardHandler::KeyboardHandler() {
    tcgetattr(0, &initial_settings);
    struct termios new_settings = initial_settings;
    new_settings.c_lflag &= ~ICANON;
    new_settings.c_lflag &= ~ECHO;
    tcsetattr(0, TCSANOW, &new_settings);
}

KeyboardHandler::~KeyboardHandler() {
    tcsetattr(0, TCSANOW, &initial_settings);
}

int KeyboardHandler::initKeyboard() {
    return fcntl(0, F_SETFL, O_NONBLOCK);
}

char KeyboardHandler::getKey() {
    char ch = 0;
    if(read(0, &ch, 1) > 0) return ch;
    return 0;
}

int KeyboardHandler::kbhit() {
    struct timeval tv;
    fd_set fds;
    tv.tv_sec = 0;
    tv.tv_usec = 0;
    FD_ZERO(&fds);
    FD_SET(0, &fds);
    return select(1, &fds, NULL, NULL, &tv);
}