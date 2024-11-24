#include "terminal.h"

void setError(int errno);
int getError();
void clearError();
void fault(int errno,char* string);
void fault(int errno);
void panic(int errno,char* string);
void panic(int errno);