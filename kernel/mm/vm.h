#ifndef VM_H
#define VM_H

#include "defs.h"
#include "types.h"
#include "riscv.h"

// 内核全局页表
extern pagetable_t kernel_pagetable;

// ----------------- 页表管理接口 -----------------

// 递归查找/创建页表项
pte_t* walk_create(pagetable_t pt, unsigned long va);

// 查找页表项，不分配
pte_t* walk_lookup(pagetable_t pt, unsigned long va);

// 建立虚拟地址 -> 物理地址映射
int map_page(pagetable_t pt, unsigned long va, unsigned long pa, int perm);

// 映射连续虚拟地址区域
void map_region(pagetable_t pt, uint64 va_start, uint64 pa_start, uint64 size, int perm);

// 创建根页表
pagetable_t create_pagetable(void);

// 释放页表（简化实现）
void destroy_pagetable(pagetable_t pt);

// 启动页表初始化
void kvminit(void);
void kvminithart(void);


#endif // VM_H

