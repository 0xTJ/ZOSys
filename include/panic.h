#ifndef INCLUDE_PANIC_H
#define INCLUDE_PANIC_H

void disable_all_interrupts(void);
void panic_no_message(const char *file, const char *line);
void panic_message(const char *file, const char *line, const char *err_message);

#define PANIC_STRINGIZE(x) PANIC_STRINGIZE2(x)
#define PANIC_STRINGIZE2(x) #x
#define PANIC_LINE_STRING PANIC_STRINGIZE(__LINE__)

#define panic() panic_no_message(__FILE__, PANIC_LINE_STRING);
// #define panic(err_message) panic_message(__FILE__, PANIC_LINE_STRING, err_message);

#endif
