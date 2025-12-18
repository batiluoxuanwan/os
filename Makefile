# 定义交叉编译工具链前缀
CROSS = riscv64-unknown-elf-
CC = $(CROSS)gcc
LD = $(CROSS)ld
OBJDUMP = $(CROSS)objdump
NM = $(CROSS)nm
QEMU = qemu-system-riscv64

# Host 工具（用于构建 mkfs）
HOSTCC = gcc
HOSTCFLAGS = -O2 -Wall
MKFS = mkfs/mkfs

# 编译选项
CFLAGS = -Wall -O2 -ffreestanding -nostdlib -nostartfiles -mcmodel=medany
CFLAGS += -Iinclude
LDFLAGS = -T scripts/kernel.ld

# 目标文件
OBJS = \
	kernel/boot/entry.o \
	kernel/boot/start.o \
	kernel/boot/sbi.o \
	kernel/lib/uart.o \
	kernel/lib/console.o \
	kernel/lib/printf.o \
	kernel/lib/snprintf.o \
	kernel/lib/string.o \
	kernel/mm/kalloc.o \
	kernel/mm/vm.o \
	kernel/proc/proc.o \
	kernel/proc/sysproc.o \
	kernel/proc/swtch.o \
	kernel/proc/swtest.o \
	kernel/fs/bio.o \
	kernel/fs/file.o \
	kernel/fs/fs.o \
	kernel/fs/log.o \
	kernel/fs/sysfile.o \
	kernel/fs/virtio_disk.o \
	kernel/sync/spinlock.o \
	kernel/sync/sleeplock.o \
	kernel/trap/trap.o \
	kernel/trap/kernelvec.o \
	kernel/trap/trampoline.o \
	scripts/usys.o


# 最终生成的内核文件
KERNEL = kernel.elf

# 默认目标
all: $(KERNEL)

# 编译汇编文件
%.o: %.S
	$(CC) $(CFLAGS) -c $< -o $@

scripts/usys.S: scripts/usys.pl
	perl $< > $@

scripts/usys.o: scripts/usys.S
	$(CC) $(CFLAGS) -c $< -o $@


# 编译 C 文件
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# 链接生成内核文件
$(KERNEL): $(OBJS)
	$(LD) $(LDFLAGS) $(OBJS) -o $@

# 构建 host 上的 mkfs 工具（用于生成 fs.img）
$(MKFS): mkfs/mkfs.c
	$(HOSTCC) $(HOSTCFLAGS) -I. -o $(MKFS) mkfs.c

# 生成文件系统镜像 fs.img（使用当前目录下的 mkfs）
fs.img: $(MKFS)
	./$(MKFS) fs.img

# 运行内核
qemu: $(KERNEL) fs.img scripts/usys.o
	$(QEMU) -machine virt -nographic -bios default -kernel $(KERNEL) -global virtio-mmio.force-legacy=false -drive file=fs.img,if=none,format=raw,id=x0 -device virtio-blk-device,drive=x0,bus=virtio-mmio-bus.0
# 清理生成的文件
clean:
	rm -f $(OBJS) $(KERNEL) fs.img usys.S usys.o
