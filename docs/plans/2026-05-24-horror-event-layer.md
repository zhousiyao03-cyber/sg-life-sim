# Plan 15: Horror Event Layer（恐怖事件层 / 农历七月）实施计划

> **状态：** ✅ 完成（2026-05-24）
> **方向：** docs/decisions/2026-05-24-first-person-horror-pivot.md（第一人称 + lo-fi 恐怖，恐怖按桥段插入）
> **关联：** 复用 Plan 9 随机事件模式 + Plan 5/6/11 对话系统 + 现有时间日历

**Goal:** 转恐怖方向后的第一个真正「瘆人」内容增量，纯代码挂进现有时间系统——新加坡都市传说 + 中元节（农历七月）禁忌 + 异乡人心理恐怖，深夜随机降临。

---

## Task 序列

### Task 1: 恐怖事件纯核心 + 农历七月 ✅
`HorrorEventTypes.h`（`EHorrorEvent` 都市传说 + 七月限定 + `FHorrorEventDef` 含 Mood/Health/Weight/bGhostMonthOnly）。`FHorrorEventSystem`：`IsGhostMonth(month)`（线性月号按 12 折算，第 7 个月即鬼月）+ 加权 `PickEvent(stream, bGhostMonth)`（鬼月限定事件仅鬼月入池；鬼月「无事」权重更低 → 更易出事）+ `GetEventDef`。

### Task 2: HorrorEventSubsystem + HUD + NPC 鬼故事 ✅
`UHorrorEventSubsystem`：订阅 `OnTimeAdvanced`，每天入夜（`LateNight`）掷一次（每天一次），鬼月概率更高且解锁限定事件；命中按定义扣心情/健康、广播 `OnHorrorEvent`；`SetHorrorEnabled`/`SetSeed` 供测试。玩家订阅 → 用底部对话气泡呈现阴森文案（停留 8s）；HUD 进阶行鬼月挂「🕯 农历七月」。保安 Uncle Lim 加七月鬼故事分支（13 楼电梯）。

### Task 3: 测试 + 重建 + 文档 ✅

---

## 完成情况（2026-05-24）

全量重建成功，**全套 66 个 AutomationTest 全绿、零失败**（64 + 新增 2）。

| 产出 | 测试 |
|---|---|
| `FHorrorEventSystem` + `UHorrorEventSubsystem` + 9 个恐怖事件（走廊灯灭/空楼层电梯/空屋拖椅/地铁无倒影/旧樟宜医院/遣返噩梦 + 鬼月限定：冥纸禁忌/咖啡店提醒/Pontianak）+ Uncle Lim 鬼故事分支 + HUD 鬼月指示 | `Horror.PicksAndGates`（鬼月检测、鬼月限定门控、种子可复现、定义有文案）+ `Integration.HorrorEventAffectsMood`（ApplyEvent 扣 Mood/Health、None 不扣、LastEvent） |

**设计：** 恐怖在深夜降临（最应景），农历七月明显升温；事件多为氛围文案 + 小幅心情/健康代价（恐惧/没睡好），稀有的 Pontianak 代价更重。心理恐怖一条（被遣返噩梦）扣健康，呼应异乡人主题。注入种子可复现，`SetHorrorEnabled` 让经济/月结类确定性测试不受干扰（本子系统只在入夜掷，默认不影响月结测试）。

**坑：** 动态多播委托无 `AddLambda`（需 UFUNCTION 接收）；集成测试改用返回值 + 属性变化 + `GetLastEvent` 验证，不直接断言广播。

**待 PIE 复核：** 阴森文案气泡观感、鬼月节奏、深夜触发频率手感。

**留后续：** lo-fi 后处理（噪点/暗角/雾）+ 灯光做氛围；第一人称恐怖桥段（如 13 楼电梯实景）；恐怖音效；更多事件 + 把鬼月做成可见的环境变化（场景变暗/路边祭品）。
