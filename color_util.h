#ifndef COLOR_UTIL_H
#define COLOR_UTIL_H

#define COLOR_BLACK   0
#define COLOR_BLUE    1
#define COLOR_GREEN   2
#define COLOR_CYAN    3
#define COLOR_RED     4
#define COLOR_MAGENTA 5
#define COLOR_YELLOW  6
#define COLOR_WHITE   7

void SetColor(int fg, int bg);
void ResetColor(void);

#endif
