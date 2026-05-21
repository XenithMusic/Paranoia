#include "cmos.h"
#include "types.h"

#define R_SECONDS 0x00
#define R_MINUTES 0x02
#define R_HOURS 0x04
#define R_WEEKDAY 0x06
#define R_DAY 0x07
#define R_MONTH 0x08
#define R_YEAR 0x09
#define R_CENTURY 0x32
#define R_STATUS_A 0x0A
#define R_STATUS_B 0x0B

namespace RTC {
    Date date;
    uint8_t bcdToBin(uint8_t bcd) {
        uint8_t tens = (bcd & 0xf0) >> 4;
        uint8_t ones = bcd & 0xf;
        return tens*10 + ones;
    }
    void bcdToBin(uint8_t* bcd) {
        *bcd = bcdToBin(*bcd);
    }
    void calibrate() {
        date = {};
        while ((CMOS::read_register(R_STATUS_A)&0b10000000) == 0) {}
        while ((CMOS::read_register(R_STATUS_A)&0b10000000) != 0) {}
        // guaranteed to not be busy here
        date.seconds = CMOS::read_register(R_SECONDS);
        date.minutes = CMOS::read_register(R_MINUTES);
        date.hours = CMOS::read_register(R_HOURS);
        date.weekday = CMOS::read_register(R_WEEKDAY);
        date.day = CMOS::read_register(R_DAY);
        date.month = CMOS::read_register(R_MONTH);
        date.year = CMOS::read_register(R_YEAR) + (100*CMOS::read_register(R_CENTURY));
        date.mode = CMOS::read_register(R_STATUS_B);
        bool pm = false;
        if ((date.mode&0b10) == 0) {
            if ((date.hours & (0x80)) == 1) pm = true;
            date.hours &= ~(0x80);
            if (date.hours == 12) date.hours = 0;
        }
        if ((date.mode&0b100) == 0) {
            bcdToBin(&date.seconds);
            bcdToBin(&date.minutes);
            bcdToBin(&date.hours);
        }
        if (pm) date.hours += 12;
    }
}