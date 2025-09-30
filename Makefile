# ==================================================
# RISC-V OS Makefile
# 支持编译 kernel.c/.S 文件并链接生成 ELF
# 支持 QEMU 运行和调试
# ==================================================

# 工程目录
KERNEL_DIR := kernel
SCRIPT_DIR := scripts

# 工具链前缀
TOOLCHAIN_PREFIX := riscv64-unknown-elf-
CC := $(TOOLCHAIN_PREFIX)gcc

# 编译参数
CFLAGS := -Wall -Werror -O -fno-omit-frame-pointer -ggdb
CFLAGS += -mcmodel=medany -ffreestanding
CFLAGS += -Iinclude                    # 公共头文件
CFLAGS += $(foreach dir,$(wildcard $(KERNEL_DIR)/*/),-I$(dir))  # kernel 子目录作为 include 路径

# 链接参数
LDFLAGS := -T $(SCRIPT_DIR)/kernel.ld -Wl,-no-relax -nostdlib

# 查找所有 C 和汇编源文件
C_SRCS := $(shell find $(KERNEL_DIR) -name "*.c")
S_SRCS := $(shell find $(KERNEL_DIR) -name "*.S")

# 目标文件
OBJS   := $(C_SRCS:.c=.o) $(S_SRCS:.S=.o)
TARGET := kernel.elf

.PHONY: all clean qemu debug

# ==================================================
# 默认目标
all: $(TARGET)

# ==================================================
# 链接生成 ELF 文件
# entry.o 放在最前面，确保 _entry 符号优先
$(TARGET): $(OBJS) $(SCRIPT_DIR)/kernel.ld
	@echo "Linking $(TARGET)..."
	$(CC) -march=rv64imac -mabi=lp64 $(LDFLAGS) $(filter $(KERNEL_DIR)/entry.o,$(OBJS)) $(filter-out $(KERNEL_DIR)/entry.o,$(OBJS)) -o $@
	@echo "Done."

# ==================================================
# 编译 C 文件
%.o: %.c
	@echo "Compiling $<..."
	$(CC) $(CFLAGS) -c $< -o $@

# 编译汇编文件
%.o: %.S
	@echo "Assembling $<..."
	$(CC) $(CFLAGS) -c $< -o $@

# ==================================================
# 运行 QEMU
qemu: $(TARGET)
	@echo "Starting QEMU..."
	qemu-system-riscv64 -nographic -machine virt -m 128M -bios none -kernel $(TARGET)

# 调试模式运行 QEMU（GDB 调试）
debug: $(TARGET)
	@echo "Starting QEMU in debug mode..."
	qemu-system-riscv64 -nographic -machine virt -m 128M -bios none -kernel $(TARGET) -s -S

# ==================================================
# 清理中间文件
clean:
	@echo "Cleaning up..."
	rm -f $(OBJS) $(TARGET)
	@echo "Done."

