#include <stdint.h>

extern "C" {
	void cli() {
		__asm__ __volatile__ ("cli");
	}

	void outb(uint16_t port, uint8_t val) {
		__asm__ __volatile__ ("outb %0, %1" : : "a" (val), "Nd" (port));
	}

	uint8_t inb(uint16_t port) {
		uint8_t val;
		__asm__ __volatile__ ("inb %1, %0" : "=a" (val) : "Nd" (port));
		return val;
	}
}