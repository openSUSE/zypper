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

/*
enum zypper_colors
{
  ZYPPER_COLOR_MSG_NORMAL           = 1,
  ZYPPER_COLOR_MSG_HIGHLIGHTED      = 2,
  ZYPPER_COLOR_MSG_ERROR            = 3,
  ZYPPER_COLOR_MSG_WARNING          = 4
};
*/

/** Simple check whether stdout can handle colors. */
bool has_colors();

/**
 * Print string \a s in given color.
 *
 * \param s                string to print
 * \param ansi_color_seq   color to print with
 * \param prev_color       color to restore after printing. If NULL,
 *                         COLOR_RESET will be used
 */
void print_color(const std::string & s,
    const char * ansi_color_seq, const char * prev_color);

#endif /* UTILS_COLORS_H_ */
