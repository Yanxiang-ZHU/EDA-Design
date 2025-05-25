# 混合仿真器：EDA课程期末项目

> 姓名：祝彦翔
>
> 学号：22307130073

本项目是复旦大学微电子学院《电子设计自动化（EDA）》课程的期末项目，旨在实现模拟信号与数字信号混合系统的并行加速仿真。


---

## 项目简介

本项目实现了一个支持**可变线程数的多线程并行混合仿真器**，在**尽量减少回退的前提下**提升仿真速度。其设计核心是模拟A事件（Analog）与D事件（Digital）的异步触发与同步推进。

主要实现内容包括：

* 实现基于第二事件同步机制（Second-Event Synchronization, SES）框架的混合仿真；
* 提供灵活的线程调度策略；
* 提高仿真精度与并行效率的平衡性；
* 在`MMSimulator.c`中加入关键停顿控制逻辑，避免事件错乱。

---

## 文件结构

```bash
PJ2/
├── config.c / config.h          # 配置文件与文法分析模块
├── simulator.c / simulator.h    # A/D事件生成模块
├── MMSimulator.c / MMSimulator.h# 仿真主控制模块（重点）
├── whitebox/                    # 6个测试用例输入配置
├── backup_BASIC_METHOD1_LockStep/
├── backup_BASIC_METHOD2_LeapFrogSES/
├── backup_ADVANCED_METHOD3/
```

### 模块说明

* `MMSimulator.c`：主控制逻辑，负责多线程调度、初始等待D事件、A/D事件并发执行、仿真合并及回退控制。

  * 关键停顿逻辑位置：

    * 第50行：`generate_A_parallel` 中
    * 第64行：`generate_A` 中
    * 第78行：`generate_D` 中
  * 每次`usleep`后会调用 `generate_next_A` 和 `generate_next_D` 函数，共3次。
* `config.c/h`：完成配置文件`con.txt`的读取与参数解析。
* `simulator.c/h`：负责事件的产生与调度逻辑，结构沿用期中作业基础框架。

---

### 模块说明
我在MMSimulator.c里的停顿逻辑位于第50，64，78行，分别位于generate_A_parallel, generate_A 和generate_D三个函数内，只有在停顿过后才会调用generate_next_A及generate_next_D两个函数（一共也是只在usleep后出现了三次）。

```c
    usleep(tA * 1000);
    AEvent a_parallel = generate_next_A(a, a.T, Vth);
```

```c
    usleep(args->tD * 1000);
    DEvent d_parallel = generate_next_D(d, d.T);
```


## 编译与运行

1. 使用`gcc`进行编译（需支持pthread）：

```bash
gcc -O2 config.c simulator.c MMSimulator.c -o MMSimulator -lpthread
```

2. 运行程序：

```bash
./MMSimulator
```

* 输入文件为当前目录下的`con.txt`
* 输出文件为当前目录下的`sim_res.txt`

---

## 测试用例

项目包含了6个测试实例，位于`whitebox/`文件夹中，每个子文件夹均包含可直接运行的`con.txt`文件，可替换主目录下文件进行测试。

---

## 历史版本备份

为记录仿真策略的演进过程，保留了三种不同方法版本：

* `backup_BASIC_METHOD1_LockStep`：最基础的锁步法实现；
* `backup_BASIC_METHOD2_LeapFrogSES`：初步实现蛙跳类同步机制；
* `backup_ADVANCED_METHOD3`：最终的并行混合仿真加速方案。

---

## 开发环境

* 系统：Linux
* 编译器：gcc 7.0+
* 标准库：pthread, <stdio.h>, <stdlib.h>, <time.h>, <sys/time.h>, <unistd.h>, <math.h>

---
