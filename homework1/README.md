# SPICE网表生成器

## 介绍
该程序用于生成随机电路网表，支持电压源、电流源、电阻、电容、电感等元件，并可根据配置文件 `netlistConfig` 控制生成规则。

---

## 编译与运行

### 1. 编译
使用 `gcc` 进行编译：
```sh
gcc -o netlist_generator netlist_generator.c
```

### 2. 运行
执行以下命令：
```sh
./netlist_generator
```

---

## 依赖
- C 标准库（`stdio.h`、`stdlib.h`、`string.h`、`time.h`）
- 无需额外的第三方库

---

## 配置文件 `netlistConfig`
程序会读取 `netlistConfig` 文件中的配置，格式如下例：
```txt
nVoltS 2
nCurrS 1
nR 5
nC 3
nL 2
nShort 4
nLoop 1
bOpen 1
```
其中：
- `nVoltS`：电压源数量
- `nCurrS`：电流源数量
- `nR`：电阻数量
- `nC`：电容数量
- `nL`：电感数量
- `nShort`：短接的节点对数
- `nLoop`：是否形成回路（当前代码未使用）
- `bOpen`：是否生成额外的开路电阻

---

## 生成的网表文件 `netlistDump.sp`
在代码中我们规定了网表文件的输出格式：
```
<元件类型> <名称> <节点1> <节点2> <数值>
```
---
我们自己创建了netlistConfig文件。对程序编译、运行后，在当前目录下生成了 `netlistDump.sp` ，符合项目的要求。


