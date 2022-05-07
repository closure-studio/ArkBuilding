# ArkBuilding 明日方舟基建排班计算器
交流群：[603525846](https://jq.qq.com/?_wv=1027&k=Lc4UEoAe)  欢迎提问/建议/~~涩涩~~
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

计算前需要加载 `building_data.json`,`char_meta_table.json`,`character_table_.json`文件或其中的内容。

#### 单个房间需要提供的参数：
1. 任意标识符，仅用于在输出中区分房间
2. 房间类型
3. 房间槽位数
4. 订单/产品类型
5. （可选）房间属性（见[房间属性说明](#房间属性说明)）

#### 单个干员需要提供的参数：
干员可选择提供以下若干种参数的有效组合（后文有提到）来解析其拥有的技能：

| 参数类型                               | 备注                                                                                                             |
|------------------------------------|----------------------------------------------------------------------------------------------------------------|
| `干员名称/ID/任意标识符`                    | -                                                                                                              |
| `精英阶段 + 等级`                        | -                                                                                                              |
| `若干个技能的 {名称/ID} 列表或 { 技能图标ID } 列表` | 两种列表选其一，如`["trade_ord_spd&cost_P[000]", "默契"]`（名称/ID列表）或`["bskill_tra_texas1", "bskill_tra_texas2"]`（技能图标ID列表） |

以及用于代入计算的参数：
* `心情 (0, 24]`

其中用于解析技能的参数的有效组合如下
1. 指定`干员名称/ID + 精英阶段 + 等级`，将从游戏数据中解析出干员拥有的技能。若干员 Buff 未建模则不会参与计算。`干员ID/名称`无效时，不会参与计算。
2. 指定`干员名称/ID + 若干个技能`，将尝试查询出`精英阶段 + 等级`，如找到则相当于选项1，未找到相当于选项3。
3. 只指定`任意标识符 + 若干个技能`。将尝试查询出干员及等级，如找到则相当于选项1，未找到则也会使用指定的技能直接代入模型计算。某个`技能ID/名称`无效时，不会参与计算。


#### 干员参数备注：
* 干员名称/ID（如“德克萨斯” / `char_102_texas`）现在只用于模型中羁绊/联动体系的处理和异格干员判断。非必须选项。
* 查询操作相当于用`干员名称/ID + 若干技能`或`若干技能`查询出`干员`及其`精英阶段 + 等级`。

### 输出数据
房间的集合（不包括没有排班结果的房间），每个元素包含了房间中选中的干员、预计持续时长（单位：秒）（从开始到第一位干员心情耗尽）
和预计产能（可看作 `平均效率倍数 * 持续时长（秒）`）

### 使用相关
* 提供参数的细节可见 API 头文件及 API 用例（如[JSON 用例](#JSON-输入（例）)）
* 目前的参数组合和提供的集成方式可能还存在局限性，如果你在如何集成方面有问题或有好的见解，欢迎提出 Issue/PR 来改进。

## 模型
[计算流程图](img/arkbuilding.png)

// TODO: 模型

## 目前局限/问题
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
// TODO: CLI

## API 使用
[API 示例](https://github.com/closure-studio/ArkBuilding/tree/main/src/examples/src)

API 接口使用方法见 API 头文件。

### API JSON 格式数据使用说明
JSON 中的所有数据约定[同上](#使用)
#### JSON 输入（例）
```json5
{
  // 模型时间限制，代表计算持续的时间。
  "modelTimeLimit": 57600,
  // 求解超时。
  "solveTimeLimit": 60,
  "chars": {
    // 此处为参数组合选项1：干员+精英化+等级
    "char_1": {
      // 或者 "id": "char_102_texas"
      "name": "德克萨斯",
      "phase": 2,
      "level": 1,
      "morale": 24,
    },
    // 选项2：干员+若干技能
    "Char2": {
      "name": "拉普兰德",
      "skills": [
        "醉翁之意·β"
      ],
      "morale": 24,
    },
    // 选项3：任意标识符+若干技能，此处可以查询到干员。
    "jueSe3": {
      "skills": [
        "物流专家"
      ],
      "morale": 24,
    },
    // 此处无法查询到干员，但是技能仍会代入模型
    "anonymous": {
      "skills": [
        "标准化·β"
      ],
      "morale": 24,
    },
    // 干员 + 图标列表
    "刻俄柏": {
      name: "刻俄柏",
      "skills": [
        "bskill_man_limit&cost1",
        "bskill_man_spd_add1"
      ],
      "morale": 24,
    },
    // ... 以下省略其他干员，完整例可见 src/examples/src/json_input.cpp
  },
  "rooms": {
    "room_1": {
      // 房间类型 见下述表格
      "type": "TRADING",
      // 产品类型，见下述表格
      "prodType": "GOLD",
      // 槽位数
      "slots": 3,
      "attributes": {
        // 已有订单/产品数
        "prodCount": 3,
      }
    },
    "room_2": {
      "type": "TRADING",
      "prodType": "GOLD",
      "slots": 3
    },
    "room_3": {
      "type": "MANUFACTURE",
      "prodType": "GOLD",
      "slots": 3,
      "attributes": {
      }
    },
    "room_4": {
      "type": "MANUFACTURE",
      "prodType": "GOLD",
      "slots": 3,
      "attributes": {
      }
    },
    "room_5": {
      "type": "MANUFACTURE",
      "prodType": "RECORD",
      "slots": 3,
      "attributes": {
      }
    },
    "room_6": {
      "type": "MANUFACTURE",
      "prodType": "RECORD",
      "slots": 3,
      "attributes": {
      }
    }
  }
}
```

##### 参数说明
| 参数                             | 参数类型       | 缺省值     | 参数说明                          |
|--------------------------------|------------|---------|-------------------------------|
| `modelTimeLimit`               | `double`   | `57600` | 模型时间限制，代表计算持续的时间。             |
| `solveTimeLimit`               | `double`   | `60`    | Cbc 求解器的超时。                   |
| `chars`                        | `object`   | -       | 键供在输出中区分使用。               |
| `chars[identifier].name`       | `string`   | -       | 干员名称                          |
| `chars[identifier].id`         | `string`   | -       | 干员ID                          |
| `chars[identifier].phase`      | `int`      | -       | `{0, 1, 2}` 干员精英阶段            |
| `chars[identifier].level`      | `int`      | -       | `[0, 90]` 干员等级                |
| `chars[identifier].morale`     | `double`   | `24`    | `(0, 24]` 干员心情值               |
| `chars[identifier].skills`     | `string[]` | -       | 干员技能名称/ID列表或图标ID列表            |
| `rooms`                        | `object`   | -       | 键供在输出中区分使用。                   |
| `rooms[identifier].type`       | `string`   | -       | 房间类型，见下述表格                    |
| `rooms[identifier].prodType`   | `string`   | -       | 产品类型，见下述表格。                   |
| `rooms[identifier].slots`      | `int`      | -       | 房间槽位数                         |
| `rooms[identifier].attributes` | `object`   | -       | 房间属性，见下述表格                    |

##### 房间类型说明
| 房间类型 | 名称            | 产品类型                                        |
|------|---------------|---------------------------------------------|
| 制造站  | `MANUFACTURE` | `{GOLD, RECORD, SHARD, CHIP}` 赤金，录像，源石碎片，芯片 |
| 贸易站  | `TRADING`     | `{GOLD, SHARD}` 赤金，合成玉                      |

##### 房间属性说明
| 房间属性     | 名称             | 类型       | 缺省值              | 说明                                   |
|----------|----------------|----------|------------------|--------------------------------------|
| 已有产品数量   | `prodCount`    | `int`    | `0`              | 房间内已有的产品数量。现在用于孑技能的计算。（计划用于计算产品满仓情况） |
| 基础产品容量   | `baseProdCap`  | `int`    | `10`             | 房间容纳的产品数。现在用于孑技能的计算。（计划用于计算产品满仓情况）   |
| 基础心情消耗倍率 | `baseCharCost` | `double` | `1`              | 全局的心情消耗倍率                            |
| 基础产能倍率   | `baseProdEff`  | `double` | `1 + 0.01 * 槽位数` | 房间基础产能倍率                             |

#### JSON 输出（例）
```json5
{
  "errors": {
    "chars": {},
    "errors": [],
    "rooms": {}
  },
  "rooms": {
    "room_1": {
      "chars": [
        "Char2",
        "char_1",
        "孑"
      ],
      "duration": 57600.0,
      "score": 127296.0
    },
    "room_2": {
      "chars": [
        "jueSe3",
        "空弦",
        "雪雉"
      ],
      "duration": 57600.0,
      "score": 97920.000000000015
    },
    "room_3": {
      "chars": [
        "anonymous",
        "刻俄柏",
        "豆苗"
      ],
      "duration": 57600.0,
      "score": 80640.0
    },
    "room_6": {
      "chars": [
        "泡泡",
        "火神",
        "石棉"
      ],
      "duration": 57600.0,
      "score": 135359.99999999997
    }
  }
}
```

##### 参数说明
| 参数                           | 参数类型       | 参数说明                                                    |
|------------------------------|------------|---------------------------------------------------------|
| `rooms`                      | `object`   | 每个房间的排班结果，不包括没有排班的房间。                                   |
| `rooms[identifier].chars`    | `string[]` | 选中的干员。                                                  |
| `rooms[identifier].duration` | `double`   | 从排班开始到出现第一位精力涣散的干员的时长。小于或等于 `modelTimeLimit`。           |
| `rooms[identifier].score`    | `double`   | 房间的总产能。从 `duration` 到 `modelTimeLimit` 之间会以全员精力涣散来计算产能。 |
| `errors`                     | `object`   | 运行中输出的错误信息。                                             |
| `errors.chars[identifier]`   | `string`   | 错误信息，当干员无法被加入模型时发生。                                     |
| `errors.rooms[identifier]`   | `string`   | 错误信息，当房间无法被加入模型时发生。                                     |
| `errors.errors`              | `string[]` | 全局产生的错误信息。每个项目一行。                                       |


## 构建源码
项目 C++ 标准为 C++ 17，目前支持使用 CMake 配置。

### 目前测试过的环境：
* Windows GCC 11.2.0 x86_64-w64-mingw32
* Linux GCC 10.2.1, x86_64 / ARM64
* Windows MSVC 17.1.6 x86_64

如果你遇到了构建/集成的问题，欢迎提出。

1. 环境
   * GCC 9.0+ / MSVC 14.1+
   * CMake

2. 初始化 Submodule
   ```console
   ~/ArkBuilding# git submodule update --init --recursive
   ```

4. CMake
    ```console
    ~/ArkBuilding# mkdir build
    ~/ArkBuilding# cd build/
    ~/ArkBuilding/build# cmake ..
    ```
   
5. 编译 CMake 中的某个 Target
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
* [boost-ext/di](https://github.com/boost-ext/di)

## License
MIT
