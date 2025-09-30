#include <stdint.h>
#include "uart.h"

// 定义 UART 寄存器的内存地址和关键位。
#define UART_BASE 0x10000000UL
#define UART_THR  (UART_BASE + 0x00) // 发送数据寄存器
#define UART_LSR  (UART_BASE + 0x05) // 线路状态寄存器
#define LSR_THRE  0x20               // 发送缓冲区空闲位

// 通过内存映射I/O (MMIO) 读取8位数据。
static inline uint8_t mmio_read8(uintptr_t addr) {
    return *(volatile uint8_t *)addr;
}

// 通过 MMIO 写入8位数据。
static inline void mmio_write8(uintptr_t addr, uint8_t val) {
    *(volatile uint8_t *)addr = val;
}

// 发送单个字符。
void uart_putc(char c) {
    // 自动为换行符添加回车符。
    if (c == '\n')
        uart_putc('\r');

    // 等待 UART 发送缓冲区空闲。
    while ((mmio_read8(UART_LSR) & LSR_THRE) == 0)
        ;
    // 将字符写入 UART。
    mmio_write8(UART_THR, (uint8_t)c);
}

// 发送一个字符串。
void uart_puts(const char *s) {
    // 遍历字符串并逐个发送字符。
    while (*s)
        uart_putc(*s++);
}
