
    .globl irq1_assembly
    .extern irq1_handler
    .globl basic_eoi_assembly_low
    .extern basic_eoi_handler_low
    .globl basic_eoi_assembly_high
    .extern basic_eoi_handler_high

basic_eoi_assembly_low:
    push %ds
    pushal
    mov $0x10, %ax
    mov %ax, %ds
    push $0
    push $33 # 0x21
    call basic_eoi_handler_low
    add $8, %esp
    popal
    pop %ds
    iret
basic_eoi_assembly_high:
    push %ds
    pushal
    mov $0x10, %ax
    mov %ax, %ds
    push $0
    push $33 # 0x21
    call basic_eoi_handler_high
    add $8, %esp
    popal
    pop %ds
    iret
irq1_assembly:
    push %ds
    pushal
    mov $0x10, %ax
    mov %ax, %ds
    push $0
    push $33 # 0x21
    call irq1_handler
    add $8, %esp
    popal
    pop %ds
    iret