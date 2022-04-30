//
// Created by Nonary on 2022/4/24.
//

#include "json_input.h"
const char *GetTestJsonInput()
{
    return
        u8R"(
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
    "孑": {
      "name": "孑",
      "skills": [
        "摊贩经济"
      ],
      "morale": 24
    },
    "刻俄柏": {
      "name": "刻俄柏",
      "skills": [
        "bskill_man_limit&cost1",
        "bskill_man_spd_add1"
      ],
      "morale": 24
    },
    "雪雉": {
      "name": "雪雉",
      "skills": [
        "天道酬勤·β"
      ],
      "morale": 24
    },
    "空弦": {
      "name": "空弦",
      "skills": [
        "虔诚筹款·β"
      ],
      "morale": 24
    },
    "石棉": {
      "name": "石棉",
      "skills": [
        "特立独行",
        "探险者"
      ],
      "morale": 24
    },
    "泡泡": {
      "name": "泡泡",
      "skills": [
        "囤积者",
        "大就是好！"
      ],
      "morale": 24
    },
    "豆苗": {
      "name": "豆苗",
      "skills": [
        "磐蟹·豆豆",
        "磐蟹·阿盘"
      ],
      "morale": 24
    },
    "火神": {
      "name": "火神",
      "skills": [
        "工匠精神·β",
      ],
      "morale": 24
    },
    "清流": {
      "name": "清流",
      "skills": [
        "清洁能源",
        "再生能源"
      ],
      "morale": 24
    },
    "杰西卡": {
      "name": "杰西卡",
      "skills": [
        "标准化·β"
      ],
      "morale": 24
    }
  },
  "rooms": {
    "room_1": {
      "type": "TRADING",
      "prodType": "GOLD",
      "slots": 3,
      "attributes": {
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
)";
}
