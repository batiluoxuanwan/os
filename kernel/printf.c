#include <stdarg.h>
#include "console.h"

// 打印整数（支持 10进制 / 16进制）
static void print_number(long num, int base, int sign) {
    char buf[32];
    int i = 0;
    unsigned long n;

    if (sign && num < 0) {
        console_putc('-');
        n = (unsigned long)(-num);
    } else {
        n = (unsigned long)num;
    }

    if (n == 0) buf[i++] = '0';
    while (n) {
        int digit = n % base;
        buf[i++] = (digit < 10) ? '0' + digit : 'a' + digit - 10;
        n /= base;
    }

    while (i--) console_putc(buf[i]);
}

// 格式化输出
int printf(const char *fmt, ...) {

    va_list ap;//访问可变参数
    va_start(ap, fmt);

    for (const char *p = fmt; *p; p++) {
        if (*p != '%') {
            console_putc(*p);
            continue;
        }

        p++;
        switch (*p) {
            //十进制整数
            case 'd': {
                int val = va_arg(ap, int);
                print_number(val, 10, 1);
                break;
            }
            //十六进制整数
            case 'x': {
                unsigned int val = va_arg(ap, unsigned int);
                print_number(val, 16, 0);
                break;
            }
            //char
            case 'c': {
                char val = (char)va_arg(ap, int);
                console_putc(val);
                break;
            }
            //string
            case 's': {
                const char *val = va_arg(ap, char*);
                console_puts(val);
                break;
            }
            case 'l': {
	        p++;  // 跳过 'l'
	        if (*p == 'x') {
		    unsigned long val = va_arg(ap, unsigned long);
		    print_number(val, 16, 0);
		} 
		else if (*p == 'd') {
		    long val = va_arg(ap, long);
		    print_number(val, 10, 1);
		}
		break;
	    }
            //%
            case '%': {
                console_putc('%');
                break;
            }
            //unkown
            default: {
                console_putc('%');
                console_putc(*p);
                break;
            }
        }
    }

    va_end(ap);//结束访问，释放资源
    return 0;
}

