#ifndef RISCV_H
#define RISCV_H

#include "memlayout.h"

// ----------------- 页表项位 -----------------
#define PTE_V (1UL << 0)  // 页表项有效
#define PTE_R (1UL << 1)  // 读权限
#define PTE_W (1UL << 2)  // 写权限
#define PTE_X (1UL << 3)  // 执行权限
#define PTE_U (1UL << 4)  // 用户态可访问

typedef unsigned long pte_t;  // 页表项类型（64位）

// ----------------- 页表索引宏 -----------------
// level = 2,1,0 对应 Sv39 的三级页表
#define VPN_SHIFT(level) (12 + 9*(level))
#define VPN_MASK 0x1FF
#define VPN(va, level) (((va) >> VPN_SHIFT(level)) & VPN_MASK)

// 页表项与物理地址转换
#define PTE_PA(pte) ((((unsigned long)(pte)) >> 10) << 12)
#define PA2PTE(pa)  ((((unsigned long)(pa)) >> 12) << 10)

#define MAKE_SATP(pagetable) ((8UL << 60) | (PA2PTE(pagetable) & 0xFFFFFFFFFFFUL))

// ----------------- 内核版 memset -----------------
// 内核环境下没有 libc，需要自己实现
static inline void memset(void *dst, int c, unsigned long n) {
    unsigned char *p = dst;
    for(unsigned long i = 0; i < n; i++)
        p[i] = (unsigned char)c;
}

// ----------------- CSR 寄存器读写 -----------------
static inline unsigned long r_satp(void) {
    unsigned long x;
    asm volatile("csrr %0, satp" : "=r"(x));
    return x;
}

static inline void w_satp(unsigned long x) {
    asm volatile("csrw satp, %0" :: "r"(x));
}

// 刷新 TLB
static inline void sfence_vma(void) {
    asm volatile("sfence.vma");
}

#endif // RISCV_H

