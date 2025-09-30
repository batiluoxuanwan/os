#include "riscv.h"
#include "defs.h"
#include "kalloc.h"
#include "vm.h"
#include "memlayout.h"
//#include <string.h>
#include <stdio.h>

pagetable_t kernel_pagetable;

extern char etext[];


// ----------------- 递归查找/创建页表项 -----------------
pte_t* walk_create(pagetable_t pt, unsigned long va) {
    for (int level = 2; level > 0; level--) {
        int idx = VPN(va, level);
        pte_t* pte = &pt[idx];

        if (*pte & PTE_V) {
            pt = (pagetable_t)PTE_PA(*pte);
        } else {
            pagetable_t new_pt = (pagetable_t)kalloc();
            if (!new_pt) return 0;
            memset(new_pt, 0, PGSIZE);
            *pte = PA2PTE(new_pt) | PTE_V;
            pt = new_pt;
        }
    }
    return &pt[VPN(va, 0)];
}

// ----------------- 查找页表项，不分配 -----------------
pte_t* walk_lookup(pagetable_t pt, unsigned long va) {
    for (int level = 2; level > 0; level--) {
        int idx = VPN(va, level);
        pte_t* pte = &pt[idx];
        if (!(*pte & PTE_V)) return 0;
        pt = (pagetable_t)PTE_PA(*pte);
    }
    return &pt[VPN(va, 0)];
}

// ----------------- 建立虚拟地址 -> 物理地址映射 -----------------
int map_page(pagetable_t pt, unsigned long va, unsigned long pa, int perm) {
    pte_t* pte = walk_create(pt, va);
    if (!pte) return -1;
    if (*pte & PTE_V) return -1; // 已经映射
    *pte = PA2PTE(pa) | perm | PTE_V;
    return 0;
}

// ----------------- 映射连续区域 -----------------
void map_region(pagetable_t pt, uint64 va_start, uint64 pa_start, uint64 size, int perm) {
    for (uint64 offset = 0; offset < size; offset += PGSIZE) {
        map_page(pt, va_start + offset, pa_start + offset, perm);
    }
}

// ----------------- 创建根页表 -----------------
pagetable_t create_pagetable(void) {
    pagetable_t pt = (pagetable_t)kalloc();
    if (pt) memset(pt, 0, PGSIZE);
    return pt;
}

// ----------------- 释放页表（简化） -----------------
void destroy_pagetable(pagetable_t pt) {
    kfree(pt);
}


// ----------------- 内核页表初始化 -----------------
void kvminit(void) {
    kernel_pagetable = create_pagetable();
    map_region(kernel_pagetable, KERNBASE, KERNBASE, (uint64)etext - KERNBASE, PTE_R | PTE_X);
    map_region(kernel_pagetable, (uint64)etext, (uint64)etext, PHYSTOP - (uint64)etext, PTE_R | PTE_W);
    map_region(kernel_pagetable, UART0, UART0, PGSIZE, PTE_R | PTE_W);
}

// ----------------- 激活内核页表 -----------------
void kvminithart(void) {
    w_satp(MAKE_SATP(kernel_pagetable));
    sfence_vma();
}

