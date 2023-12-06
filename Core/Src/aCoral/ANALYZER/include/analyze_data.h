

#ifndef __ANALYZE_DATA_H
#define __ANALYZE_DATA_H
    /* 系统分析宏定义 */
    #define ANALYZE
#endif

#ifdef ANALYZE2
/* 线程状态改变 */
struct ThreadStateChange {
    unsigned int threadId;              // 线程ID
    unsigned int startTimestamp;        // 线程状态开始改变的时间戳
    unsigned int endTimestamp;          // 线程状态改变完成的时间戳
    unsigned char oldState;             // 旧线程状态
    unsigned char newState;             // 新线程状态
};

/* 线程分配内存 */
struct MemoryAllocation {
    unsigned int threadId;              // 线程ID
    unsigned int startTimestamp;        // 分配开始时间戳
    unsigned int endTimestamp;          // 分配完成时间戳
    unsigned int startAddress;          // 分配操作的起始地址
    unsigned int endAddress;            // 分配操作的结束地址
};

/* 线程释放内存 */
struct MemoryDeallocation {
    unsigned int threadId;              // 线程ID
    unsigned int startTimestamp;        // 释放开始时间戳
    unsigned int endTimestamp;          // 释放完成时间戳
    unsigned int startAddress;          // 释放操作的起始地址
    unsigned int endAddress;            // 释放操作的结束地址
};

/* 线程切换 */
struct ThreadSwitchOverhead {
    unsigned int threadFromId;          // 被切换线程ID
    unsigned int threadToId;            // 切换线程ID
    unsigned int startTimestamp;        // 切换开始时间戳
    unsigned int endTimestamp;          // 切换结束时间戳
};

/* 中断处理 */
struct InterruptResponseOverhead {
    unsigned int interruptType;         // 中断类型
    unsigned int startTimestamp;        // 中断处理开始时间戳
    unsigned int endTimestamp;          // 中断处理结束时间戳
};

SysTick->VAL;

#endif
