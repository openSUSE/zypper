/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/

#ifndef UTILS_COLORS_H_
#define UTILS_COLORS_H_

#include <iosfwd>

#define COLOR_GREEN             "\033[32m"
#define COLOR_GREEN_BOLD        "\033[1;32m"
#define COLOR_RED               "\033[31m"
#define COLOR_RED_BOLD          "\033[1;31m"
#define COLOR_WHITE             "\033[37m"    // grey
#define COLOR_WHITE_BOLD        "\033[1;37m"
#define COLOR_YELLOW            "\033[33m"    // brown
#define COLOR_YELLOW_BOLD       "\033[1;33m"

#define COLOR_RESET             "\033[m"

bool has_colors();
void print_color(const std::string & s, const char * ansi_color_seq);

#endif /* UTILS_COLORS_H_ */
