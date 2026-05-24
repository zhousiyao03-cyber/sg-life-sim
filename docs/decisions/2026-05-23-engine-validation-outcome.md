# Engine Validation Prototype 验证结果

> **日期：** 2026-05-24
> **执行：** ZEUS ZHOU + Claude（经 UE5 MCP 自动化桥驱动编辑器）
> **关联 plan：** docs/plans/2026-05-23-engine-validation-prototype.md
> **关联 spec：** docs/specs/2026-05-23-sg-life-sim-design.md

## 验证目标清单

- [x] UE5.6.1 项目能在本机正常构建 + 运行
- [ ] 编辑器冷启动 < 90 秒 —— 未单独计时（编辑器全程开启，未冷启动测量）
- [x] 等距俯视 45° 镜头看起来对 —— 正交投影 Pitch/Yaw=-45，OrthoWidth=700（用户拍板），截图见 Saved/Screenshots
- [x] 主角能 WASD 走动 + 触发交互 —— 已验证（方向正确、撞墙停、稳站地板、相机跟随）
- [x] 时间块能推进 —— `UTimeSubsystem` + T 键绑定，有单元测试（Task 2）；HUD 状态行由 `USGHudWidget`（C++ UMG）显示
- [x] 对话框能弹出 —— E→OnInteract（日志确认）+ NPC 台词显示在 `USGHudWidget` 对话气泡（C++ UMG），5 秒自动消失
- [x] 两个场景能切换 —— M 键 OpenLevel，实测 L_Rental ↔ L_HawkerCenter 切换成功，pawn 正确重生
- [~] 外部资产导入 —— Mixamo 导入管线已验证；外部素材下载需人手浏览器步骤（见 asset-import-workflow.md）
- [x] 全部提交到 git 仓库

图例：[x] 完成 / [~] 机制就绪但有替代实现或待人手补 / [ ] 未做

## 关键回答的问题

### 1. UE5 编辑器性能可接受吗？
可接受。开发全程编辑器常开 + Live Coding 热编译（函数体改动几秒~几十秒生效），迭代顺畅。
**注意 Live Coding 限制**：新增 UPROPERTY / 新 UCLASS 的反射不刷新，需关编辑器做完整 Build。

### 2. 等距 45° 是 spec §9.1 想要的视觉吗？
是。正交 + Pitch/Yaw=-45 的等距盒子感符合 Disco Elysium / Cult of the Lamb 调性。
OrthoWidth 经对比（1500 vs 700）由用户定为 **700**（角色清晰可辨，适合人生模拟里盯着角色看）。

### 3. 外部资产能正常融合吗？
Mixamo 角色导入 + 单节点 Idle/Walk 切换工作良好。外部道具的风格融合待人手下载真实资产后评估。

### 4. C++ + Blueprint 工作流对一人开发高效吗？
本原型大量逻辑落在 **C++ 核心**（移动、等距相机、locomotion 切换、交互、时间推进、场景切换、原型 HUD），
Blueprint 仅作薄壳（BP_PlayerCharacter 设资产引用）。配合 UE5 MCP 自动化，编辑器侧操作（导资产、建关卡、
摆 actor、配 Enhanced Input）也能脚本化。结论：对一人开发高效，C++ 优先 + BP 薄壳的取向成立。

### 5. 愿意继续在 UE5 全职 6+ 个月吗？
（待用户填）

## 决策

- [ ] **GO** — 继续 UE5，写 Plan 2（核心系统骨架）
- [ ] **PIVOT** — 切 Unity 6
- [ ] **HOLD** — 还需更多验证：____

（待用户勾选）

## 已知限制 / 与原 plan 的偏差

1. **UMG widget**（已基本解决，2026-05-24 补）。MCP/Python 不能编辑 BP 控件树
   （widget-authoring 子动作不在 schema；Python 不暴露 WidgetTree），但**绕开 BP、纯 C++ 写 UMG 可行**：
   `USGHudWidget : UUserWidget` 在 `RebuildWidget()` 里用 `WidgetTree->ConstructWidget` 搭控件树，
   `ASGPlayerCharacter` `CreateWidget + AddToViewport`。已替换原来的屏幕文本：
   - HUD 状态行（左上）：「Day X · 周几 · 时间块 + 操作提示」
   - 交互提示 + 对话气泡（底部）：靠近显示「[E] 对话」，交互显示 NPC 台词、5 秒消失
   - 场景切换：M 直接跳转（仍未做弹出菜单，原型够用）
   做这步需**关编辑器完整 rebuild**（新 UCLASS 反射 Live Coding 不刷新）——本次已编译通过。
   仅剩"切换菜单"这类交互式 widget（按钮/列表）未做，留待真正需要时。

2. **场景命名**：用 `L_Rental` / `L_HawkerCenter`（plan 原写 `L_Apartment`，等价）。

3. **Live Coding 反射限制（已做完整 rebuild）**：本会话新增的 C++ UPROPERTY（`IdleAnim`/`WalkAnim`/
   `WalkSpeedThreshold` 等）与 `OrthoWidth=700`，原先靠 Live Coding 函数体 + LoadObject 兜底；
   **2026-05-24 已关编辑器做完整 Build**（连同新 UMG 类一起 bake，编译通过）。重开编辑器后这些属性即在
   BP 面板可见可调，`UpdateLocomotionAnimation` 的 LoadObject 兜底可保留作安全网或清理。

4. **外部资产下载**（Task 7）需浏览器登录 Fab/Sketchfab，是唯一留给人手的 5 分钟步骤。

## 经验教训（记入 future plans）

- UE5 MCP 自动化桥能覆盖绝大多数编辑器操作（导 FBX、建蓝图壳、配 Enhanced Input、摆关卡、PIE、截图）。
  两个空白及其绕过：① **BP 控件树编辑不可达** → 改用**纯 C++ UMG**（`UUserWidget::RebuildWidget` +
  `WidgetTree->ConstructWidget`），完全不碰 BP widget；② **新 C++ 类型/UPROPERTY 反射 Live Coding 不刷新**
  → 关编辑器跑一次 `Build.bat`（本机增量约 7 秒）即可，编辑器关着时可反复编译迭代到通过再重开。
- Mixamo 动画 FBX 必须显式指定目标 skeleton（Python FbxImportUI），否则骨骼对不上。
- locomotion 不必上 AnimBlueprint：纯 C++ 按速度 `PlayAnimation` 单节点切换，简单可靠且符合 C++ 核心取向。
- 验证「看不见的逻辑」优先用日志/状态查询（如 `[NPC]` 日志、`GetCurrentLevelName`），别只依赖截图。
- **编辑器视口截图只在窗口前台可见时才渲染**：后台时 `control_editor screenshot` 永不落盘（或全黑）。
  截图前用 PowerShell + Win32 `SetForegroundWindow` 把编辑器拉到前台即可正常落盘。两关卡最终观感
  （出租屋暖木地板+家具、食阁 teal 桌配凳+深色摊位柜台）就是这样补拍确认的。
- **灯光过曝排查**：DirectionalLight intensity 偏高（食阁原为 10）+ 无手动曝光 → 自动曝光把浅色地板冲成纯白。
  统一方案：directional ~3.0 暖白 + 一个 unbound PostProcessVolume 设 manual exposure（EV 11），两关卡观感一致可控。
