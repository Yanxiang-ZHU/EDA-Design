# SPICE网表生成器

## 介绍
该程序用于生成随机电路网表，支持电压源、电流源、电阻、电容、电感等元件，并可根据配置文件 `netlistConfig` 控制生成规则。

---

## 编译与运行

### 1. 编译
使用 `gcc` 进行编译：
```sh
gcc -o generation generation.cpp
```
编译结果generation即会生成于本文件夹下。

### 2. 运行
执行以下命令：
```sh
./generation
```
运行编译程序之后，在文件夹目录下会发现有网表文件 `netlistDump.sp`生成，即运行成功！

---

## 依赖
- C 标准库（`stdio.h`、`stdlib.h`、`string.h`、`time.h`）

---

## 配置文件 `netlistConfig`
程序会读取 `netlistConfig` 文件中的配置，格式如项目说明中所述：
```txt
nVoltS     int #电压源个数
nCurrS     int #电流源个数
nR         int #电阻个数
nC         int #电容个数
nL         int #电感个数
nShort     int #短路节点对数
nLoop      int #最大环路数
bOpen      bool # true/1, 有开路，false/0，无开路
```
其中，本项目中我们规定：
- 电路开路与短路均是针对电源而言；
- 环路的个数只统计没有内环的小环；

---

## 生成的网表文件 `netlistDump.sp`
在代码中我们规定了网表文件的输出格式：
```
<元件类型> <名称> <节点1> <节点2> <数值>
```
---
我们自己创建了netlistConfig文件。对程序编译、运行后，在当前目录下生成了 `netlistDump.sp` ，符合项目的要求。


