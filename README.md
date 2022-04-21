# ArkBuilding 明日方舟基建排班计算器
## 简介
（项目内部缩写 ALBC: Albc-Like Building Calculator）

该项目是为明日方舟自动化脚本开发的基建计算库。

支持基于 Buff 和干员的建模进行单房间中一定时间内产能的模拟，并枚举出多个房间中模型的所有组合。
将组合加入到整数规划模型后可求出多房间产能总和最大的方案。

对于不便建模的情况（如多房间联动等），同样可创建特化组合模板将组合加入到整数规划模型中参与求解。

## 特点
* 高性能：使用 Cbc 求解器
* 高产能：大多数情况比贪心算法更优

## 使用
***目前正在开发的，相对比较通用的集成方式（命令行程序）的输入数据方式和使用方式仍在积极征求意见，欢迎提出 Issue/PR！***

项目目前提供了支持 CMake 构建的 C++ 源码、 命令行程序（WIP）和可通过 C/C++ API 调用的库供集成。
### 输入数据
#### 静态数据
静态数据中用到的 JSON 文件来源详见[此处](https://github.com/Kengxxiao/ArknightsGameData/tree/master/zh_CN/gamedata/excel)

计算前需要加载 `building_data.json` 文件，如果需要基于干员名称（不包括ID）来提供干员参数，
则还需要加载 `character_table.json` 文件（用于获取干员中文名）。

#### 单个房间需要提供的参数：
1. 任意标识符，仅用于在输出中区分房间
2. 房间类型
3. 房间槽位数
4. 订单/产品类型
5. （可选）房间已有产品数量

#### 单个干员需要提供的参数：
干员可选择提供以下三种参数来解析其拥有的技能：
* `干员名称/ID/任意标识符`
* `精英阶段 + 等级`
* `若干个技能的名称/ID`

以及用于代入计算的参数：
* 心情(> 0, <= 24)

其中用于解析技能的参数的有效组合如下
1. 指定`干员名称/ID + 精英阶段 + 等级`，将从游戏数据中解析出干员拥有的技能。若干员 Buff 未建模则不会参与计算。`干员ID/名称`无效时，不会参与计算。
2. 指定`干员名称/ID + 若干个技能`，将尝试查询出`精英阶段 + 等级`，如找到则相当于选项1，未找到相当于选项3。
3. 只指定`任意标识符 + 若干个技能`。将尝试查询出干员及等级，如找到则相当于选项1，未找到则也会使用指定的技能直接代入模型计算。某个`技能ID/名称`无效时，不会参与计算。


#### 干员参数备注：
* 干员名称/ID（如“德克萨斯” / `char_102_texas`）现在只用于模型中羁绊/联动体系的处理。非必须选项。
* 查询操作相当于用`干员名称/ID + 若干技能`或`若干技能`查询出`干员`及其`精英阶段 + 等级`。

### 输出数据
房间的集合（不包括没有排班结果的房间），每个元素包含了房间中选中的干员、预计持续时长（单位：秒）（从开始到第一位干员心情耗尽）
和预计产能（可看作 `平均效率倍数 * 持续时长（秒）`）

### 使用相关
* 提供参数的细节可见 API 头文件及 API 用例。
* 目前的参数组合和提供的集成方式可能还存在局限性，如果你在如何集成方面有问题或有好的见解，欢迎提出 Issue/PR 来改进。

## 模型
[计算流程图](img/arkbuilding.png)

// TODO: 模型

## 目前局限
* 目前的问题模型里只考虑最大化一定时间内产能，不考虑其他因素。
* 目前只进行了初步的建模 + 求解的实现。
* 欢迎提出 Issue 或 Pull Request 来改进。

## 二进制文件
见 [GitHub Release](https://github.com/closure-studio/ArkBuilding/releases)

| 系统  | 命令行程序 | 动态库 | 静态库 | 备注  |
|-----|-------|-----|-----|-----|
// TODO: Binaries

| 头文件名    | 描述     |
|---------|--------|
| albc.h  | C++ 接口 |
| calbc.h | C 接口   |

## 命令行程序使用方法
```console
$ albccli -h
Usage:
  albccli [options]
Available options:
  -p, --playerdata       Path to player data file. 
                           PATH                            : string
  -g, --gamedata         Path to Arknights building data file.
                           PATH                            : string
  -l, --log-level        Log level.
                           Default is WARN.
                           <DEBUG|INFO|WARN|ERROR>         : string
  -t, --model-max-time   Model time limit in seconds.
                           Default is 57600 (16 hours).
                           TIME                            : double
  -T, --sovle-max-time   Problem solving timeout in seconds.
                           Default is 20.
                           TIME                            : double
  -m, --test-mode        Test mode. Leave empty for normal mode.
                           <ONCE|SEQUENTIAL|PARALLEL>      : string
  -P, --test-param       Test param.
                           NUM_CONCURRENCY|NUM_ITERATIONS  : int
  -L, --lp-file          Generate a lp-format file describing the problem.
                              : FLAG
  -S, --solution-detail  Generate a text file describing all feasible solution.
                              : FLAG
  -a, --all-ops          Show all operators info.
                              : FLAG
  -h, --help             Produce help message.
                              : FLAG
```

除 ONCE 外， `-m {测试模式}` 应与 `-P {测试参数}` 同时使用。

| 测试模式       | 测试参数      |
|------------|-----------|
| ONCE       | 不要求测试参数   |
| SEQUENTIAL | 依次运行的测试次数 |
| PARALLEL   | 并行运行的测试次数 |

例（测试数据位于`test/`目录中）：
```console
$ albccli -p test/player_data.json -g test/building_data.json -c test/character_table.json -m ONCE -l INFO -L -S
```
该命令将运行一次测试，并将包含描述整数规划问题的 .lp 格式文件、全部组合的详细信息输出到工作目录。

// TODO: More info

## API 使用
[API 示例](https://github.com/closure-studio/ArkBuilding/tree/main/src/examples/src)

API 接口使用方法见 API 头文件。

## 构建源码（目前在 CMake + Windows MSYS2 MinGW / Linux GCC, x64 / ARM64 上经过测试） 
1. 环境
   * GCC 9.0+
   * CMake （未确定版本，如出现不兼容请尽量升级到最新版）

2. CMake
    ```console
    ~/ArkBuilding# mkdir build
    ~/ArkBuilding# cd build/
    ~/ArkBuilding/build# cmake ..
    ```
   
3. 编译 CMake 中的某个 Target
   * "albccli" : 命令行程序
   * "albc" : 动态库
   * "albc_static" : 静态库
   * "albcexample" : API示例，见[此处](#API-使用)

## 项目中使用的第三方库及资源
* [Kengxxiao/ArknightsGameData](https://github.com/Kengxxiao/ArknightsGameData) （干员数据、基建数据）
* [ampl/coin](https://github.com/ampl/coin) （Cbc 的 CMake 构建支持）
* [open-source-parsers/jsoncpp](https://github.com/open-source-parsers/jsoncpp)
* [cameron314/concurrentqueue](https://github.com/cameron314/concurrentqueue)
* [Fytch/ProgramOptions.hxx](https://github.com/Fytch/ProgramOptions.hxx)
* [bombela/backward-cpp](https://github.com/bombela/backward-cpp)

## License
// TODO: license
