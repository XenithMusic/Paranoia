extern "C" {
    void exception_handler();
    void divzero_handler();
    void irq1_handler();
    void syscall();
    void syscall_c(int id,int arg1,int arg2,int arg3);
}