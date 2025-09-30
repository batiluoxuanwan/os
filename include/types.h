#ifndef TYPES_H
#define TYPES_H

typedef unsigned long long uint64;
typedef long long          int64;
typedef unsigned int       uint32;
typedef int                int32;
typedef unsigned short     uint16;
typedef short              int16;
typedef unsigned char      uint8;
typedef signed char        int8;

// 布尔类型
typedef unsigned char bool;
#define true  1
#define false 0

// NULL 指针
#ifndef NULL
#define NULL ((void*)0)
#endif

#endif // TYPES_H

