// file: kernel/main.c

#include "uart.h"

//EX2
#include "printf.h"
#include "console.h"

//EX3
#include "riscv.h"
#include "memlayout.h"
#include "kalloc.h"
#include "vm.h"
#include "defs.h"

static void panic(const char *msg) {
    uart_puts(msg);  // 输出错误信息
    while(1) { __asm__ volatile("wfi"); }  // 死循环
}

#define assert(x) do { if (!(x)) panic("assert failed: " #x); } while(0)


// 定义基本测试
void test_printf_basic(void) {
    printf("Testing integer: %d\n", 42);
    printf("Testing negative: %d\n", -123);
    printf("Testing zero: %d\n", 0);
    printf("Testing hex: 0x%x\n", 0xABC);
    printf("Testing string: %s\n", "Hello");
    printf("Testing char: %c\n", 'X');
    printf("Testing percent: %%\n");
    // 多种格式组合
    printf("Mix: %d %x %c %s %%\n", 42, 0xFF, 'A', "Test");
}

// 定义边界测试
void test_printf_edge_cases(void) {
    printf("INT_MAX: %d\n", 2147483647);
    printf("INT_MIN: %d\n", -2147483648);
    printf("NULL string: %s\n", (char*)0);
    printf("Empty string: %s\n", "");
}

// 性能测试：大量输出
void test_performance() {
    printf("=== Performance Test ===\n");
    for (int i = 0; i < 100; i++) {
        printf("%d ", i);
        if (i % 20 == 19) printf("\n"); // 每行20个
    }
}

// 错误恢复测试：未知格式符
void test_error() {
    printf("=== Error Recovery Test ===\n");
    printf("Unknown format: %q %y %z\n");
}

// ----------------- 打印页表映射（调试） -----------------
void dump_pagetable(pagetable_t pt, int level, unsigned long va_base) {
    for (int i = 0; i < 512; i++) {
        pte_t pte = pt[i];
        if (pte & PTE_V) {
            unsigned long pa = PTE_PA(pte);
            unsigned long va = va_base + i * (1UL << (12 + 9*level));
            if (level > 0) {
                dump_pagetable((pagetable_t)pa, level - 1, va);
            } else {
                printf("VA 0x%lx -> PA 0x%lx [", va, pa);
                if (pte & PTE_R) printf("R");
                if (pte & PTE_W) printf("W");
                if (pte & PTE_X) printf("X");
                if (pte & PTE_U) printf("U");
                printf("]\n");
            }
        }
    }
}


void test_physical_memory(void) {
    // 测试基本分配和释放
    void *page1 = kalloc();
    void *page2 = kalloc();
    assert(page1 != page2);
    assert(((uint64)page1 & 0xFFF) == 0); // 页对齐检查

    // 测试数据写入
    *(int*)page1 = 0x12345678;
    assert(*(int*)page1 == 0x12345678);

    // 测试释放和重新分配
    kfree(page1);
    void *page3 = kalloc();
    // page3可能等于page1（取决于分配策略）

    kfree(page2);
    kfree(page3);
}
void test_pagetable(void) {
    pagetable_t pt = create_pagetable();

    // 测试基本映射
    uint64 va = 0x1000000;
    uint64 pa = (uint64)kalloc();
    assert(map_page(pt, va, pa, PTE_R | PTE_W) == 0);

    // 测试地址转换
    pte_t *pte = walk_lookup(pt, va);
    assert(pte != 0 && (*pte & PTE_V));
    assert(PTE_PA(*pte) == pa);

    // 测试权限位
    assert(*pte & PTE_R);
    assert(*pte & PTE_W);
    assert(!(*pte & PTE_X));
}
void test_virtual_memory(void) {
    printf("Before enabling paging...\n");

    // 启用分页
    //kvminit();       // 创建内核页表
    //kvminithart();   // 激活内核页表
    
    // 1. 初始化物理内存
    printf("1. 初始化物理内存...\n");
    kinit();  // 初始化空闲页链表
    test_physical_memory(); // 可选测试物理页分配器

    // 2. 创建内核页表
    printf("2. 创建内核页表...\n");
    kernel_pagetable = create_pagetable();

    // 3. 映射内核区域
    printf("3. 映射内核区域...\n");
    map_page(kernel_pagetable, KERNBASE, KERNBASE, PTE_R | PTE_X);           // 内核代码
    map_page(kernel_pagetable, KERNBASE + PGSIZE, KERNBASE + PGSIZE, PTE_R | PTE_W); // 内核数据
    map_page(kernel_pagetable, UART0, UART0, PTE_R | PTE_W);                  // 设备

    // 4. 打印页表映射（调试）
    printf("4. 打印页表映射（调试）...\n");
    dump_pagetable(kernel_pagetable, 2, 0);  // 递归打印所有映射

    // 5. 激活内核页表
    printf("5. 激活内核页表...\n");
    w_satp(((unsigned long)8 << 60) | PA2PTE((unsigned long)kernel_pagetable)); // MODE=8 Sv39
    sfence_vma(); // 刷新 TLB

    printf("虚拟内存启用完成。\n");

    // 6. 测试页表功能
    test_pagetable();
    
    printf("After enabling paging...\n");

    // 测试内核代码仍然可执行
    // 测试内核数据仍然可访问
    // 测试设备访问仍然正常
}


// 操作系统主函数，由汇编代码调用。
void start(void) {
    uart_puts("=== This is EX1 ===\n");
    // 打印启动信息，确认系统已成功运行。
    uart_puts("Hello OS!\n");
    
    
    //EX2
    printf("\n");
    clear_screen();
 
    printf("=== This is EX2 ===\n");

    // 调用测试
    test_printf_basic();
    test_printf_edge_cases();
    test_performance();
    test_error();
    
    //EX3
    printf("\n");
    clear_screen();
    printf("=== This is EX3 ===\n");
    
    test_virtual_memory();
    
    
    // 进入无限循环，等待中断。
    while (1) {
        // CPU 进入低功耗模式，等待中断唤醒。
        __asm__ volatile("wfi"); 
    }
}


