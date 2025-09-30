#ifndef KALLOC_H
#define KALLOC_H

#include "defs.h"

// ----------------- 物理页分配器接口 -----------------

// 初始化物理内存分配器，建立空闲页链表
void kinit(void);

// 分配一个物理页（返回页首地址），可能返回 NULL
void* kalloc(void);

// 释放一个物理页
void kfree(void*);


#endif // KALLOC_H

