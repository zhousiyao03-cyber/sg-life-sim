# 开放城市枢纽设计

日期:2026-05-24
状态:设计待复审（用户授权自主推进实现）

## 背景与目标

此前空间是「纯室内关卡 + M 菜单瞬切」：只有出租屋、食阁两个关卡，靠菜单文字切换，
世界无在场感、撑不起「过一生」的叙事。

升级为**开放城市枢纽**：出租屋出来是一张可自由行走的室外城市地图，地图上散布建筑——
**标记的地点能进**（走到门口按 E → 切进室内关卡），**其他建筑只是长方体外壳**（纯装饰、
填充城市感、不可进）。一个小地图 UI 显示玩家在城市的位置。

**架构选型：城市是枢纽，入室内切关卡。**（非「全装进一个大关卡」——那性能/加载/重做成本都最大。）
室内外解耦，各自换皮；复用现有 OpenLevel 关卡切换机制，改动最小最稳。

美术：现阶段全部用代码 spawn 占位盒子（城市地面 / 建筑外壳 / 入口），**美术资产后续一次性解决**
（领整套场景资产包换皮）。所以本设计严格做到**逻辑与美术解耦**：spawn 的 mesh 引用可整体替换，
布局 / 交互 / 切换逻辑不动。

## 架构

```
L_City（新建大室外关卡）
  └ USGCityPopulatorSubsystem（WorldSubsystem，BeginPlay 铺城市）
       ├ 按 FLocationRegistry 在各地点城市坐标 spawn ASGBuildingEntrance（标记建筑，可进）
       ├ 批量 spawn 长方体装饰楼（不可进，填充城市感）
       └ spawn 地面平面

玩家在 L_City 自由走 → 走近 ASGBuildingEntrance → HUD「[E] 进入 X」→ 按 E
       ↓
  ASGBuildingEntrance::OnInteract → USGLocationManagerSubsystem::EnterLocation(Location)
       ↓  记下玩家城市坐标 / 朝向（回程用）
  OpenLevel(室内关卡，如 L_Rental)
       ↓  室内活动（现有：活动菜单 / NPC 对话 / 恐怖触发……）
  室内「离开」→ ReturnToCity()
       ↓  OpenLevel(L_City) + 玩家传送回离开时的建筑门口坐标
```

### 职责切分

- **`FLocationRegistry`**（纯数据 / 函数，可单测）：游戏所有地点的单一事实源。
  `ELocation`（Rental/Hawker/Office/Corridor/MRT/Mall）→ `FLocationDef`
  （室内关卡名 / 显示名 / **城市世界坐标** / 可做活动集 / NPC roster 引用）。
  城市建筑摆哪、进哪个关卡、室内能干什么，全从这张表读。新增地点 = 加一条 + 建 .umap 壳。
- **`USGLocationManagerSubsystem`**（GameInstanceSubsystem，跨关卡存活）：
  `EnterLocation(ELocation)` 记城市坐标 + OpenLevel 室内；`ReturnToCity()` OpenLevel 回城市
  + 传送玩家回门口。补上 Plan 24 遗留的「回程重生 PlayerStart」脆弱点。
- **`ASGBuildingEntrance`**（Actor，实现 `IInteractableInterface`）：城市里的可进建筑门口。
  走近显「[E] 进入 X」，按 E 调 LocationManager。配占位盒子 mesh。
- **`USGCityPopulatorSubsystem`**（WorldSubsystem）：L_City BeginPlay 时按注册表铺城市
  （入口建筑 + 装饰楼 + 地面），全代码 spawn，幂等。
- **小地图**（纯 C++ UMG）：俯视描点显示玩家在城市的位置 + 各标记建筑点。

### 复用 / 改动现有

- **`FindNearbyInteractable` 改按接口找**：现在硬编码只找 `ASGInteractableNPC`（cpp:218），
  改为 `GetAllActorsWithInterface(UInteractableInterface)`，NPC + 建筑入口都能被发现交互。
  `TryInteract` 已用通用 `Execute_OnInteract`，建筑进入逻辑放各自 `OnInteract_Implementation`。
- **`SGWorldPopulatorSubsystem` 的 roster** 并入 / 复用 `FLocationRegistry`（地点→NPC 名单
  本就是地点数据的一部分）。
- **M 菜单**：城市里 M 菜单保留存档 / 读档等全局项；「去哪」由走路 + 进门取代
  （地点切换不再靠菜单按钮）。室内的 M 菜单保留「离开 → 回城市」。

## 六个地点（首批）

| ELocation | 室内关卡 | 城市坐标 | 室内内容 |
|---|---|---|---|
| Rental | L_Rental（已有） | 城市一角 | 睡 / 学 / 接私活 / 拜拜；Ah Hua / Uncle Lim |
| Hawker | L_HawkerCenter（已有） | 邻近 | 吃饭 / 听八卦；Ah Mei / Wei |
| Office | L_Office（待建壳） | 商业区 | 上班 / 升职 / 跳槽；同事 NPC |
| Corridor | L_Corridor（待建壳） | 出租屋楼附近 | 恐怖核心：夜归 / 电梯事件入口 |
| MRT | L_MRT（待建壳） | 交通枢纽 | 通勤 / 地铁恐怖 |
| Mall | L_Mall（待建壳） | 商业区 | 购物 / 消费 / 社交 |

## 实现顺序（全代码、TDD、每步编译 + 测试）

1. `FLocationRegistry` + 单测（每地点合法关卡名 / 唯一城市坐标 / 活动集 / roster）
2. `USGLocationManagerSubsystem` + 集成测（EnterLocation 记坐标、ReturnToCity 复位）
3. `ASGBuildingEntrance`（交互接口 + 占位盒子）
4. `USGCityPopulatorSubsystem`（按注册表铺城市）
5. `FindNearbyInteractable` 改按接口找（不回归现有对话）
6. 小地图 UMG + 全量编译 + 测试全绿

## 测试策略

- **`FLocationRegistry` 单测**：每地点关卡名合法、城市坐标互不重叠、可做活动是全集子集、
  roster NPC 都有注册对话树（交叉现有校验）。
- **`LocationManager` 集成测**（InitializeStandalone）：EnterLocation 记下坐标 / 目标；
  ReturnToCity 复位状态。OpenLevel 运行时行为留 PIE 验证（同 Plan 24 处理）。
- **回归**：现有 81 测试全绿；FindNearbyInteractable 改动不破坏 NPC 对话。
- 城市行走 / 进门 / 小地图渲染属运行时，留 PIE 验证。

## 不做（YAGNI）

- 城市内 NPC / 车流 / 时间天气（先把「走路 + 进门 + 小地图」骨架跑通）
- 室内外无缝（明确选了切关卡，不做大世界流送）
- 装饰楼的多样外观（先统一长方体，资产包换皮）
- 真实新加坡地理还原（坐标先按玩法布局，不求写实）

## 与现有系统的衔接

- **恐怖**：Corridor / MRT 是恐怖触发的物理落点；Plan 24 电梯场景从 Corridor 进。
- **活动**：Activity 按 `FLocationRegistry` 的可做活动集过滤（室内菜单只列本地点能做的）。
- **里程碑 / 经济**：不受影响，仍是全局 Subsystem。

## 已知风险

- L_City 大量 spawn 盒子的性能：首批地点 + 适量装饰楼可控；装饰楼数量设上限。
- 城市坐标与小地图坐标的映射：注册表存世界坐标，小地图按比例缩放描点。
