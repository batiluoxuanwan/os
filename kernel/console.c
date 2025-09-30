#include "uart.h"
#include "console.h"

// 输出单个字符到串口
void console_putc(char c) {
    uart_putc(c);
}

// 输出字符串（NULL 安全）
void console_puts(const char *s) {
    if (!s) {
        console_puts("(null)");
        return;
    }
    while (*s) {
        console_putc(*s++);
    }
}

// 使用 ANSI 转义序列清屏
void clear_screen(void) {
    console_puts("\033[2J"); // 清屏
    console_puts("\033[H");  // 光标回到左上角
}

