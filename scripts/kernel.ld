/* * 链接脚本: 定义内核内存布局
 */

/* 目标架构：RISC-V */
OUTPUT_ARCH(riscv)

/* 程序入口点：_entry */
ENTRY(_entry)

SECTIONS
{
    /* 内核从 0x80000000 地址开始加载 */
    . = 0x80000000;

    /* 代码段 */
    .text : ALIGN(16) {
        *(.text.entry)
        *(.text .text.*)
        etext = .;    /* 定义 etext */
    }

    /* 只读数据段 */
    .rodata : ALIGN(16) {
        *(.rodata .rodata.*)
    }

    /* 已初始化数据段 */
    .data : ALIGN(16) {
        *(.data .data.*)
    }

    /* 未初始化数据段 */
    .bss : ALIGN(16) {
        __bss_start = .;
        *(.bss .bss.*)
        *(COMMON)
        __bss_end = .;
    }
    PROVIDE(__bss_start = __bss_start);
    PROVIDE(__bss_end = __bss_end);

    /* 栈段 (16KB) */
    .stack : ALIGN(16) {
        . += 0x4000;
        stack_top = .;
    }
    PROVIDE(stack_top = stack_top);
    
    /* 定义内核结束符，用于物理内存初始化 */
    end = .;
    PROVIDE(end = end);
}
