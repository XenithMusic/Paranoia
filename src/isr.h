extern "C" {
    void exception_handler();
    void divzero_handler();
    void genprotfault_handler();
    void irq1_assembly();
    void irq1_handler();
    void basic_eoi_assembly_low();
    void basic_eoi_handler_low();
    void basic_eoi_assembly_high();
    void basic_eoi_handler_high();
    void syscall();
    void syscall_c(int id,int arg1,int arg2,int arg3);
}