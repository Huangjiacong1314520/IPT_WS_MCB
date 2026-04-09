# 多核 DSP 详解（第三部分）：多核协作篇

> 这是《多核 DSP 详解》的第三部分，涵盖第 6-8 章。**这是全文最核心、最有干货的一部分**。前两部分讲的是"一个核内部怎么运作"，从这一部分开始我们真正进入"多核"的世界——核与核之间怎么说话、怎么同步、怎么避免踩坑。
>
> 如果你是运动控制工程师，这一部分值得反复读，因为**运动控制项目里大部分诡异的 bug 都出在多核同步和 Cache 一致性上**。

---

# 第三部分 · 多核协作篇

---

## 第 6 章 核间通信机制

### 6.1 三个核要协作，得先学会"说话"

想象一下这样的场景：CORE0 读到了一批新的传感器数据，想让 CORE1 接着处理。它怎么告诉 CORE1 "数据好了，快来干活"？

你可能第一反应是："**用全局变量呗**"——CORE0 设置一个 `flag = 1`，CORE1 死循环检查这个 `flag`。这在单核上没问题，但在多核上有两个大问题：

1. **全局变量放在哪里？** 如果放在 CORE0 的 L2 SRAM 里，CORE1 根本看不到——前面讲过 L2 是每核私有的。
2. **死循环浪费 CPU**：CORE1 站在那儿啥也不干，只是不停地读一个变量，100% 占用算力。

所以核间通信需要解决两个独立的问题：

- **问题 A · 数据怎么传**：CORE0 的数据怎么让 CORE1 也能看见？
- **问题 B · 事件怎么通知**：CORE1 怎么知道"现在该干活了"而不用傻等？

这两个问题在 C6678 上分别有专门的机制来解决：

- **问题 A 的答案**：**MSMC 共享内存**——所有核都能访问的公共区域
- **问题 B 的答案**：**IPC 中断**——CORE0 可以主动触发一个中断"踢醒"其他核

本项目的做法就是把这两者**组合起来使用**：先把数据写进共享内存，然后发一个 IPC 中断通知对方"数据好了，过来取"。这是多核 DSP 最经典的通信模式，叫**共享内存 + 中断通知**。

### 6.2 核间通信的三种基本方式

在展开讲本项目的做法之前，先对整体格局有个概念。多核 DSP 上常见的核间通信方式有三种：

#### 方式一：共享内存（Shared Memory）

**原理**：多个核都能访问同一块物理内存。A 核写进去，B 核读出来。

**优点**：
- 数据量可以很大（几 KB、几 MB 都行）
- 访问方式和普通内存一样，代码简单
- 延迟低（MSMC 大约 20 周期）

**缺点**：
- **需要自己处理同步问题**（数据什么时候可以读？什么时候写完了？）
- **Cache 一致性问题**要手动管理（第 7 章细讲）

**适用场景**：传大块数据。

#### 方式二：邮箱（Mailbox）

**原理**：硬件提供一组小寄存器作为"信箱"。A 核往信箱写一个值，B 核读信箱。写入时可以自动产生中断通知 B 核。

**优点**：
- 硬件保证原子性，不需要手动同步
- 写入就能触发中断，一步到位

**缺点**：
- 容量极小（通常只有几个 32 位字）
- 只适合传小的控制信号，不适合传大数据

**适用场景**：传状态码、命令号之类的小东西。

C6678 上的 Mailbox 不像某些其他 DSP 那样是一组独立硬件，而是**用 IPC 中断寄存器的低位当作小信息字段**来实现的，所以和下面的 IPC 中断是一套机制。

#### 方式三：IPC 中断（Inter-Processor Interrupt）

**原理**：A 核写一个专门的寄存器，硬件就会在 B 核上产生一个中断。B 核进入中断服务函数，知道"A 核来消息了"。

**优点**：
- **事件驱动**，B 核不用轮询
- 响应及时（中断延迟通常 < 100 ns）

**缺点**：
- 只是"通知"，不传数据（或只能传极少的数据，如几位信息字段）
- 需要和共享内存配合才能完成"数据 + 通知"的完整通信

**适用场景**：事件通知、栅栏同步、唤醒休眠的核。

#### 三种方式的组合：本项目的选择

看完三种方式的特点就明白为什么本项目用"**MSMC 共享内存 + IPC 中断**"的组合了：

- 传感器数据、控制量这些**大块数据**通过共享内存传
- "**数据好了**"这种**事件**通过 IPC 中断通知
- 两者互补，既高效又解耦

这也是多核 DSP 编程里的常见做法。下面分别详细展开。

### 6.3 MSMC 共享内存：核间通信的"公告板"

第 4 章提到过 MSMC（Multicore Shared Memory Controller）是 C6678 上一块 **4 MB 的 SRAM**，地址从 `0x0C000000` 开始，所有 8 个核都能直接读写。在链接脚本里叫 `SHRAM`：

```cmd
SHRAM: o = 0x0C000000 l = 0x00400000   /* 4MB Multicore shared Memmory */
```

#### 6.3.1 怎么让 C 代码访问 MSMC

最朴素的方法是直接用地址：

```c
// 定义一个指向 MSMC 的指针
volatile unsigned int *shared = (volatile unsigned int *)0x0C000000;

// CORE0 写
shared[0] = 42;

// CORE1 读
int value = shared[0];
```

这样能工作，但代码丑、容易出错——要是你想放一个结构体而不只是整数，地址计算会很痛苦。所以实际工程里的做法是：**定义一个共享内存的结构体，然后让它固定放在 MSMC 的某个地址**。

#### 6.3.2 本项目的做法：MultiCoreShareData

看 `MCB_CORE0/control/interrupt.c` 里大量出现的这种访问方式：

```c
MultiCoreShareData->CurState = Default;
MultiCoreShareData->Phase[0] = Free;
MultiCoreShareData->Phase[1] = Free;
// ...
MultiCoreShareData->Phase[8] = Free;
```

`MultiCoreShareData` 是一个**指向共享内存结构体的全局指针**，所有核都用同一个指针名访问同一块共享数据。这种写法的好处是：

1. **代码可读性强**：像访问普通结构体成员一样使用
2. **结构化**：共享数据有清晰的类型和字段
3. **容易维护**：加字段改字段只动一个头文件

#### 6.3.3 共享结构体的典型布局

本项目共享结构体里目前能看到的字段有：

- `CurState`：系统当前状态（Silence、Activate、Default、Initialize、SpecificTask、Stop 等）。**所有核都能读**，用来知道"系统现在处于什么阶段"。
- `Phase[0..8]`：9 个元素的阶段数组，用作**栅栏同步标志**（后面第 7 章展开）。
- 传感器数据区：CORE0 采集完传感器后，把预处理好的数据放这里。
- 控制量数据区：从核算完控制量后，把结果放这里。
- 参数区：运行时可调的控制参数（PID 增益、滤波系数等）。

可以把 `MultiCoreShareData` 想象成一张**所有核共用的白板**——CORE0 在上面写传感器读数，从核看一眼白板知道这一轮要算什么；从核把结果写到白板另一块区域，CORE0 再看白板知道大家都算完了。整个控制周期就在这块白板上完成数据交换。

#### 6.3.4 共享内存的地址怎么约定

这里有一个容易被初学者忽视的关键问题：**CORE0 的代码里的 `MultiCoreShareData` 和 CORE1 的代码里的 `MultiCoreShareData`，怎么保证它们指向同一块物理内存？**

答案是：**两边约定好同一个绝对地址**。在本项目里，CORE0 工程和 CORE1 工程各自编译各自的 `.out`，但两边的头文件里都把 `MultiCoreShareData` 指针初始化为同一个 MSMC 地址（比如 `0x0C001000`）。这样虽然是两份独立编译出来的代码，但它们**物理上访问同一块内存**。

这种约定可以写在一个共享的头文件里（比如 `msmAddr.h`），CORE0 和 CORE1 的工程都包含它，就能保证地址一致。

#### 6.3.5 MSMC 的内存布局规划

4 MB 说大不大说小不小，实际用的时候需要规划。典型的 MSMC 分区大概是这样的：

```
0x0C000000 +-------------------+
           |   系统状态区      |  几十字节
           |  (CurState,...)   |
0x0C000100 +-------------------+
           |   同步标志区      |  几十字节
           |  (Phase[], Lock)  |
0x0C000200 +-------------------+
           |   传感器数据区    |  几 KB
           |   (CORE0 写入)    |
0x0C010000 +-------------------+
           |   控制量数据区    |  几 KB
           |   (从核写入)      |
0x0C020000 +-------------------+
           |   参数/配置区     |  几十 KB
0x0C100000 +-------------------+
           |                   |
           |   可扩展预留区    |
           |                   |
0x0C400000 +-------------------+
```

**为什么要分区？** 因为不同数据的**读写方向**不同——CORE0 写传感器、从核读传感器；从核写控制量、CORE0 读控制量。**Cache 一致性操作是按地址范围做的**，把不同方向的数据分开存，才能精确地刷新需要刷的那部分，不浪费时间也不误伤。这个思路在第 7 章讲 Cache 一致性时会更清楚。

### 6.4 IPC 中断：一声"铃响"，把另一个核踢醒

**IPC** 的全称是 **Inter-Processor Communication**，中文叫**处理器间通信**，在 C6678 语境下特指**核与核之间触发中断**的机制。

#### 6.4.1 IPC 中断的硬件原理

C6678 为每个核都准备了两个专门的寄存器：

- **IPCGR**（IPC Generation Register，IPC 产生寄存器）：**别的核**写这个寄存器可以给**本核**产生一个中断
- **IPCAR**（IPC Acknowledge Register，IPC 确认寄存器）：本核用来确认/清除收到的中断

每个核有自己的 IPCGR 和 IPCAR，总共 8 组。从 `MCB_CORE0/ipc/drv_ipc.c` 里能看到这 8 组寄存器的地址：

```c
#define CHIP_LEVEL_REG  0x02620000

/* IPCGR Info - 8 个核各有一个 IPCGR */
unsigned int iIPCGRInfo[8] = {
    CHIP_LEVEL_REG + 0x240,    // CORE0 的 IPCGR
    CHIP_LEVEL_REG + 0x244,    // CORE1 的 IPCGR
    CHIP_LEVEL_REG + 0x248,    // CORE2 的 IPCGR
    CHIP_LEVEL_REG + 0x24C,    // CORE3 的 IPCGR
    CHIP_LEVEL_REG + 0x250,    // CORE4 的 IPCGR
    CHIP_LEVEL_REG + 0x254,    // CORE5 的 IPCGR
    CHIP_LEVEL_REG + 0x258,    // CORE6 的 IPCGR
    CHIP_LEVEL_REG + 0x25C     // CORE7 的 IPCGR
};

/* IPCAR Info - 8 个核各有一个 IPCAR */
unsigned int iIPCARInfo[8] = {
    CHIP_LEVEL_REG + 0x280,    // CORE0 的 IPCAR
    CHIP_LEVEL_REG + 0x284,
    CHIP_LEVEL_REG + 0x288,
    CHIP_LEVEL_REG + 0x28C,
    CHIP_LEVEL_REG + 0x290,
    CHIP_LEVEL_REG + 0x294,
    CHIP_LEVEL_REG + 0x298,
    CHIP_LEVEL_REG + 0x29C
};
```

**代码在说什么？** 它告诉我们每个核的 IPCGR/IPCAR 寄存器的物理地址。比如 CORE1 的 IPCGR 在 `0x02620244`，任何核只要往这个地址写一个值，CORE1 就会收到一个 IPC 中断。

#### 6.4.2 触发 IPC 中断的函数

继续看 `drv_ipc.c`：

```c
void IPC_SendInterruptToCoreX(unsigned char nNextCore, unsigned int nInfo)
{
    // Unlock Config —— 先解锁 Kicker（下一节解释）
    KICK0 = KICK0_UNLOCK;
    KICK1 = KICK1_UNLOCK;

    // 把 nInfo 写进目标核的 IPCGR 寄存器
    *(volatile unsigned int *)iIPCGRInfo[nNextCore] = nInfo;

    // 再写一次，低位置 1 触发中断
    *(volatile unsigned int *)iIPCGRInfo[nNextCore] |= 1;

    // Unlock Config
    KICK0 = KICK0_UNLOCK;
    KICK1 = KICK1_UNLOCK;
}
```

**这个函数在做什么？** 它把一个 32 位的 `nInfo` 写进目标核（`nNextCore`）的 IPCGR 寄存器，然后把最低位置 1。**最低位就是"中断触发位"**——它从 0 变成 1 的瞬间，目标核就会产生一个 IPC 中断。高 31 位可以携带一些小信息（比如"这次是要做什么任务"）传给目标核。

你会注意到 `IPC_SendInterruptToCoreX` 这个函数目前在 `interrupt.c` 里没有直接被调用，取而代之的是 TI 官方 CSL 库提供的 `CSL_IPC_genGEMInterrupt()`：

```c
KickUnlock();
CSL_IPC_genGEMInterrupt(1, 0);  // 触发 CORE1 的 IPC 中断
CSL_IPC_genGEMInterrupt(2, 0);  // 触发 CORE2
CSL_IPC_genGEMInterrupt(3, 0);  // 触发 CORE3
CSL_IPC_genGEMInterrupt(7, 0);  // 触发 CORE7
KickLock();
```

两个函数做的事情一样——都是往 IPCGR 写值触发中断。前者是项目自己封装的版本，后者是 TI CSL（Chip Support Library，芯片支持库）提供的官方版本。项目用的是官方版，自己封装的 `IPC_SendInterruptToCoreX` 可能是早期代码的遗留。

#### 6.4.3 接收 IPC 中断的一侧

从核（CORE1/CORE7）那一侧怎么响应 IPC 中断？流程是：

1. 在初始化阶段，从核**注册一个中断服务函数**（ISR）到 IPC 中断的中断号上
2. 从核进入主循环或等待状态
3. CORE0 触发 IPC 中断，从核的 CPU 立刻跳到 ISR
4. ISR 里读 IPCGR 看看是什么事件（高位的信息字段），然后调用对应的处理函数（比如"开始算位置环"）
5. 处理完后调用 `IPC_ClearSourceInfo()` 清除中断标志，等下次

`drv_ipc.c` 里的清除函数：

```c
void IPC_GetCoreSoureInfo(unsigned char nCore, unsigned int * pInfo)
{
    *pInfo = *(volatile unsigned int*)iIPCGRInfo[nCore];
}

void IPC_ClearSourceInfo(unsigned char nCore)
{
    // 把 IPCGR 的值写到 IPCAR，就清除了对应的中断标志
    *(volatile unsigned int *)iIPCARInfo[nCore] = *(volatile unsigned int*)iIPCGRInfo[nCore];
}
```

**这里有一个小细节**：清除中断的方式是把 IPCGR 的当前值**原样写到 IPCAR**。这是因为 IPCGR 里可能同时有多个源触发的中断挂着（虽然每次只触发一位，但如果前一次没清干净会累积），IPCAR 的含义是"**把这些位都确认清除**"。

### 6.5 Kicker 机制：TI 的防误写设计

前面在 `IPC_SendInterruptToCoreX` 里看到了两对神秘的东西：

```c
KICK0 = KICK0_UNLOCK;    // 0x83E70B13
KICK1 = KICK1_UNLOCK;    // 0x95A4F1E0
```

这是什么？为什么每次写 IPCGR 前后都要来这么一下？

#### 6.5.1 为什么需要 Kicker

C6678 作为一颗复杂的 SoC（System on Chip，片上系统），有很多关键寄存器控制着整颗芯片的基本行为——时钟配置、PLL、引脚复用、中断路由、**IPC 中断生成**等等。**这些寄存器如果被误写了，后果很严重**：轻则某个外设挂掉，重则整个芯片死机。

这种"误写"在嵌入式开发里其实很常见——比如一个指针用错了，或者数组越界写到了一个关键寄存器的地址。调试这种 bug 非常痛苦。

TI 的设计思路是：**给这些关键寄存器加一把"锁"，平时锁着，要写之前必须先解锁，写完再锁上。** 这把锁就叫 **Kicker**（意思像"启动机构"）。

#### 6.5.2 Kicker 的工作方式

从 `MCB_CORE0/ipc/drv_ipc.c` 开头的定义：

```c
#define CHIP_LEVEL_REG  0x02620000

#define KICK0       *(unsigned int*)(CHIP_LEVEL_REG + 0x0038)
#define KICK1       *(unsigned int*)(CHIP_LEVEL_REG + 0x003C)
#define KICK0_UNLOCK (0x83E70B13)
#define KICK1_UNLOCK (0x95A4F1E0)
#define KICK_LOCK    0
```

KICK0 和 KICK1 是**两个特殊的寄存器**，位于 `0x02620038` 和 `0x0262003C`。它们就是那把"锁"。

**解锁的步骤**：

1. 往 KICK0 写入魔数 `0x83E70B13`
2. 紧接着往 KICK1 写入魔数 `0x95A4F1E0`
3. 顺序和数值都不能错，错了就锁不开

**锁上**：往 KICK0 或 KICK1 写入 0（或任何不是那两个魔数的值）。

这两个魔数不是随便挑的——它们是足够"特殊"的值，普通的野指针误写几乎不可能凑巧写出这两个数，所以能有效防止"意外解锁"。

#### 6.5.3 在代码里怎么用

本项目封装了 `KickUnlock()` 和 `KickLock()` 两个宏或函数，把它们当作"临界区"的括号：

```c
KickUnlock();                       // 开锁
CSL_IPC_genGEMInterrupt(1, 0);      // 写 IPC 寄存器
CSL_IPC_genGEMInterrupt(2, 0);
CSL_IPC_genGEMInterrupt(3, 0);
CSL_IPC_genGEMInterrupt(7, 0);
KickLock();                         // 锁回去
```

一对 `Unlock/Lock` 里可以做**多次**关键寄存器写操作，不用每次都解锁-加锁，提高效率。

> **小提醒**：Kicker 只保护一部分关键寄存器，不是所有寄存器都需要解锁。哪些寄存器受 Kicker 保护要看 C6678 的数据手册。IPC 生成寄存器是其中之一，所以触发 IPC 中断必须先解锁。

#### 6.5.4 从 Kicker 能学到什么

Kicker 机制看似不起眼，但它反映了一个很重要的工程思想：**在硬件层面做防御性设计**。这种思路在安全关键（safety-critical）系统里非常常见——航空电子、汽车电子、医疗设备都有类似的保护机制。做运动控制也一样，一个野指针写错了可能会让电机失控，这种代价是承受不起的，所以多一层保护总是好的。

### 6.6 为什么本项目选择"IPC 中断 + MSMC 共享内存"组合

回到开头的问题。现在我们可以给出完整的答案了：

**传数据用 MSMC 共享内存，因为：**
- 传感器数据、控制量这些一次几百字节到几 KB，Mailbox 装不下
- 访问方式简单，C 代码里像访问普通结构体一样
- 延迟稳定，不会抖动
- 4 MB 空间足够划分多个区域（状态、数据、参数等）

**发通知用 IPC 中断，因为：**
- 从核不用轮询，省 CPU 给计算用
- 响应极快，几十纳秒级
- 可以**同时**触发多个核（CORE0 一次踢醒 4 个从核），天然支持并行
- 事件驱动模型清晰，容易调试

**两者配合的时序是这样的**：

```
时间 →

CORE0:  [读传感器] → [写 MSMC] → [Kick踢从核] → [等从核完成] → [读 MSMC] → [发 FPGA]
                                       ↓
CORE1:                            [被中断唤醒]      [算完]
                                   ↓                ↑
                                [读 MSMC] → [算] → [写 MSMC]
```

**这个配合的精妙之处**：

1. CORE0 写 MSMC 的时候，CORE1 还没被唤醒，所以不存在"同时读写"的冲突
2. CORE1 在算的时候，CORE0 在等待，也不会同时读写 CORE1 正在写的区域
3. CORE0 和 CORE1 读写的是 MSMC 里**不同的区域**（传感器数据区 vs 控制量数据区），即便有时间重叠也不冲突

这就是为什么**本项目能用看起来很"土"的 `Phase[]` 标志位做同步**——因为时序本身保证了数据访问不会冲突，同步只需要"告诉对方你做完了"这么简单。这种设计叫**时序隔离**，是运动控制系统里保证正确性的常用手段。

### 6.7 第 6 章小结

1. **核间通信要解决两个问题**：数据传输 + 事件通知
2. **三种基本方式**：共享内存（大数据）、Mailbox（小消息）、IPC 中断（事件）
3. **本项目的选择是"MSMC 共享内存 + IPC 中断"组合**：共享内存传数据，IPC 中断传信号
4. **MSMC 是 4 MB 的多核共享 SRAM**，所有核约定同一个基地址访问同一个结构体（MultiCoreShareData）
5. **IPC 中断通过 IPCGR 寄存器触发**：核 A 写核 B 的 IPCGR → 核 B 产生中断
6. **Kicker 机制**是 TI 的防误写设计，写 IPC 这类关键寄存器前必须先用两个魔数（`0x83E70B13` / `0x95A4F1E0`）解锁
7. **时序隔离**让"Phase[] 标志位 + 共享内存"的简单做法成为可能，不需要复杂的互斥锁

但上面这一切有一个**重大前提**没有讲——**Cache 一致性**。共享内存共享的是物理地址没错，但数据的"最新版本"可能被卡在某个核的 Cache 里。这是多核编程里最容易踩的坑，也是下一章的主题。

---

## 第 7 章 同步与 Cache 一致性

### 7.1 先讲一个诡异的 bug 场景

想象下面这段代码：

```c
// CORE0 的代码
void CORE0_Task() {
    MultiCoreShareData->sensor_value = 12345;    // 写入共享内存
    CSL_IPC_genGEMInterrupt(1, 0);               // 踢 CORE1
}

// CORE1 的中断服务函数
void CORE1_IPC_ISR() {
    int v = MultiCoreShareData->sensor_value;    // 读共享内存
    printf("got: %d\n", v);
}
```

按道理应该打印 `got: 12345`，对吧？

**但你很可能看到 `got: 0` 或者上一次残留的旧值。** 这就是 Cache 一致性问题。原因很诡异也很重要——我们下面一步一步拆解。

### 7.2 为什么会这样：Cache 不听话

#### 7.2.1 复习 Cache 的工作方式

Cache 的基本工作原理是：

- CPU 读一个地址时，Cache 先查自己有没有这个地址的副本
- 如果有（**命中**），直接从 Cache 返回，不用去主存
- 如果没有（**缺失**），去主存把整个 Cache 行（通常 64 或 128 字节）搬到 Cache，再返回

写的时候有两种策略：

- **Write-through（直写）**：写 Cache 的同时写主存，速度慢但主存永远是最新的
- **Write-back（写回）**：只写 Cache，打个"脏"标记；只有当 Cache 行被换出或者强制刷新时，才把脏数据写回主存。C66x 用的是**写回策略**（更快，但有一致性风险）。

#### 7.2.2 多核场景下的"两个副本"问题

回到前面的例子：

**第 1 步**：CORE0 执行 `MultiCoreShareData->sensor_value = 12345`。

- CPU 试图写地址 `0x0C00xxxx`（MSMC 里的共享数据）
- 这个地址的 Cache 行已经被加载到 CORE0 的 L1D Cache 里（之前的访问带进来的）
- CPU 只写到 L1D Cache，**不写主存**（写回策略）
- 此时 L1D Cache 里的值是 12345，但 **MSMC 里的值还是旧的**

**第 2 步**：CORE0 触发 IPC 中断。

**第 3 步**：CORE1 被唤醒，执行 `int v = MultiCoreShareData->sensor_value`。

- CPU 试图读地址 `0x0C00xxxx`
- **CORE1 的 L1D Cache 里也有这个地址的副本**（可能是之前读过的残留），里面的值是 0 或旧值
- CPU 命中 CORE1 的 L1D，直接返回 **0 或旧值**
- 主存里还是旧值，CORE0 写入的 12345 此时**只存在于 CORE0 的 Cache 里**

**结果**：CORE1 读到了错误的数据。

#### 7.2.3 问题的本质

**每个核有自己独立的 L1D 和 L2 Cache，它们之间不会自动同步。**

"自动同步"这件事在**某些多核 CPU 上有硬件支持**（叫 Cache Coherence Protocol，比如 MESI 协议），Intel/ARM 的现代处理器都有。但**C66x 没有硬件 Cache 一致性**——这是 DSP 为了追求极致性能和低功耗做的取舍。代价就是：**核间共享数据的一致性，完全由程序员手动管理**。

> **建议搜索图**：「cache coherence problem」或者「多核 Cache 一致性问题」，能看到直观的示意图。

### 7.3 解决办法：手动刷 Cache

既然硬件不自动同步，那就**手动**。手动的方法有两种：

- **Writeback（写回）**：把 Cache 里的脏数据主动写回主存
- **Invalidate（无效化）**：把 Cache 里的副本标记为"失效"，下次读的时候强制从主存重新加载

#### 7.3.1 Writeback 和 Invalidate 的区别

这两个操作的方向正好相反，务必分清楚：

| 操作 | 作用 | 什么时候用 |
|------|------|-----------|
| **Writeback（写回）** | 把 Cache 的脏数据推到主存 | **写之后**：你写了共享内存，要让别人看得到 |
| **Invalidate（无效化）** | 把 Cache 里的副本扔掉 | **读之前**：别人写了共享内存，你要读到新值 |

**一个简单的记忆方法**：

- 我**写**完，要让别人看到 → Writeback（把我这边的修改推出去）
- 我要**读**，不能用自己 Cache 里的陈旧副本 → Invalidate（把我这边的旧副本扔掉）

#### 7.3.2 C66x 的 Cache 操作寄存器

`include/hw_C6678.h` 里定义了这几个关键寄存器：

```c
#define CACHE_BASE          0x01840000
#define L2WBINV             (CACHE_BASE + 0x5004) // L2 Writeback + Invalidate
#define L2INV               (CACHE_BASE + 0x5008) // L2 Invalidate
#define L1PINV              (CACHE_BASE + 0x5028) // L1P Invalidate
#define L1DWBINV            (CACHE_BASE + 0x5044) // L1D Writeback + Invalidate
#define L1DINV              (CACHE_BASE + 0x5048) // L1D Invalidate
```

这些寄存器分别对应：

- **L1DWBINV**：把 L1D Cache 的某段地址**写回并无效化**——先把脏数据推到下一级，再扔掉副本
- **L1DINV**：直接**无效化** L1D Cache 的某段地址——不写回，直接扔
- **L2WBINV** / **L2INV**：对 L2 Cache 做同样的事
- **L1PINV**：无效化 L1P（指令 Cache），用于代码被修改时

**使用方法**：往这些寄存器写一个地址值，硬件就会对那一小块（一个 Cache 行）做对应操作。TI CSL 库和 `drv_cache.c` 通常会封装成函数，让你传入一个地址范围（起始地址 + 字节数），函数内部循环处理一个个 Cache 行。

#### 7.3.3 典型的函数接口

本项目的 Cache 操作封装大概长这样（注意 `KeyStone_Navigator_init_drv.c` 里就出现过 `WritebackCache()` 调用）：

```c
// 写回一段地址的 L1D 和 L2 Cache（把脏数据推到下一级）
void WritebackCache(void *addr, unsigned int size);

// 无效化一段地址的 L1D 和 L2 Cache（扔掉副本，下次读强制重新加载）
void InvalidateCache(void *addr, unsigned int size);

// 写回并无效化（两个操作一起做）
void WritebackInvalidateCache(void *addr, unsigned int size);
```

一般 `drv_cache.c` 里就是这三个函数，加上一些底层寄存器操作。

### 7.4 一致性操作要放在哪个时间点

理解了"为什么"和"怎么做"，还有一个关键问题：**什么时候做**。放错时间点等于没做。

#### 7.4.1 写方的时间点：先写数据，再 Writeback，最后发通知

**错误做法**：

```c
WritebackCache(&shared->data, 100);    // ← 先 Writeback
shared->data = 12345;                   // 再写 ← 这个改动没被刷！
CSL_IPC_genGEMInterrupt(1, 0);          // 通知对方
```

**正确做法**：

```c
shared->data = 12345;                   // 先写
WritebackCache(&shared->data, 100);    // 再 Writeback
CSL_IPC_genGEMInterrupt(1, 0);          // 最后通知对方
```

**顺序必须是**：**写数据 → Writeback → 发通知**。这个顺序保证了当对方收到通知时，数据一定已经从你的 Cache 推到了共享内存。

#### 7.4.2 读方的时间点：先 Invalidate，再读

**错误做法**：

```c
int v = shared->data;                    // 直接读 ← 可能读到自己 Cache 里的旧值
InvalidateCache(&shared->data, 100);    // 再 Invalidate ← 已经晚了
```

**正确做法**：

```c
InvalidateCache(&shared->data, 100);    // 先 Invalidate（扔掉旧副本）
int v = shared->data;                    // 再读 ← 强制从主存加载，拿到新值
```

**顺序必须是**：**Invalidate → 读数据**。

#### 7.4.3 把两方结合起来

一次完整的"CORE0 写，CORE1 读"的通信序列是这样的：

```
CORE0 这边：
    1. shared->data = 新值
    2. WritebackCache(&shared->data, size)
    3. CSL_IPC_genGEMInterrupt(1, 0)   ← 踢 CORE1

CORE1 这边（被中断唤醒后）：
    4. InvalidateCache(&shared->data, size)
    5. int v = shared->data
    6. 使用 v 做计算
```

**如果 CORE1 算完之后还要把结果写回共享内存给 CORE0 读**，那就还要加：

```
CORE1 继续：
    7. shared->result = 算出来的值
    8. WritebackCache(&shared->result, size)
    9. shared->Phase[x] = Free    ← 通知 CORE0 "我算完了"
    10. WritebackCache(&shared->Phase[x], size)   ← Phase 标志也要刷

CORE0 这边轮询等：
    11. InvalidateCache(&shared->Phase, size)
    12. if (Phase[x] == Free) { ... 全部完成 ... }
    13. InvalidateCache(&shared->result, size)
    14. 读 shared->result
```

看起来很繁琐？**是的，就是这么繁琐**。这就是 DSP 多核编程比 PC 多线程编程难的地方——PC 上 CPU 硬件帮你做了一致性，DSP 上需要你自己做。好消息是只要理清楚"谁写谁读、什么时候"，按模板写就不会错。

### 7.5 Phase[] 标志位作为"轻量级栅栏"

**栅栏**（Barrier）是并行编程里的一个概念：**所有参与的核都到达某个点之后才能继续往下走**。像赛跑里的"所有人都跑完这一圈才算这一轮结束"。

#### 7.5.1 为什么运动控制需要栅栏同步

在本项目里，CORE0 每个控制周期都需要等待**所有从核**算完各自的任务后，才能汇总结果发给 FPGA。如果某个从核还没算完 CORE0 就读结果，读到的就是上一周期的陈旧数据，电机会接收到过期的控制量——这是运动控制里最可怕的错误之一。

所以需要一个"**等所有从核都完成**"的同步机制，这就是栅栏。

#### 7.5.2 用 Phase[] 实现栅栏

本项目的做法极其简洁——在共享内存里开一个数组 `Phase[0..8]`，每个元素代表一个轴（或一个子任务）的状态：

```c
// 状态枚举大概是这样
enum {
    Free = 0,      // 空闲（已完成或还没开始）
    Busy = 1,      // 正在处理
    Done = 2,      // 刚刚完成（可选中间状态）
};
```

**CORE0 在周期开始时**：

```c
// 把所有 Phase 都置为某个"待处理"状态，或者重置为 Free
MultiCoreShareData->Phase[0] = Free;
MultiCoreShareData->Phase[1] = Free;
// ...
MultiCoreShareData->Phase[8] = Free;
```

这段代码就来自 `IsrMotionControl()` 的 Default 状态分支。然后触发 IPC 中断唤醒从核。

**从核在中断服务函数里**：

```c
void CORE1_IPC_ISR() {
    MultiCoreShareData->Phase[axisNum] = Busy;      // 标记为忙
    // ... 跑算法 ...
    MultiCoreShareData->result[axisNum] = computed_value;
    WritebackCache(&MultiCoreShareData->result[axisNum], sizeof(...));
    MultiCoreShareData->Phase[axisNum] = Free;       // 标记为完成
    WritebackCache(&MultiCoreShareData->Phase[axisNum], sizeof(char));
}
```

**CORE0 在周期末尾检查**：

```c
if (CheckAllAxisFinish() == 0) {
    MoveFinishLock(0);   // 说明所有从核都完成了
}
```

其中 `CheckAllAxisFinish()` 大概是这样实现的：

```c
int CheckAllAxisFinish() {
    InvalidateCache(&MultiCoreShareData->Phase, sizeof(Phase));
    for (int i = 0; i < N; i++) {
        if (MultiCoreShareData->Phase[i] != Free) {
            return -1;   // 还有核没完成
        }
    }
    return 0;   // 全部完成
}
```

#### 7.5.3 为什么这个方案能工作

这个看起来很"土"的方案能工作，有三个关键前提：

1. **CORE0 和从核访问 `Phase[]` 的时机不重叠**：CORE0 先写全部 Free → 踢醒从核 → 从核更新自己的一项 → CORE0 最后读取。写和读在时间上是分开的。

2. **Cache 一致性被手动处理**：从核写完 Phase 之后 Writeback，CORE0 读 Phase 之前 Invalidate。

3. **`Phase[]` 每个元素是独立地址**：不同从核写不同的数组元素，不会互相踩。前提是每个元素最好占**一整条 Cache 行**（64 字节），避免"**伪共享**"（false sharing）——后面会讲这个坑。

#### 7.5.4 伪共享问题（False Sharing）

**什么是伪共享？** 假设 `Phase[0]` 和 `Phase[1]` 分别是两个 `char`，它们在内存里相邻，**在同一个 Cache 行里**。CORE1 写 `Phase[1]`，CORE2 写 `Phase[2]`。表面上看它们写的是不同地址不冲突，但实际上——**它们在同一个 Cache 行**，每次一个核写入都会把整个 Cache 行"拉"到自己的 Cache 里，另一个核的 Cache 副本就失效了，需要重新加载。这会严重影响性能。

更糟糕的是，如果两个核都在高速写各自的那个字节，Cache 行会不停地在两个核之间"**弹来弹去**"，出现所谓的 **Cache line ping-pong**（Cache 行乒乓）现象。

**解决方法**：让每个共享变量独占一条 Cache 行。可以用 `__attribute__((aligned(128)))` 对齐到 128 字节（C6678 的 L1D Cache 行大小），或者在结构体里加填充字节：

```c
typedef struct {
    volatile char state;
    char padding[127];   // 填充到 128 字节
} AlignedPhase;

AlignedPhase Phase[9] __attribute__((aligned(128)));
```

这样每个 Phase 独占一个 Cache 行，多个核并发写不同元素时不会互相影响。

### 7.6 一致性操作的性能代价

手动刷 Cache 不是免费的。每一次 Writeback 或 Invalidate 都要几十到几百个时钟周期。如果每个控制周期要刷好几块区域，开销会累加。

**优化思路**：

1. **只刷必要的范围**：如果只改了 4 个字节，就只刷这 4 个字节所在的 Cache 行，不要整个结构体一把梭。
2. **把"经常一起变"的数据放在一起**：这样一次刷新可以覆盖多项更改。
3. **把"只读**" **和"可写"数据分开**：只读数据比如常量参数，启动时刷一次就可以，运行时不用再刷。
4. **避免不必要的 Cache 污染**：如果某段内存只被一个核访问，别让它进其他核的 Cache——不过这个要通过内存映射属性配置实现，进阶话题。

### 7.7 第 7 章小结

1. **多核共享内存的核心难题是 Cache 一致性**：每个核有自己的 Cache，写入可能只在本地生效，其他核读到的是旧值
2. **C66x 没有硬件 Cache 一致性协议**，完全靠程序员手动处理
3. **两个基本操作**：
   - **Writeback**：写之后，把自己 Cache 里的脏数据推到主存
   - **Invalidate**：读之前，把自己 Cache 里的旧副本扔掉
4. **顺序关键**：写方是"**写 → Writeback → 通知**"，读方是"**Invalidate → 读**"
5. **Phase[] 栅栏同步**依赖时序隔离 + 手动 Cache 操作 + 独占 Cache 行
6. **当心伪共享**：相邻的共享变量可能共用一个 Cache 行，导致 Cache 行乒乓，破坏性能
7. **Cache 操作有开销**，要精确控制刷新范围

Cache 一致性是多核 DSP 编程里最容易踩的坑。学会它之后，你就掌握了写正确并发代码的半壁江山。

---

## 第 8 章 AMP 与 SMP 编程模型

### 8.1 两种完全不同的多核使用方式

到这里我们已经把**怎么在多核之间通信和同步**讲完了，现在拉高一层，看一下**整个项目是怎么组织多核代码的**。

多核的使用方式在业界主要有两大流派：**AMP** 和 **SMP**。本项目用的是 AMP，但理解两者的区别能让你更清楚"**为什么运动控制要选 AMP**"。

### 8.2 SMP：对称多处理

**SMP** 的全称是 **Symmetric Multi-Processing**，中文叫**对称多处理**。这是我们平时用的电脑、手机的工作方式。

**核心思想**：

- **所有核跑同一份操作系统**，一份可执行文件
- **操作系统动态调度线程到不同核上执行**
- 程序员写代码时通常不关心"这段代码会跑在哪个核上"
- 共享内存的访问由 OS 和硬件联合管理

**举例**：你在 Windows 上开一个 Chrome，Chrome 有几十个线程，Windows 的调度器决定"这个线程现在放 CORE0，那个线程放 CORE3"，运行过程中线程还可能在核之间迁移。你写代码时只管 `std::thread`，不管物理核号。

**优点**：
- 编程模型简单，和单核写法几乎一样
- 负载自动均衡（闲的核自动接活儿）
- OS 处理一切复杂度（同步、调度、迁移）

**缺点**：
- **时序不可预测**：OS 调度是动态的，你不知道线程什么时候跑、跑在哪儿
- **有调度开销**：线程切换、上下文保存需要额外的 CPU 周期
- **共享内存一致性由 OS 保证**，但一致性协议本身有性能开销
- **需要一个完整的 OS**（Linux、Windows、RTOS），内存开销大

**典型用途**：服务器、桌面、手机应用。**不适合硬实时控制**。

### 8.3 AMP：非对称多处理

**AMP** 的全称是 **Asymmetric Multi-Processing**，中文叫**非对称多处理**。

**核心思想**：

- **每个核跑自己独立的一份可执行文件**
- **每个核的任务是程序员在编译时就固定分配好的**
- 没有 OS 做动态调度（或者每个核跑一个极简的裸机循环/轻量 RTOS）
- 核间通信和同步由程序员**显式编程**

**举例**：本项目就是典型 AMP。CORE0 跑 `MCB_CORE0.out`（里面是主控逻辑），CORE1 跑 `MCB_CORE1.out`（里面是控制算法），CORE7 跑 `MCB_CORE7.out`。三个核**从启动到关机**都在执行各自那份代码，不会"调度"或"迁移"。

**优点**：
- **时序完全可预测**：每个核在干什么、多久一个周期，都是设计好的
- **没有调度开销**：核不会被 OS 打断
- **极低延迟**：中断直接进 ISR，不经过调度器
- **代码简单可控**：不需要 OS，每个核的代码可能就几千行
- **故障隔离**：一个核崩溃，其他核可以继续跑

**缺点**：
- **编程复杂度全压在程序员身上**：核间通信、同步、启动顺序、Cache 一致性都要手动管
- **负载不自动均衡**：如果 CORE1 的任务变重了，不会自动挪一部分给 CORE7
- **每个核有一份代码**，调试时要同时管多个工程
- **灵活性差**：改任务分配要重新编译多个工程

**典型用途**：嵌入式实时控制、通信基带、雷达、音频、**运动控制**。

### 8.4 两者的直观对比

| 维度 | SMP | AMP |
|------|-----|-----|
| 可执行文件数 | 1 份（所有核共享） | N 份（每核一份） |
| 操作系统 | 必需（Linux、RTOS） | 可选（裸机也行） |
| 任务分配 | 运行时动态 | 编译时静态 |
| 线程迁移 | 可以 | 不可以 |
| 时序确定性 | 低 | 高 |
| 调度开销 | 有 | 无 |
| 实时性 | 软实时 | 硬实时 |
| 编程复杂度 | 低 | 高 |
| 负载均衡 | 自动 | 手动 |
| 代表场景 | 服务器、桌面、手机 | 控制、通信、DSP |

### 8.5 为什么运动控制必须选 AMP

现在我们能清晰地回答"为什么运动控制要用 AMP"这个问题：

**1. 硬实时需求**

运动控制的核心需求是"**每 100 微秒一次的控制周期必须准时完成**"。SMP 的 OS 调度会带来微秒级的抖动，这在运动控制里是不可接受的——抖动就是电机的振动、位置精度的损失、甚至失控。AMP 没有调度器，时序由硬件定时器中断直接驱动，抖动可以降到纳秒级。

**2. 确定性优先于吞吐量**

SMP 追求"**平均性能最优**"——总吞吐量高就行，偶尔一次慢点没关系。运动控制追求的是"**最差情况下也能满足时限**"，只关心最长那次周期耗时有没有超标，不关心平均值。AMP 让你能精确控制每个周期的执行路径，确定性高得多。

**3. 算力需求不是"尽量快"而是"刚刚够"**

SMP 的设计哲学是"榨干硬件"——线程越多越好，核越忙越好。运动控制的算力需求是固定的（每周期 X 次乘法、Y 次访存），只需要"**够用**"。AMP 让你精确地给每个核分配"刚刚够"的任务量，多出来的算力留作裕度（比如应对偶发的重活），不需要为了调度器的复杂度付代价。

**4. 可验证和认证**

高端工业设备往往需要通过安全认证（比如 IEC 61508、ISO 13849）。认证机构要求代码路径**可追溯、可预测、可验证**。AMP 由于代码静态分配到核，非常容易做静态分析、时间分析（WCET，最坏情况执行时间分析）。SMP 由于有 OS 和动态调度，认证起来要难一个数量级。

**5. 故障隔离**

AMP 模式下，CORE1 挂了不会影响 CORE7——它们物理上是独立的，只在共享内存上有交集。这种隔离对高可靠系统至关重要。SMP 下一个核出问题可能拖累整个系统（OS 崩溃、共享资源死锁等）。

### 8.6 本项目的 AMP 具体实现方式

既然选了 AMP，就要看看本项目**具体是怎么实现的**。关键特征有几个：

#### 8.6.1 每个核独立的工程

项目里有 `MCB_CORE0/`、`MCB_CORE1/`、`MCB_CORE7/` 三个独立的 CCS（Code Composer Studio）工程目录。每个工程有：

- 自己的源代码（`.c`、`.h`）
- 自己的链接脚本（`C6678_unified.cmd`）
- 自己的 CCS 配置（`.cproject`、`.project`）
- 自己的编译输出（`.out` 可执行文件）

这些工程**分别编译**，产生三份独立的 `.out` 文件。

#### 8.6.2 每个核的代码侧重不同

**CORE0 工程包含**（从目录结构看）：

- LWIP 以太网协议栈（`ethernet/`）
- FPGA 通信（`fpga/`）
- SRIO（`srio/`）
- CamLink 相机接口（`camlink/`）
- RS485、CAN、RS422（`rs485/`、`can/`、`rs422/`）
- NAND、SPI Flash 存储（`nand/`、`norwrite/`）
- 传感器采集（EDMA 驱动）
- 轨迹规划、参考曲线（`control/refcurve.c`）
- 输入整形（`control/inputshaping.c`）
- 指令分发、系统诊断（`control/connector.c`、`control/diagnose.c`）

**CORE1 / CORE7 工程包含**（目录极其精简）：

- 基础驱动（EDMA3、GPIO、SPI）
- IPC 驱动
- Cache 驱动
- 控制算法（不含上面任何通信协议）

**这种不对称分工是 AMP 的典型特征**——CORE0 是"I/O + 主控核"，CORE1/7 是"纯计算核"。每个核的代码规模差异很大，CORE0 的代码量可能是 CORE1/7 的好几倍。这和 SMP 里"所有核跑同一份代码"完全相反。

#### 8.6.3 启动流程：谁先启动谁后启动

AMP 模式下，多个核的启动是有顺序的。C6678 的典型启动流程是：

1. **上电后 CORE0 自动启动**，从 SPI Flash / NAND 加载自己的 `.out` 到 L2
2. CORE0 完成系统初始化（时钟、DDR3、共享内存）
3. CORE0 把 CORE1 和 CORE7 的 `.out` 分别加载到它们的 L2
4. CORE0 通过 **Boot Configuration 寄存器**设置每个从核的**启动地址**（entry point）
5. CORE0 **释放从核的复位信号**，从核从指定地址开始执行
6. 从核初始化自己的驱动（EDMA、IPC、Cache）
7. 从核进入等待状态，等 CORE0 的 IPC 中断

这个"CORE0 当老大启动从核"的流程也叫 **Master-Slave Boot**。CORE0 在硬件上就是默认的启动核（芯片的复位向量跳到 CORE0 的启动代码）。

#### 8.6.4 运行时的"主从同步并行"模式

运行起来之后，本项目的运行模式已经在前面讨论过了，这里完整总结一遍：

```
Timer 中断
    ↓
CORE0: IsrMotionControl() 启动
    ↓
CORE0: 读传感器 → 预处理 → 写共享内存
    ↓
CORE0: WritebackCache
    ↓
CORE0: KickUnlock + CSL_IPC_genGEMInterrupt × N + KickLock
    ↓                                                    ↓
    ↓                                       CORE1/7: IPC ISR 启动
    ↓                                                    ↓
    ↓                                       CORE1/7: InvalidateCache
    ↓                                                    ↓
    ↓                                       CORE1/7: 读共享内存
    ↓                                                    ↓
    ↓                                       CORE1/7: 跑控制算法
    ↓                                                    ↓
    ↓                                       CORE1/7: 写结果到共享内存
    ↓                                                    ↓
    ↓                                       CORE1/7: WritebackCache
    ↓                                                    ↓
    ↓                                       CORE1/7: Phase[x] = Free
    ↓                                                    ↓
    ↓                                       CORE1/7: ISR 返回
    ↓
CORE0: 轮询 CheckAllAxisFinish()
    ↓ (等所有 Phase 都 Free)
CORE0: InvalidateCache
    ↓
CORE0: 读结果 → 汇总 → VxDooBellSend() 发给 FPGA
    ↓
CORE0: IsrMotionControl() 返回，等下一次 Timer 中断
```

这是一个**主从同步并行**（Master-Slave Synchronous Parallel）的经典模式。关键特征：

1. **主核驱动**：CORE0 是总指挥，决定"现在开始算"和"所有人都完成了没"
2. **从核被动**：CORE1/7 是干活工，平时休眠，被踢醒才工作
3. **栅栏同步**：每个周期结束前所有核必须到达同步点（Phase 全 Free）
4. **周期性心跳**：整个循环由 Timer 中断以固定节奏驱动

### 8.7 AMP 编程的几个常见坑

AMP 编程复杂度高，新手容易踩坑。这里列几个：

#### 坑 1：忘记 Cache 一致性操作

最常见的坑。症状：读到旧数据、程序行为诡异、偶尔正常偶尔错。解决：严格按"**写 → Writeback → 通知**"和"**Invalidate → 读**"的模板写。

#### 坑 2：共享变量没对齐

Cache 行是 64 或 128 字节对齐的。共享变量如果跨两个 Cache 行，一次刷新可能刷不干净。解决：`__attribute__((aligned(128)))` 对齐。

#### 坑 3：多个核同时写同一个地址

即便时序上看起来不重叠，只要没有明确的同步约束，就可能出问题。解决：要么彻底隔离（不同核写不同地址），要么用原子操作或锁。

#### 坑 4：启动顺序错了

从核启动前，CORE0 还没把 `MultiCoreShareData` 初始化。从核一启动就去访问，读到垃圾值。解决：CORE0 初始化完共享数据再启动从核，或者让从核启动后先等一个"初始化完成"的 IPC 信号。

#### 坑 5：中断服务函数里做太多事

IPC ISR 里跑一段长时间的算法会阻塞其他中断。解决：ISR 只做最必要的事（清标志、设任务位），真正的计算放到主循环或更低优先级的中断里。

#### 坑 6：各核的链接脚本地址冲突

如果两个核的 `.out` 都试图放到同一个 DDR3 地址，启动时会互相覆盖。解决：AMP 下每个核通常只用自己的 L2 SRAM 不碰 DDR3，或者精确地划分 DDR3 给每个核专用区域。

这些坑在第 12 章"常见坑与调试技巧"里会进一步展开。

### 8.8 第 8 章小结

1. **SMP 是"一份 OS 调度多个核"**，编程简单但时序不确定
2. **AMP 是"每个核独立跑自己的代码"**，编程复杂但时序可控
3. **运动控制必须选 AMP**，因为硬实时、确定性、可认证、故障隔离都需要 AMP 的特性
4. **本项目的 AMP 实现**：CORE0/CORE1/CORE7 三个独立工程 + 独立 .out + 各自职责
5. **启动流程是"Master-Slave Boot"**：CORE0 先启动，然后加载并释放从核
6. **运行模式是"主从同步并行"**：Timer → CORE0 采集 → 踢从核 → 并行算 → 栅栏同步 → CORE0 输出 → 等下一周期
7. **AMP 的坑集中在 Cache 一致性、共享变量对齐、启动顺序、ISR 里做太多事等地方**

---

**第三部分结束。**

到这里你已经掌握了多核 DSP 编程最难的核心知识：核间通信、Cache 一致性、AMP 编程模型。这三个概念合起来是"**怎么让多个核协作正确**"的完整答案。后面两部分都是在此基础上展开。

**下一部分（第四部分 · 开发与优化篇，第 9-11 章）**会讲：

- 第 9 章 开发工具链（CCS、SYS/BIOS、编译器、多核调试、启动流程）
- 第 10 章 性能优化要点（双缓冲、Cache 优化、内在函数、软件流水）
- 第 11 章 **运动控制应用案例（全文高潮）**——把前面所有知识串起来，完整讲一个控制周期从 Timer 触发到 FPGA 输出的全过程，配上真实代码片段

请你阅读后告诉我：

1. 第 6 章的核间通信讲得够不够清楚？Kicker 机制和 IPC 寄存器的讲法有没有问题？
2. 第 7 章 Cache 一致性是最难的部分，深度是否合适？有没有讲得太抽象或太啰嗦？
3. 第 8 章 AMP vs SMP 的对比是否到位？本项目的"主从同步并行"模式描述得对不对？
4. 有没有哪个概念讲得太浅或太深需要调整？

确认 OK 后说"**开始生成第四部分**"，我会继续写第 9-11 章。
