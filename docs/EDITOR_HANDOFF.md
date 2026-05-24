# 编辑器实操落地清单（Editor Handoff）

> 截至 commit `2ca1736`（Plan 26）。逻辑层基本成形（89 测试全绿），真正的瓶颈全在编辑器实操：建关卡、导资产、调灯光、接 UI 触发、PIE 手感。
>
> 这份清单把散落在十几个 commit 注释里的"待编辑器"尾巴汇总成一张可执行地图，附每项的**代码侧接口**（关卡名 / Subsystem 方法 / Widget 类），照着干即可，无需反向考古代码。
>
> **前提：** 编辑器/MCP 工具需 Claude 会话重启才加载；编译需先关编辑器（DLL 锁）。

---

## 1. 需要新建的关卡（11 个）

逻辑层全部按关卡名字符串匹配（单一事实源在注册表），**关卡名必须精确**，否则进出/演出链断。

### 城市枢纽 + 6 个可进地点

| 关卡名 | 用途 | 注册表来源 |
|---|---|---|
| `L_City` | 开放城市枢纽（自由行走 + 小地图 + 走门口按 E 进建筑） | `FLocationRegistry::GetCityLevelName()` |
| `L_Rental` | 出租屋（睡觉/学习/接私活/健身/拜拜） | LocationRegistry |
| `L_HawkerCenter` | 食阁（吃饭/听八卦；NPC: AhMei/Wei） | LocationRegistry |
| `L_Office` | 公司（上班/接私活；NPC: ManagerTan） | LocationRegistry |
| `L_Corridor` | 组屋楼道 | LocationRegistry |
| `L_MRT` | 地铁站 | LocationRegistry |
| `L_Mall` | 商场/超市（购物/吃饭） | LocationRegistry |

### 4 个恐怖场景关卡（OpenLevel 过去演出，演完回原关卡）

| 关卡名 | 对应事件 | 注册表来源 |
|---|---|---|
| `L_ElevatorHorror` | 电梯空楼层 ElevatorGhostFloor | `FHorrorSceneRegistry::GetSceneDef(Elevator)` |
| `L_SubwayHorror` | 末班地铁无倒影 MrtNoReflection | Subway |
| `L_CorridorHorror` | 空屋拖椅声 NeighbourEmptyFlat | Corridor |
| `L_MallHorror` | 打烊后的商场 MallAfterHours | Mall |

**每个关卡至少要有：** 一个 `PlayerStart`（代码按它锚定 NPC 生成位置 + 玩家出生）；可走的地面碰撞。其余靠代码 spawn（见 §2）。

---

## 2. 代码已 spawn 占位道具，资产到位后"换皮"（逻辑不动）

这些已用引擎自带盒子/胶囊跑通，**只需把 mesh 引用换成真资产**，不动逻辑：

- **城市装饰楼**：`USGCityPopulatorSubsystem` 代码铺一排长方体盒子楼（`SGBuildingEntrance.cpp` 占位 Cube 拉伸）。换真建筑 mesh。
- **建筑入口**：`ASGBuildingEntrance` 占位盒子 + "[E] 进入X"。换门 mesh / 招牌。
- **NPC 外观**：`SGWorldPopulatorSubsystem` 复用主角 `SK_Player` + `A_Idle` 当占位。换各 NPC 专属 mesh（视觉分化）。
- **电梯恐怖演出道具**：`AElevatorHorrorDirector` 代码 spawn 电梯壳/门/女鬼剪影/顶灯（全占位盒子）。
  - 女鬼剪影：瘦高盒子 `Scale(0.3, 0.6, 1.8)` → **换导入的 Meshy 女鬼 FBX**（A-Pose，存 `RawAssets/`）。
  - 门：开门目前是瞬移 `SetRelativeLocation` → 后续改 Lerp 平滑。
  - 音效/后处理 TODO 见 `ElevatorHorrorDirector.cpp:109/126/140`（叮声+楼层乱跳 / 低频 drone+脚步 / 尖锐音效+骤暗）。

> 另 3 个恐怖场景（地铁/楼道/商场）目前**没有专属 Director**，只有 EnterScene→关卡→ExitScene 结算文案。要么各写一个 Director（仿 ElevatorHorrorDirector），要么在关卡里手摆 Sequencer 演出。建议先跑通电梯一条，验证手感再复制。

---

## 3. 待接的 UI 触发入口（逻辑已就绪，缺 UMG 面板/按钮）

纯 C++ UMG 已有 6 个 widget（`Source/SGLifeSim/Public/UI/`）：HUD/Activity菜单/对话/终局/地点菜单/小地图。以下功能逻辑已就绪，**只差面板或菜单按钮**：

### 3a. 购物面板（Plan 25）
- 入口：商场 `L_Mall` 菜单加「购物…」按钮。
- 调用：`UShopSubsystem::TryPurchase(EShopItem)`（咖啡/零食/外套/护身符），`CanAfford(Item)` 灰显买不起。
- 反馈：订阅 `OnPurchase`(FText) / `OnPurchaseFailed`(FText) 弹气泡。
- 建议：仿 `SGActivityMenuWidget` 列商品+价格+买按钮。

### 3b. 恐怖图鉴面板（Plan 21）
- 入口：HUD 或暂停菜单加「都市传说图鉴」。
- 数据：`UHorrorCodexSubsystem::GetEntries()` → `TArray<FHorrorCodexEntry>`（已发现含文案，未发现是占位条目）；`GetDiscoveredCount()`/`GetTotalCount()` 做"N/M"。
- HUD 目标行已显示「🕯都市传说 N/M」，面板是点开看详情。

### 3c. 路边祭品博弈入口（Plan 26）
- 入口：鬼月深夜在 `L_City` 走到某路口触发，或 M 菜单显「🔥路边的金纸」。
- 门控：`URoadsideOfferingSubsystem::IsAvailable()`（鬼月+深夜）。
- 抉择：`MakeChoice(ERoadsideOfferingChoice)`（绕开/跨过去/拜一拜），订阅 `OnResolved`(FText) 弹气泡。
- **参照已接好的夜归电梯抉择**（`SGLocationMenuWidget` 的「🛗停在13楼」入口，commit `5eb8bc7`），同模式照搬。

---

## 4. 美术资产包（一次性解决）

用户决定领一整套场景资产包统一换皮（美术弱，恐怖藏拙）。换皮时只动 mesh/材质引用与关卡摆放，**不动任何 C++ 逻辑**——这是整个架构（数据驱动 + 占位盒子）刻意保的解耦。

- 女鬼：Meshy AI 自生成（Pontianak 提示词，A-Pose），FBX 存 `RawAssets/`。
- 城市/室内/恐怖场景：资产包到位后逐关换。

---

## 5. PIE 手感复核（建完关卡后）

- 第一人称移动/视角（鼠标 legacy 轴、相对视线移动、自身 mesh 隐藏）。
- 走门口按 E 进建筑 → OpenLevel → 回程传送（`LocationManagerSubsystem`）。
- 恐怖场景演出节拍（电梯 14 秒女鬼贴脸时间线）锁视角是否到位。
- 深夜恐怖事件 / 两个七月博弈在鬼月触发链是否顺。
- lo-fi 后处理 + 灯光（暗、噪点、低饱和——藏占位资产的廉价感）。
