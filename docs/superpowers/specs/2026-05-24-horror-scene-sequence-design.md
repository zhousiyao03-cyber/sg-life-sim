# 恐怖场景演出系统设计 — 第一条链:电梯空楼层

日期:2026-05-24
状态:设计待复审

## 背景与目标

项目恐怖玩法此前止步于「文案 + 数值惩罚」:深夜掷骰命中某个 `EHorrorEvent`,
只弹一行阴森气泡、悄悄扣属性,玩家人站原地、画面不变。重复几次就麻木,缺乏冲击。

本设计把恐怖从「飘文字」升级为「**真实场景 + 脚本化演出**」:重磅事件把玩家
**传送进一个专属恐怖场景**,走一段锁视角的定时演出,演完送回。

**范围(刻意收窄):先做 1 个事件跑通整条技术链** —— 电梯空楼层(`ElevatorGhostFloor`)。
验证「触发 → 进场景 → 演出 → 送回 → 结算」可行后,再复制到其他重磅事件。
**本设计不涉及**其余 12 种恐怖事件的场景化(它们继续走轻量文案气泡)。

## 关键决策(brainstorm 已确认)

| 维度 | 决策 |
|---|---|
| 覆盖范围 | 先做 1 个(电梯空楼层)跑通整条链 |
| 触发源 | ① Plan 23 夜归「赶紧进去」赌输;② 深夜掷骰命中 `ElevatorGhostFloor`。两源接同一场景 |
| 场景互动 | 锁视角的脚本化演出(玩家被锁,最多转头) |
| 场景载体 | 独立关卡 `L_ElevatorHorror`,`OpenLevel` 进出 |
| 演出驱动 | 纯 C++ 时间线(状态机 / 定时器序列),不依赖 Sequencer |
| 后果 | 固定扣 20 理智 + 记图鉴「都市传说」条目 |
| 鬼怪呈现 | 直面看清一个女鬼(Pontianak),灯爆闪瞬间贴脸出现约 0.8s,下次灯灭即消失 |
| 美术来源 | 女鬼用 **Meshy AI 自生成**(text-to-3D, T-pose, FBX);电梯壳用免费模型或占位;音效 CC0;后处理零资产 |

## 架构

```
触发源 ──────────────────────────────────┐
  ① NightCommute「赶紧进去」赌输            │
  ② HorrorEvent 深夜命中 ElevatorGhostFloor │
                                          ↓
        UHorrorSequenceSubsystem::EnterScene(EHorrorScene::Elevator)
                                          ↓  记来源关卡名 + 玩家坐标;置 bInScene
                                  OpenLevel("L_ElevatorHorror")
                                          ↓  新关卡 BeginPlay
                        AElevatorHorrorDirector(关卡内演出导演 Actor)
                                          ↓  纯 C++ 定时线:锁视角 / 灯 / 门 / 女鬼 / 音效
                                          ↓  演完
        UHorrorSequenceSubsystem::ExitScene()
                                          ↓  结算:Sanity->Drain(20) + Codex 记录 + 广播事后文案
                                  OpenLevel(来源关卡名)
```

### 职责切分(单一职责、可独立理解/测试)

- **`UHorrorSequenceSubsystem`**(GameInstanceSubsystem):跨关卡的「去 / 回」总控。
  存来源关卡名、触发 OpenLevel、回来后结算、防重入。**不管演出细节**。
- **`AElevatorHorrorDirector`**(Actor,只活在电梯关卡):跑定时演出时间线 + 锁玩家视角
  + 代码 spawn 道具(电梯壳 / 门 / 女鬼 / 灯)。演完调 Subsystem 的 ExitScene。
- **`FHorrorSceneRegistry`**(纯函数 / 数据,可单测):场景 → 定义(关卡名 / 理智代价 /
  图鉴条目 / 事后文案)。
- **触发源**(NightCommute / HorrorEvent):只调 `EnterScene`,不知道演出细节。

做成 Actor 而非 Subsystem,因为导演要锁玩家、spawn 道具、且只存在于那一关。

## 演出时间线(`AElevatorHorrorDirector`,约 14 秒)

玩家被锁在电梯中央(禁移动 / 禁交互,保留转头)。`FTimerManager` 排定时回调驱动:

| 时刻 | 事件 | 实现 |
|---|---|---|
| 0s | 进场:锁玩家,门关,顶灯正常 | 关移动输入,保留 `bUsePawnControlRotation` |
| 1s | 「叮」一声,楼层数字乱跳(…12→13→…) | 音效 + 楼层文本刷新 |
| 3s | 顶灯开始闪烁,越来越频 | 定时改电梯内 PointLight 强度 |
| 5s | 灯灭一拍,全黑 0.5s | 灯强度归零 |
| 5.5s | 灯回,门缓缓开,门外是空的黑楼道 | 灯恢复 + 门 Actor 代码 Lerp 平移 |
| 7s | 门外黑暗,低频 drone 渐起,稀疏脚步声 | 音效 |
| 8s | 灯爆闪一下:女鬼已贴在门口,清晰占满视野约 0.8s | 女鬼可见性开 + 灯瞬亮 |
| 8.8s | 灯灭 → 再亮已空无一人 | 女鬼可见性关 |
| 11s | 尖锐音效 + 屏幕骤暗(后处理畸变) | 音效 + 后处理参数瞬变 |
| 12s | 门关,灯恢复正常,一切如常 | 门复位 + 灯恢复 |
| 14s | 通知 Subsystem → 结算 → 送回 | `ExitScene()` |

**节奏数据**(各节点秒数)抽成可单测数据:验证序列单调递增、总时长合理、
末节点必调 ExitScene。

「女鬼」「门」「灯」由导演在 `BeginPlay` 代码 spawn;占位阶段用基础几何体,
资产到位后换 mesh 引用,**时间线代码不动**。

## 进出衔接 + 数据流

**进:** 触发源调 `EnterScene(Elevator)` → Subsystem 记当前关卡名 + 玩家坐标(GameInstance
跨关卡存活)→ 置 `bInScene` → `OpenLevel("L_ElevatorHorror")`。

**演:** 电梯关卡导演跑时间线。

**出:** 导演调 `ExitScene()` → Subsystem 结算(`Sanity->Drain(20)` + Codex 记
`ElevatorGhostFloor` + 广播事后文案「你瘫坐在自家门口,冷汗浸透后背。电梯早已停在一楼。」)
→ 清 `bInScene` → `OpenLevel(来源关卡名)`。

### 已定取舍

- **回程坐标**:OpenLevel 回原关卡重生在 PlayerStart(不还原离开点)。坐标先存但不强制
  还原,重定位逻辑留作后续。第一版接受「恐怖结束后人回到关卡入口」。
- **防重入**:`bInScene` 期间拒绝新的 `EnterScene`,演出不叠。

## 落地改动

### 纯 C++(可独立完成)

| 文件 | 改动 |
|---|---|
| 新 `HorrorSceneTypes.h` | `EHorrorScene` 枚举 + `FHorrorSceneDef`(关卡名 / 理智代价 / 图鉴条目 / 事后文案) |
| 新 `HorrorSceneRegistry.h/.cpp` | 纯函数:场景 → 定义。可单测 |
| 新 `HorrorSequenceSubsystem.h/.cpp` | EnterScene / ExitScene / 防重入 / 存来源关卡 / 结算 |
| 新 `ElevatorHorrorDirector.h/.cpp` | 关卡内演出导演 Actor:锁视角 + 定时线 + spawn 道具 |
| 改 `NightCommuteSubsystem.cpp` | StepIn 赌输:由「扣理智」改为调 `EnterScene(Elevator)` |
| 改 `HorrorEventSubsystem.cpp` | 命中 `ElevatorGhostFloor`:改调 `EnterScene(Elevator)` |
| 新测试 ×2 | Registry 单测 + Subsystem 集成测 |

### 必须进编辑器 / MCP(一次性)

- 新建 `L_ElevatorHorror.umap`(近乎空关卡,道具代码 spawn)
- 资产到位后:女鬼 / 电梯壳 mesh 赋给导演引用、配后处理 Volume + 灯光氛围

## 资源清单(免费 / 自生成)

| 资源 | 来源 | 授权 |
|---|---|---|
| 女鬼模型 | **Meshy AI** text-to-3D 自生成(T-pose / Standard / FBX);提示词见下 | Meshy CC BY 4.0(致谢署名)或付费私有 |
| 电梯壳(可选) | Fab 免费区 / Sketchfab "elevator";占位阶段用代码盒子 | 各资产单独许可,领时核对 |
| 音效(叮 / drone / 脚步) | Freesound(CC0 筛)/ OpenGameArt CC0 / Pixabay / itch.io "ROT: Horror Audio Bundle" | CC0 可商用免署名 |
| lo-fi 后处理(噪点 / 暗角 / 低分辨率感) | UE 自带 Post Process Volume | 零资产 |

**Meshy 女鬼提示词:**
```
A Singaporean Pontianak female ghost, standing upright in T-pose, facing forward,
long black hair covering most of her pale face, wearing a tattered long white dress,
gaunt emaciated body, bare feet, hollow eyes, deathly pale grey skin, eerie and
unsettling, full body, realistic horror style, dark muted colors
```

## 测试策略

- **`FHorrorSceneRegistry` 单测**:每个 `EHorrorScene` 有合法关卡名、理智代价 > 0、
  对应图鉴条目;时间线节奏单调递增、总时长合理、末节点调 ExitScene。
- **Subsystem 集成测**(`InitializeStandalone`):`EnterScene` 置 `bInScene` 并存来源关卡;
  `ExitScene` 真扣 20 理智 + 记图鉴 + 清标志;演出途中重入被拒。
- **回归**:现有 78 测试保持全绿(触发源改动不破坏 NightCommute / HorrorEvent 既有测试)。

注:OpenLevel / 真实演出 / 锁视角属运行时行为,自动化测试覆盖不到,留 PIE 验证。

## 不做(YAGNI)

- 其余 12 种恐怖事件的场景化(本链验证后再说)
- 还原离开点坐标(第一版重生 PlayerStart)
- 场景内可行走探索 / 关键交互玩法(本版纯锁视角演出)
- 多结局分支(被鬼拖走=死亡结局等,超出第一条链)
- Sequencer 电影化镜头(纯 C++ 时间线足够)

## 关联的待决改动(本链开工前先清理工作区)

当前工作区有两组未提交改动,需在动工前定夺:
1. **Plan 23 PIE 入口**(4 文件,已编译、78 测试绿):建议提交。
2. **「每晚必出」**(`PickEvent` 加 `bGuaranteeEvent`,3 文件,未编译未测):
   做了真场景后「每晚必出」会加速麻木,需与用户确认保留 / 调整 / 回退。
