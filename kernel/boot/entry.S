# file: kernel/entry.S

# 设置代码段，确保其加载到内存起始地址 0x80000000。
.section .text.entry
# 声明 _entry 为全局入口点。
.global _entry

_entry:
    # 禁用所有中断。
    csrwi mstatus, 0
    csrwi mie, 0

    # 打印调试字符 'S'。
    li t0, 0x10000000   # UART 基地址
    li t1, 'S'          # 将 'S' 加载到寄存器 t1
1:
    li a0, 0x10000005   # 将 UART_LSR 地址加载到 a0
    lbu a1, 0(a0)       # 读取 UART_LSR 状态
    andi a1, a1, 0x20   # 检查发送缓冲区是否为空
    beqz a1, 1b         # 如果不空，继续等待
    sb t1, 0(t0)        # 写入 'S'

    # 设置栈指针。
    la sp, stack_top

    # 打印调试字符 'P'。
    li t1, 'P'          # 将 'P' 加载到寄存器 t1
2:
    li a0, 0x10000005   # 将 UART_LSR 地址加载到 a0
    lbu a1, 0(a0)
    andi a1, a1, 0x20
    beqz a1, 2b
    sb t1, 0(t0)        # 写入 'P'

    # 打印换行符。
    li t1, '\n'         # 将 '\n' 加载到寄存器 t1
3:
    li a0, 0x10000005
    lbu a1, 0(a0)
    andi a1, a1, 0x20
    beqz a1, 3b
    sb t1, 0(t0)        # 写入 '\n'

    # 清零 BSS 段。
    la t0, __bss_start
    la t1, __bss_end
4:
    bge t0, t1, 5f
    sd zero, 0(t0)
    addi t0, t0, 8
    j 4b

    # 调用 C 语言主函数。
5:
    call start

    # 进入无限循环，防止程序结束。
hang:
    j hang
