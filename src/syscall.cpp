#include "fail.h"
#include "memory.h"
#include "types.h"
#include "pit.h"

extern "C" {
	void handleSyscall(enum syscall_id id, int arg1, int arg2, int arg3) {
		switch (id) {
			case SCALL_SLEEP: {
				sleep((double)arg1);
			}
		}
	}
}