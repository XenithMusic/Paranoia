#include "types.h"

extern "C" {
	void handleSyscall(enum syscall_id id, int arg1, int arg2, int arg3);
}