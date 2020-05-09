#ifndef INCLUDE_DS1302_H
#define INCLUDE_DS1302_H

#include <stdint.h>
#include <time.h>

int ds1302_in_byte(void);
int ds1302_out_byte(uint8_t send_byte);
int ds1302_read_byte(uint8_t address);
int ds1302_write_byte(uint8_t address, uint8_t data);
int ds1302_time_get(struct tm *time);

#endif
