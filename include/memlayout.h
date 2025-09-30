#ifndef MEMLAYOUT_H
#define MEMLAYOUT_H

// 内核虚拟地址起始
#define KERNBASE 0x80000000UL

// 内核可用物理内存上限
#define PHYSTOP  0x88000000UL

// 页大小：4KB
#define PGSIZE 4096

// 内核栈大小
#define KSTACKSIZE (2*PGSIZE)

// 外设基址
#define UART0 0x10000000UL
#define VIRTIO0 0x10001000UL
#define PLIC 0x0c000000UL

// 页对齐宏
#define PGROUNDUP(a) (((a) + PGSIZE - 1) & ~(PGSIZE - 1))
#define PGROUNDDOWN(a) ((a) & ~(PGSIZE - 1))

#endif // MEMLAYOUT_H

