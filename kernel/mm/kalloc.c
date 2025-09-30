#include "defs.h"
#include "riscv.h"
#include "kalloc.h"
#include "memlayout.h"
//#include <string.h>

// 空闲页链表结构
struct run {
    struct run *next;
};

static struct run* freelist;  // 空闲页头指针

// 链接脚本中定义的内核结束符
extern char end[];

// ----------------- 初始化物理内存分配器 -----------------
void kinit(void) {
    char *p = (char*)PGROUNDUP((unsigned long)end); // 页对齐
    // 遍历剩余物理内存，每页加入空闲链表
    for (; p + PGSIZE <= (char*)PHYSTOP; p += PGSIZE) {
        kfree(p);
    }
}

// ----------------- 释放一个物理页 -----------------
void kfree(void *pa) {
    struct run *r;

    if (((unsigned long)pa % PGSIZE) != 0 || (char*)pa < end || (unsigned long)pa >= PHYSTOP)
        return;

    r = (struct run*)pa;
    r->next = freelist; // 插入空闲链表头
    freelist = r;
}

// ----------------- 分配一个物理页 -----------------
void* kalloc(void) {
    struct run *r = freelist;
    if (r)
        freelist = r->next; // 更新链表头
    return (void*)r;        // 返回页地址，可能为 NULL
}


