#ifndef DEFS_H
#define DEFS_H

#include "riscv.h"

// ----------------- 物理内存分配器接口 -----------------
void kinit(void);          // 初始化物理内存分配器
void* kalloc(void);        // 分配一个物理页
void kfree(void*);         // 释放一个物理页

// ----------------- 页表管理接口 -----------------
typedef unsigned long* pagetable_t;

pagetable_t create_pagetable(void);                           // 创建根页表
int map_page(pagetable_t pt, unsigned long va, unsigned long pa, int perm); // 建立虚拟地址->物理地址映射
void destroy_pagetable(pagetable_t pt);                       // 释放页表（简化实现）

#endif // DEFS_H

