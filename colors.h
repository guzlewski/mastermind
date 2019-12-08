#ifndef __COLORS_H
#define __COLORS_H

#include "shared.h"

#ifndef NOCOLOR

#define BLACK_TEXT(x) "\033[30;1m" x "\033[0m"
#define RED_TEXT(x) "\033[31;1m" x "\033[0m"
#define GREEN_TEXT(x) "\033[32;1m" x "\033[0m"
#define YELLOW_TEXT(x) "\033[33;1m" x "\033[0m"
#define BLUE_TEXT(x) "\033[34;1m" x "\033[0m"
#define MAGENTA_TEXT(x) "\033[35;1m" x "\033[0m"
#define CYAN_TEXT(x) "\033[36;1m" x "\033[0m"
#define WHITE_TEXT(x) "\033[37;1m" x "\033[0m"

#else

#define BLACK_TEXT(x) x
#define RED_TEXT(x) x
#define GREEN_TEXT(x) x
#define YELLOW_TEXT(x) x
#define BLUE_TEXT(x) x
#define MAGENTA_TEXT(x) x
#define CYAN_TEXT(x) x
#define WHITE_TEXT(x) x

#endif

#endif