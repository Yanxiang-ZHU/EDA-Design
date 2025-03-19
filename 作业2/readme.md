# SPICE网表解析程序

## 编译、运行方法
本项目由 C 语言编写，运行环境为 Linux 系统。实验中采取的编译及运行方式为：

```sh
gcc -o NetlistParser NetlistParser.cpp
./NetlistParser <-c / -v / -cv / -vc> <spicefile>
```

## 依赖
本项目并无使用特殊库，仅需要 C 语言的基本库：
- `stdlib.h`
- `stdio.h`
- `string.h`
- `time.h`
- `stdbool.h`

## 代码说明

### 功能
本代码的功能是，分析给定的大规模SPICE网表文件，统计电路器件数目、短路节点数、开路节点数、悬空器件数和独立电路数目。并将结果输出在`NetlistReport.csv`文件中。
软件配置有运行日志文件`RunSummary.txt`，记录软件详细信息及报错信息等。
本软件有两个扩展功能，**-c**会在日志文件中详细列出悬空器件名称、短路和开路的具体节点名称；**-v**会另生成剔除悬空器件后的网表文件`TrimedNetlist.sp`。

### 文件说明
| 文件名 | 说明 |
|--------|------|
| `readme.md` | 解释性文件 |
| `NetlistParser.cpp` | SPICE网表解析代码 |
| `NetlistParser` | `NetlistParser.cpp` 的编译文件 |
| `白盒测试` | 测试文件（作业2文件夹下的各输出由白盒测试/test文件分析得到） |
| `NetlistReport.csv` | 输出文件：分析结果信息 |
| `RunSummary.txt` | 输出文件：日志文件 |
| `TrimedNetlist.sp` | 输出文件（选择性）：剔除悬空器件网表 |