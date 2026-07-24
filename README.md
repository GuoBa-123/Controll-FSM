# FSM 有限状态机框架

## 快速开始

```cpp
#include "fsm.hpp"

// 1. 创建状态机实例
FSM motor;
FSM led;    

// 2. 声明状态事件函数和转移条件
void work() { /* 电机工作中 */ }
void stop() { /* 电机停止 */ }
bool check() { return /* 某个条件 */; }

// 3. 注册状态
motor.stateCreat("START", work);
motor.stateCreat("STOP",  stop);

// 4. 注册转移规则
motor.stateSetTrans("START", "STOP", check); 
 // START → STOP，当 check() 为 true 时触发

// 5. 启动
motor.start("START");

// 6. 主循环中每帧调用
while (true) {
    motor.stateUpdate();
}
```

## API

### 状态管理

| 方法 | 说明 |
| --- | --- |
| `stateCreat(name, func)` | 注册状态，`func` 在该状态期间每帧执行（可为 `nullptr`） |
| `stateDelete(name)` | 删除状态，注意需自行清理转移规则 |

### 转移规则

| 方法 | 说明 |
| --- | --- |
| `stateSetTrans(from, to, reason)` | `from → to`，`reason` 为 `true` 时触发，触发后自动复位 |

### 运行

| 方法 | 说明 |
| --- | --- |
| `start(name)` | 设置初始状态，必须早于 `stateUpdate` 调用 |
| `stateUpdate()` | 每帧驱动：检查转移 → 切换状态 → 执行事件回调 |

## 限制

- 最大状态数：10（修改 `STATECOUNT` 宏）
- 最大转移数：`STATECOUNT × (STATECOUNT - 1)`  

## And more…

- 由于stateCreat函数填入的是字符串变量，在后续应用该状态名字时不会有拼写检查，**切勿拼写错误**^^。
- 本来想用X-Marco改进一下，但是发现那样好像比现在还要麻烦，遂不了了之。