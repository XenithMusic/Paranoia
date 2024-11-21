#include <stdint.h>

extern "C" {
	void cli();

	void outb(uint16_t port, uint8_t val);

	uint8_t inb(uint16_t port);
}