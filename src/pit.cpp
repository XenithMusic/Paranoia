#include "assemblyUtils.h"
#include "math.h"

extern "C" {
	unsigned previous_count;
	uint64_t overflowCount;

	void set_pit_count(int count) {
		unsigned uc = count%65536;
		overflowCount = 0;
		unsigned previous_count = uc;
		// Disable interrupts
		cli();
		
		// Set low byte
		outb(0x40,count&0xFF);		// Low byte
		outb(0x40,(count&0xFF00)>>8);	// High byte
		return;
	}
	uint64_t get_pit_count(void) {
		unsigned count = 0;
		
		// Disable interrupts
		cli();
		
		// al = channel in bits 6 and 7, remaining bits clear
		outb(0x43,0b0000000);
		
		count = inb(0x40);		// Low byte
		count |= inb(0x40)<<8;		// High byte

		count = 65535-count;

		if (count < previous_count) {
			overflowCount++;
		}
		previous_count = count;
		
		return count+(overflowCount*65536);
	}

	double get_pit_seconds(void) {
		uint32_t count = get_pit_count();
		double seconds = count / 1193182.0;
		return seconds;
	}
}