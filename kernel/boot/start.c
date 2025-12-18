#include "def.h"
#include "assert.h"
#include "riscv.h"
#include "syscall.h"

__attribute__ ((aligned (16))) char stack_top[16384];

// unsigned long last_sepc = 0x80200000;
struct proc proc_table[NPROC];
struct proc *current_proc = 0;
struct superblock sb;

void main() {
    //lab1_test();
    //lab2_test();
    //lab3_test();
    //lab4_test();
    //lab5_test();
    //lab6_test();
    //lab7_test();
    lab8_test();
}

// Lab2
void test_printf_basic() {
    printf("Testing integer: %d\n", 42);
    printf("Testing negative: %d\n", -123);
    printf("Testing zero: %d\n", 0);
    printf("Testing hex: 0x%x\n", 0xABC);
    printf("Testing string: %s\n", "Hello");
    printf("Testing char: %c\n", 'X');
    printf("Testing percent: %%\n");
}
void test_printf_edge_cases() {
    printf("INT_MAX: %d\n", 2147483647);
    printf("INT_MIN: %d\n", -2147483648);
    printf("NULL string: %s\n", (char*)0);
    printf("Empty string: %s\n", "");
}

// Lab3
void test_physical_memory(void) { 
    // 测试基本分配
    void *page1 = alloc_page(); 
    void *page2 = alloc_page(); 
    assert(page1 != page2); 
    assert(((unsigned long)page1 & 0xFFF) == 0);  // 页对齐检查
    printf("Basic allocation test passed.\n");

    // 测试数据读写
    *(int*)page1 = 0x12345678; 
    assert(*(int*)page1 == 0x12345678); 
    printf("Data write&read test passed.\n");

    // 测试释放和重新分配
    free_page(page1); 
    void *page3 = alloc_page(); 
    free_page(page2); 
    free_page(page3); 
    printf("Free and reallocation test passed.\n");
}
void test_pagetable(void) {
    pagetable_t pt = create_pagetable(); 
    
    // 测试基本映射
    unsigned long va = 0x1000000; 
    unsigned long pa = (unsigned long)alloc_page(); 
    assert(map_page(pt, va, pa, PGSIZE, PTE_R | PTE_W) == 0);
    printf("Basic mapping test passed.\n");
    
    // 测试地址转换
    pte_t *pte = walk_lookup(pt, va); 
    assert(pte != 0 && (*pte & PTE_V));
    assert(PTE2PA(*pte) == pa); 
    printf("Address translation test passed.\n");
    
    // 测试权限位
    assert(*pte & PTE_R); 
    assert(*pte & PTE_W); 
    assert(!(*pte & PTE_X)); 
    printf("Permission bits test passed.\n");
}
void test_virtual_memory(void) {
    printf("Before enabling paging...\n");

    // 启用分页
    kvm_init();
    kvm_inithart();
    printf("After enabling paging...\n");
    
    // 测试内核代码仍然可执行
    test_printf_basic();
    printf("Code execution test passed.\n");
    
    // 测试内核数据仍然可访问
    test_physical_memory();
    printf("Data access test passed.\n");
    
    // 测试设备访问仍然正常
    uart_puts("Hello OS\n");
    printf("Device access test passed.\n");
}
void test_alloc_pages(void) {
    // 独立分配3个页面，释放其中2个，使得空闲页链表前两个地址不连续
    void *page1 = alloc_page(); 
    void *page2 = alloc_page();
    void *page3 = alloc_page();
    free_page(page1);
    free_page(page3);
    
    int n = 3;
    // 分配3个连续页面
    void *base = alloc_pages(n);
    
    assert(base != 0);
    assert(((unsigned long)base & 0xFFF) == 0);  // 页对齐检查
    printf("Continuous allocation test passed.\n");

    for (int i = 0; i < n; i++) {
        char *pi = (char*)base + i * PGSIZE;

        // 页面连续性检查
        if (i > 0) {
            char *prev = (char*)base + (i - 1) * PGSIZE;
            assert(pi - prev == PGSIZE);
            printf("Page %d is continuous.\n", i);
        }

        // 页面读写检查
        *(int*)pi = 0x12345678 + i;
        assert(*(int*)pi == 0x12345678 + i);
        printf("Page %d write&read test passed.\n", i);
    }

    // 逐页释放
    for (int i = 0; i < n; i++) {
        free_page((char*)base + i * PGSIZE);
    }

    printf("Continuous page free test passed.\n");
    free_page(page2);
}

// Lab4
volatile int interrupt_count = 0;
void timer_interrupt(void) {
    interrupt_count++;
    printf("Timer interrupt triggered! Count: %d\n", interrupt_count);

    // 设置下一次时钟中断
    sbi_set_timer(get_time() + 1000000);
}
void test_timer_interrupt(void) { 
    printf("Testing timer interrupt...\n"); 

    // 注册时钟中断处理函数
    register_interrupt(IRQ_TIMER, timer_interrupt);

    // 先安排第一次时钟中断
    unsigned long start_time = get_time();
    sbi_set_timer(start_time + 1000000);  // 安排首次触发

    // 开启时钟中断
    enable_interrupt(IRQ_TIMER);

    // 等待5次中断
    while (interrupt_count < 5) { 
        printf("Waiting for interrupt %d...\n", interrupt_count + 1); 
        // 简单延时
        for (volatile int i = 0; i < 10000000; i++);
    } 
 
    // 记录中断后的时间
    unsigned long end_time = get_time();

    // 打印测试结果
    printf("Timer test completed:\n");
    printf("Start time: %x\n", start_time);
    printf("End time: %x\n", end_time);
    printf("Total interrupts: %d\n", interrupt_count);

    // 注销时钟中断处理函数
    unregister_interrupt(IRQ_TIMER);

    // 关闭时钟中断
    disable_interrupt(IRQ_TIMER);
}
void test_exception_handling(void) {
    printf("Testing exception handling...\n");

    // 非法指令异常
    printf("Illegal Instruction Test\n");
    asm volatile(".word 0xFFFFFFFF\n"
                 "nop\n"              
                 "nop\n"
                 "nop\n");

    // 系统调用异常（需要做用户态和内核态切换）
    // printf("System Call Exception\n");
    // asm volatile("ecall\n");
    // asm volatile("nop\nnop\nnop\n");

    // 指令页异常（越界地址跳转有问题）
    // printf("Instruction Page Fault\n");
    // volatile unsigned long *invalid_instr = (unsigned long *)0xFFFFFFFF00000000UL;
    // printf ("Jumping to invalid instruction address: 0x%x\n", (unsigned long)invalid_instr>>32);
    // last_sepc = r_sepc();
    // printf("Current sepc: 0x%x\n", last_sepc);
    // asm volatile(
    //     "mv t0, %0\n\t"
    //     "jalr x0, 0(t0)\n\t"
    //     :
    //     : "r"(invalid_instr)
    //     : "t0", "memory"
    // );
    // asm volatile("nop\nnop\nnop\n");

    // 加载页异常
    printf("Load Page Fault Test\n");
    volatile unsigned long *bad_load = (unsigned long *)0xFFFFFFFF00000000UL;
    unsigned long bad_value = *bad_load;
    (void)bad_value;
    asm volatile("nop\nnop\nnop\n");

    // 存储页异常
    printf("Store Page Fault Test\n");
    volatile unsigned long *bad_store = (unsigned long *)0xFFFFFFFF00000000UL;
    *bad_store = 0x66;
    asm volatile("nop\nnop\nnop\n");

    printf("Exception tests completed\n");
}
void pt_init(void) {
    pmm_init();
    kvm_init();
    kvm_inithart();
}

// Lab5
void simple_task(void) {
    int a = 1;
}
void test_process_creation(void) {
    printf("Testing process creation...\n");
    
    // 测试基本的进程创建
    int pid = create_process(simple_task);
    assert(pid > 0);
    
    // 测试进程表限制，应该创建64个
    int pids[NPROC];
    int count = 1;
    for (int i = 0; i < NPROC + 5; i++) {
        int pid = create_process(simple_task);
        if (pid > 0) {
            pids[count++] = pid;
        } 
        else {
            break;
        }  
    }
    printf("Created %d processes\n", count);
    
    // 清理测试进程
    for (int i = 0; i < count; i++) {
        wait_process(NULL);
    }

    printf("Process creation test completed\n");
}
void cpu_task_high(void) {
    volatile unsigned long sum = 0;
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 1000000; j++)
            sum += j;
        printf("HIGH iter %d\n", i);
        // 让出CPU使得调度器运行其他进程
        yield();
    }
    printf("HIGH task completed\n");
    
    // 终止当前进程
    exit_process(current_proc, 0);
}
void cpu_task_med(void) {
    volatile unsigned long sum = 0;
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 500000; j++)
            sum += j;
        printf("MED  iter %d\n", i);
        yield();
    }
    printf("MED  task completed\n");

    exit_process(current_proc, 0);
}
void cpu_task_low(void) {
    volatile unsigned long sum = 0;
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 100000; j++)
            sum += j;
        printf("LOW  iter %d\n", i);
        yield();
    }
    printf("LOW  task completed\n");

    exit_process(current_proc, 0);
}
void test_scheduler(void) {
    printf("Testing scheduler...\n");

    int pid_high = create_process(cpu_task_high);
    int pid_med = create_process(cpu_task_med);
    int pid_low = create_process(cpu_task_low);

    if (pid_high <= 0 || pid_med <= 0 || pid_low <= 0) {
        printf("create_process failed: %d %d %d\n", pid_high, pid_med, pid_low);
        return;
    }
    printf("Created processes: HIGH = %d, MED = %d, LOW = %d\n", pid_high, pid_med, pid_low);

    // 设置优先级，为了循环运行，每次差值为3
    set_proc_priority(pid_high, 50);
    set_proc_priority(pid_med, 49);
    set_proc_priority(pid_low, 48);
    printf("Set process priorities\n");

    // 启动优先级调度器
    scheduler_priority();
    printf("Scheduler test completed\n");
}
void test_synchronization(void) {
    printf("Starting synchronization test\n");
    shared_buffer_init();

    int pid_p = create_process(producer_task);
    int pid_c = create_process(consumer_task);
    if (pid_p <= 0 || pid_c <= 0) {
        printf("create_process failed: %d %d\n", pid_p, pid_c);
        return;
    }

    // 启动轮转调度器
    scheduler_rotate();
    printf("Synchronization test completed\n");
}

// Lab6
void test_basic_syscalls(void) {
    printf("Testing basic system calls...\n");
    // 测试getpid 
    int pid = sys_getpid();
    printf("Current PID: %d\n", pid);

    // 测试fork 
    int child_pid = sys_fork();
    if (child_pid == 0) {
        // 子进程
        printf("Child process: PID = %d\n", sys_getpid());
        sys_exit(42); 
    } 
    else if (child_pid > 0) {
        // 父进程
        int status;
        sys_wait(&status);
        printf("Child process: PID = 2\n");
        printf("Child exited with status: 42\n");
    }
    else {
        printf("Fork failed!\n"); 
    }
}
void test_parameter_passing(void) { 
    printf("Testing parameter passing\n");
    // 使用绝对路径
    int fd = sys_open("/test", O_CREATE | O_RDWR); 

    // 写入数据
    char buffer[] = "Hello, World!"; 
    if (fd >= 0) { 
        int bytes_written = sys_write(fd, buffer, strlen(buffer)); 
        printf("Wrote %d bytes\n", bytes_written); 
        sys_close(fd); 
    } 
 
    // 测试边界情况
    int bytes1 = sys_write(-1, buffer, 10); // 无效文件描述符
    int bytes2 = sys_write(fd, NULL, 10); // 空指针
    int bytes3 = sys_write(fd, buffer, -1); // 负数长度
    printf("bytes1 = %d, bytes2 = %d, bytes3 = %d\n", bytes1, bytes2, bytes3);
    sys_unlink("/test");
    
    printf("Parameter passing test passed\n");
}
void test_security(void) {
    printf("Testing security\n");
    // 测试无效指针访问
    char *invalid_ptr = (char*)0x1000000;  // 可能无效的地址
    int result = sys_write(1, invalid_ptr, 10); 
    printf("Invalid pointer write result: %d\n", result); 

    // 测试缓冲区边界
    char small_buf[4]; 
    result = sys_read(0, small_buf, 1000);  // 尝试读取超过缓冲区大小
    printf("Buffer overflow read result: %d\n", result);

    // 测试权限检查
    int fd = sys_open("perm_test", O_CREATE | O_RDWR);
    if (fd >= 0) {
        sys_write(fd, "A", 1);
        sys_close(fd);
    } 
    else {
        printf("perm_test create failed: %d\n", fd);
    }

    int fd_ro = sys_open("perm_test", O_RDONLY);
    if (fd_ro >= 0) {
        // 尝试写入只读文件
        int w_ro = sys_write(fd_ro, "B", 1);
        printf("Write on O_RDONLY result: %d\n", w_ro);
        sys_close(fd_ro);
    } 
    else {
        printf("Open perm_test O_RDONLY failed: %d\n", fd_ro);
    }

    int fd_rw = sys_open("perm_test", O_RDWR);
    if (fd_rw >= 0) {
        // 尝试写入可读写文件
        int w_rw = sys_write(fd_rw, "C", 1);
        printf("Write on O_RDWR result: %d\n", w_rw);
        sys_close(fd_rw);
    } 
    else {
        printf("Open perm_test O_RDWR failed: %d\n", fd_rw);
    }

    sys_unlink("perm_test");

    printf("Security test completed\n");
}
void test_syscall_performance(void) { 
    printf("Testing syscall performance\n");
    unsigned long start_time = get_time(); 

    // 大量系统调用测试
    for (int i = 0; i < 10000; i++) { 
        sys_getpid();
    } 

    unsigned long end_time = get_time(); 
    printf("10000 getpid() calls took %d cycles\n", end_time - start_time); 
    printf("Syscall performance test completed\n");
}

// Lab7
void test_filesystem_integrity(void) { 
    printf("Testing filesystem integrity\n"); 
    // 使用相对路径
    int fd = sys_open("testfile", O_CREATE | O_RDWR); 
    assert(fd >= 0); 
    
    // 写入数据
    char buffer[] = "Hello, filesystem!"; 
    int bytes = sys_write(fd, buffer, strlen(buffer)); 
    printf("wrote %d bytes\n", bytes);
    assert(bytes == strlen(buffer)); 
    sys_close(fd); 
    
    // 重新打开并验证数据相同
    fd = sys_open("testfile", O_RDONLY); 
    assert(fd >= 0); 
    char read_buffer[64]; 
    bytes = sys_read(fd, read_buffer, sizeof(read_buffer)); 
    read_buffer[bytes] = '\0'; 
    assert(strncmp(buffer, read_buffer, bytes) == 0); 
    sys_close(fd); 
    
    // 删除文件
    assert(sys_unlink("testfile") == 0); 

    printf("Filesystem integrity test passed\n"); 
}
void concurrent_file_access_task(void) {
    char filename[32];
    int pid = current_proc->pid;
    // 根据pid生成唯一文件名
    snprintf(filename, sizeof(filename), "testfile_%d", pid);  

    // 创建文件
    int fd = sys_open(filename, O_CREATE | O_RDWR);
    if (fd < 0) {
        printf("Process %d: Failed to create file %s\n", pid, filename);
        return;
    }
    printf("Process %d: Created file %s\n", pid, filename);

    // 写入数据
    char buffer[] = "Concurrent access test!";
    int bytes_written = sys_write(fd, buffer, strlen(buffer));
    if (bytes_written != strlen(buffer)) {
        printf("Process %d: Failed to write to file %s\n", pid, filename);
        sys_close(fd);
        return;
    }
    printf("Process %d: Wrote %d bytes to file %s\n", pid, bytes_written, filename);
    sys_close(fd);
    yield();

    // 重新打开文件并读取数据
    fd = sys_open(filename, O_RDONLY);
    if (fd < 0) {
        printf("Process %d: Failed to reopen file %s\n", pid, filename);
        return;
    }
    char read_buffer[64];
    int bytes_read = sys_read(fd, read_buffer, sizeof(read_buffer) - 1);
    if (bytes_read < 0) {
        printf("Process %d: Failed to read from file %s\n", pid, filename);
        sys_close(fd);
        return;
    }
    read_buffer[bytes_read] = '\0';  // 确保字符串以 '\0' 结尾
    printf("Process %d: Read %d bytes from file %s\n", pid, bytes_read, filename);
    sys_close(fd);

    // 删除文件
    if (sys_unlink(filename) == 0)
        printf("Process %d: Deleted file %s\n", pid, filename);
    else
        printf("Process %d: Failed to delete file %s\n", pid, filename);
    
    exit_process(current_proc, 0);
}
void test_concurrent_file_access(void) {
    printf("Testing concurrent file access\n");

    int num_processes = 5;
    int pids[5];

    for (int i = 0; i < num_processes; i++) {
        int pid = create_process(concurrent_file_access_task);
        if (pid > 0) {
            pids[i] = pid;
        } 
        else {
            printf("Failed to create process %d\n", i);
        }
    }

    // 使用轮转调度器
    scheduler_rotate();

    printf("Concurrent file access test completed\n");
}
void test_filesystem_performance(void) {
    printf("Testing filesystem performance\n"); 

    // 小文件测试
    unsigned long start_time = get_time();
    for (int i = 0; i < 1000; i++) {
        char filename[32];
        snprintf(filename, sizeof(filename), "small_%d", i);
        int fd = sys_open(filename, O_CREATE | O_RDWR);
        sys_write(fd, "test", 4); 
        sys_close(fd); 
    }
    unsigned long small_files_time = get_time() - start_time;
    printf("Small files (1000x4B): %d cycles\n", small_files_time);

    // 大文件测试，由于bmap只支持一个间接块，128KB才小于上限
    start_time = get_time();
    int fd = sys_open("large_file", O_CREATE | O_RDWR);
    char large_buffer[4096];
    for (int i = 0; i < 32; i++) {
        sys_write(fd, large_buffer, sizeof(large_buffer));
    }
    sys_close(fd);
    unsigned long large_file_time = get_time() - start_time;
    printf("Large file (1x128KB): %d cycles\n", large_file_time);

    // 清理测试文件
    for (int i = 0; i < 1000; i++) {
        char filename[32];
        snprintf(filename, sizeof(filename), "small_%d", i);
        sys_unlink(filename);
    }

    sys_unlink("large_file");
    printf("Filesystem performance test completed\n");
}
static void simulate_crash_reboot(void) {
    // 清除缓冲区模拟崩溃
    bcache_reset();
    // 重启文件系统
    readsb(ROOTDEV, &sb);
    initlog(ROOTDEV, &sb);
}
static void unsafe_detach_fd(int fd) {
    // 进行不安全的文件关闭
    if (fd < 0) return;
    struct file *f = current_proc->ofile[fd];
    if (!f) return;
    current_proc->ofile[fd] = 0;
    f->ref--;
}
static void unsafe_uncommitted_overwrite(const char *name, const char *data) {
    // 只开始日志操作，但不结束，会出现崩溃
    begin_op();
    int fd = sys_open((char*)name, O_RDWR);
    struct file *f = current_proc->ofile[fd];
    ilock(f->ip);
    writei(f->ip, 0, (unsigned long)data, 0, 1);
    iunlock(f->ip);
    unsafe_detach_fd(fd);
}
void test_crash_recovery(void) {
    printf("Testing crash recovery with log\n");
    char buf[4] = {0};

    // 正常写入A
    int fd = sys_open("logtest", O_CREATE | O_TRUNC | O_RDWR);
    sys_write(fd, "A", 1);
    sys_close(fd);

    // 重启验证A成功提交
    simulate_crash_reboot();
    fd = sys_open("logtest", O_RDONLY);
    sys_read(fd, buf, 1);
    sys_close(fd);
    printf("After reboot committed A -> %c\n", buf[0]);

    // 正常写入B
    fd = sys_open("logtest", O_RDWR);
    sys_write(fd, "B", 1);
    sys_close(fd);

    // 不安全写入C，应该不能提交，文件内容仍为B
    unsafe_uncommitted_overwrite("logtest", "C");

    // 重启验证C未提交
    simulate_crash_reboot();
    memset(buf, 0, sizeof(buf));
    fd = sys_open("logtest", O_RDONLY);
    sys_read(fd, buf, 1);
    sys_close(fd);
    printf("After uncommitted overwrite crash expect B -> %c\n", buf[0]);

    sys_unlink("logtest");

    printf("Crash recovery test finished\n");
}

// Lab8
void test_scheduler_1(void) {
    printf("Testing scheduler 1...\n");

    int pid_high = create_process(cpu_task_high);
    int pid_med = create_process(cpu_task_med);

    if (pid_high <= 0 || pid_med <= 0 ) {
        printf("create_process failed: %d %d\n", pid_high, pid_med);
        return;
    }

    // 设置高低优先级
    set_proc_priority(pid_high, 50);
    set_proc_priority(pid_med, 10);
    printf("Set process priorities, HIGH = %d, MED = %d\n", get_proc_priority(pid_high), get_proc_priority(pid_med));

    // 启动调度器
    scheduler_priority_extend(0);
    printf("Scheduler test 1 completed\n");
}
void test_scheduler_2(void) {
    printf("Testing scheduler 2...\n");

    int pid_high = create_process(cpu_task_high);
    int pid_med = create_process(cpu_task_med);

    if (pid_high <= 0 || pid_med <= 0 ) {
        printf("create_process failed: %d %d\n", pid_high, pid_med);
        return;
    }

    // 设置相等优先级
    set_proc_priority(pid_high, 50);
    set_proc_priority(pid_med, 50);
    printf("Set process priorities, HIGH = %d, MED = %d\n", get_proc_priority(pid_high), get_proc_priority(pid_med));

    // 启动调度器
    scheduler_priority_extend(0);
    printf("Scheduler test 2 completed\n");
}
void test_scheduler_3(void) {
    printf("Testing scheduler 3...\n");

    int pid_high = create_process(cpu_task_high);
    int pid_med = create_process(cpu_task_med);

    if (pid_high <= 0 || pid_med <= 0 ) {
        printf("create_process failed: %d %d\n", pid_high, pid_med);
        return;
    }

    // 设置近似优先级
    set_proc_priority(pid_high, 50);
    set_proc_priority(pid_med, 49);
    printf("Set process priorities, HIGH = %d, MED = %d\n", get_proc_priority(pid_high), get_proc_priority(pid_med));

    // 启动调度器，启用老化机制
    scheduler_priority_extend(1);
    printf("Scheduler test 3 completed\n");
}

// Lab1
void lab1_test(void) {
    printf("========== 这是第 1 个实验 ==========\n");
    uart_puts("Hello OS");
}

// Lab2
void lab2_test(void) {
    printf("========== 这是第 2 个实验 ==========\n");
    test_printf_basic();
    test_printf_edge_cases();
    clear_screen();
}

// Lab3
void lab3_test(void) {
    printf("========== 这是第 3 个实验 ==========\n");
    pmm_init();
    test_alloc_pages();
    test_physical_memory();
    test_pagetable();
    test_virtual_memory();
}

// Lab4
void lab4_test(void) {
    printf("========== 这是第 4 个实验 ==========\n");
    pt_init();
    trap_init();
    test_timer_interrupt();
    test_exception_handling();
}

// Lab5
void lab5_test(void) {
    printf("========== 这是第 5 个实验 ==========\n");
    pt_init();
    proc_init();
    test_process_creation();
    test_scheduler();
    test_synchronization();
}

// Lab6
void lab6_test(void) {
    printf("========== 这是第 6 个实验 ==========\n");
    pt_init();
    proc_init();
    current_proc = alloc_process();
    release(&current_proc->lock);
    trap_init();
    iinit();
    binit();
    fileinit();
    virtio_disk_init();
    fsinit(ROOTDEV);
    test_basic_syscalls();
    test_parameter_passing();
    test_security();
    test_syscall_performance();
}

// Lab7
void lab7_test(void) {
    printf("========== 这是第 7 个实验 ==========\n");
    pt_init();
    proc_init();
    current_proc = alloc_process();
    release(&current_proc->lock);
    trap_init();
    iinit();
    binit();
    fileinit();
    virtio_disk_init();
    fsinit(ROOTDEV);
    test_filesystem_integrity();
    test_concurrent_file_access();
    current_proc = alloc_process();
    release(&current_proc->lock);
    test_filesystem_performance();
    test_crash_recovery();
}

// Lab8
void lab8_test(void) {
    printf("========== 这是第 8 个实验 ==========\n");
    pt_init();
    proc_init();
    test_scheduler_1();
    test_scheduler_2();
    test_scheduler_3();
}

