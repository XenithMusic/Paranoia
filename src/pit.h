#define PIT_FREQUENCY 1193182.0

#include "assemblyUtils.h"

extern "C" {
	void set_pit_count(unsigned count);
	unsigned get_pit_count(void);
	double get_pit_seconds(void);
	void sleep(double seconds);
}