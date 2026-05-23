# Engine Validation Prototype Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 在 1~2 周内搭出一个能跑的 UE5 原型，验证 spec 描述的核心机制（等距俯视 / 时间推进 / 对话 / 场景跳转 / meshy 资产）在 UE5 + 一人开发的条件下是否可行。

**Architecture:** UE5.6.1 + C++（核心系统）+ Blueprint（业务逻辑）+ UMG（UI）。本 plan 实现最小可玩切片：1 个主角能在 2 个场景间走动、跟 1 个 NPC 说话、看到时间推进。所有"内容"都是占位符 —— spec 里的真实 NPC / 经济 / 关系 都不在这个 plan 范围。

**Tech Stack:**
- UE5.6.1 LTS（Unreal Engine 5.6.1，长期支持版本）
- C++17（UE5 标准）+ Blueprint
- UMG（UE5 的 UI 系统）
- Git + Git LFS（资产存储）
- Mixamo（免费角色 / 动画，用于原型主角和 NPC）
- meshy.ai（仅在 Task 7 验证一个资产，不大规模生产）
- Visual Studio 2022 Community（Windows 平台 C++ 编译）

**完成后能回答的问题：**
1. UE5 编辑器在用户机器上的启动 + 运行性能是否可接受？
2. 等距俯视 45° 镜头能否实现 spec §9 描述的视觉感觉？
3. meshy 出的资产能否正常进 UE5 + 与其他资产风格统一？
4. C++ + Blueprint 的混合工作流对一人开发是否高效？
5. 用户是否愿意继续在 UE5 全职工作 6+ 个月？

完成后会产出一份 **Decision Log**（`docs/decisions/2026-05-23-engine-validation-outcome.md`）记录答案，用于决定是否进入 Plan 2 或切 Unity。

---

## 验证目标清单（完成本 Plan 后逐项打勾）

- [ ] UE5.6.1 项目能在本机正常构建 + 运行
- [ ] 编辑器冷启动 < 90 秒（一人开发可忍受）
- [ ] 等距俯视 45° 镜头在屏幕上看起来跟 Cult of the Lamb / Disco Elysium 一个调
- [ ] 主角能在场景中用 WASD 走动 + 触发交互
- [ ] 时间块能从 HUD 推进（早 → 上午 → ... → 深夜 → 下一天）
- [ ] 对话框能正确弹出 / 关闭 / 显示一句话
- [ ] 两个场景能通过菜单切换（无明显加载卡顿）
- [ ] 至少 1 个 meshy 生成的道具被正确导入并放进场景，且不出戏
- [ ] 上述全部 push 到 D:\repos\sg-life-sim git 仓库

---

## File Structure

本 plan 完成后，项目目录结构如下：

```
D:\repos\sg-life-sim\
├── .git/
├── .gitignore                            # UE5 标准 + meshy 缓存忽略
├── .gitattributes                        # Git LFS 配置
├── README.md                             # 项目简介 + 上手指南
├── SGLifeSim.uproject                    # UE5 项目入口
├── Config/                               # UE5 自动生成
│   ├── DefaultEngine.ini
│   ├── DefaultEditor.ini
│   ├── DefaultGame.ini
│   └── DefaultInput.ini
├── Content/                              # 所有游戏资产（git-lfs 管理）
│   ├── Characters/
│   │   ├── Player/
│   │   │   ├── BP_PlayerCharacter.uasset     # 主角 Pawn Blueprint
│   │   │   └── Mixamo/                        # Mixamo 模型 + 动画
│   │   └── NPCs/
│   │       └── BP_TestNPC.uasset              # 测试 NPC
│   ├── Levels/
│   │   ├── L_Apartment.umap                  # 出租屋场景
│   │   └── L_HawkerCenter.umap               # 食阁场景
│   ├── UI/
│   │   ├── W_HUD.uasset                       # HUD（显示日期 + 时间块）
│   │   ├── W_DialogueBox.uasset               # 对话框
│   │   └── W_LocationMenu.uasset              # 场景跳转菜单
│   ├── Blueprints/
│   │   ├── GameMode/
│   │   │   └── BP_PrototypeGameMode.uasset
│   │   ├── PlayerController/
│   │   │   └── BP_PlayerController.uasset
│   │   ├── Camera/
│   │   │   └── BP_IsometricCameraRig.uasset
│   │   └── Interactables/
│   │       └── BP_InteractableNPC.uasset
│   └── MeshyTest/
│       └── SM_TestProp.uasset                 # meshy 生成的测试道具
├── Source/                               # C++ 代码
│   └── SGLifeSim/
│       ├── SGLifeSim.Build.cs
│       ├── SGLifeSim.cpp
│       ├── SGLifeSim.h
│       ├── Public/
│       │   ├── Systems/
│       │   │   ├── TimeSystem.h
│       │   │   └── TimeBlock.h                # Enum 定义
│       │   └── Interactables/
│       │       └── InteractableInterface.h
│       └── Private/
│           ├── Systems/
│           │   └── TimeSystem.cpp
│           └── Tests/
│               └── TimeSystemTest.cpp
└── docs/
    ├── specs/
    │   └── 2026-05-23-sg-life-sim-design.md
    ├── plans/
    │   └── 2026-05-23-engine-validation-prototype.md  # 本文件
    └── decisions/
        ├── 2026-05-23-engine-choice.md                # 引擎选型记录
        └── 2026-05-23-engine-validation-outcome.md    # 完成本 plan 后写
```

**模块责任：**
- `TimeSystem` (C++)：纯数据 + 推进逻辑，零 UE5 GameplayFramework 依赖（方便单元测试）
- `BP_PlayerCharacter`：主角 Pawn，含输入处理 + 交互检测
- `BP_IsometricCameraRig`：等距摄像机 actor，挂载到主角
- `W_HUD`：右上角显示时间，按钮触发推进
- `W_DialogueBox`：弹出式 UMG，显示一句话
- `W_LocationMenu`：场景跳转菜单
- `BP_InteractableNPC`：实现 `InteractableInterface`，按 E 触发对话

---

## 前置准备（开始 Task 1 前）

1. **安装 UE5.6.1** 通过 Epic Games Launcher
2. **安装 Visual Studio 2022 Community**，确保勾选了 "Game development with C++" workload 和 ".NET desktop development"
3. **安装 Git LFS**：`git lfs install` 在 PowerShell 跑一次
4. **创建 Mixamo 账号**（mixamo.com，Adobe 账号免费）—— 用于下载主角模型 / 动画
5. **创建 meshy.ai 账号** —— 用于 Task 7 生成一个测试道具

---

## Task 1：项目初始化 + Git LFS 配置

**Files:**
- Create: `D:\repos\sg-life-sim\SGLifeSim.uproject`（由 UE5 自动生成）
- Create: `D:\repos\sg-life-sim\.gitignore`
- Create: `D:\repos\sg-life-sim\.gitattributes`
- Create: `D:\repos\sg-life-sim\README.md`
- Create: `D:\repos\sg-life-sim\Source\SGLifeSim\` 等（由 UE5 自动生成）

**目标：** 在 `D:\repos\sg-life-sim` 创建一个 UE5.6.1 C++ 项目，配置 git-lfs，第一次 commit。

- [ ] **Step 1：UE5 Editor 创建项目**

打开 Epic Games Launcher → Library → 启动 UE 5.6 → 在 Project Browser 选 "Games" → "Blank" → 配置如下：
- **Project Type:** C++
- **Target Platform:** Desktop
- **Quality Preset:** Maximum（一人开发可调）
- **Starter Content:** ✅ 勾选（提供占位资产）
- **Raytracing:** ❌ 不勾（一人机器一般跑不动）
- **Project Location:** `D:\repos\`
- **Project Name:** `SGLifeSim`

点 Create。UE5 会在 `D:\repos\sg-life-sim` 下生成项目（注意：UE5 把 PascalCase 名字保留为 `SGLifeSim.uproject`，但路径已经是 `D:\repos\sg-life-sim`，git 仓库根目录不变）。等待 Visual Studio 自动打开 + 第一次编译完成（5~10 分钟）。

**预期结果：** UE5 编辑器打开 + 显示一个空场景。

- [ ] **Step 2：关闭编辑器 + Visual Studio**

继续配置前先全部关掉，避免文件锁住。

- [ ] **Step 3：写 `.gitignore`**

创建 `D:\repos\sg-life-sim\.gitignore`，内容（基于 [Unreal Engine 官方推荐](https://github.com/github/gitignore/blob/main/UnrealEngine.gitignore) + 本项目特化）：

```gitignore
# Visual Studio
.vs/
*.suo
*.user
*.userosscache
*.sln.docstates

# Visual Studio Code
.vscode/

# JetBrains Rider
.idea/

# Unreal Engine generated
Binaries/
DerivedDataCache/
Intermediate/
Saved/
Build/

# Allow source files in /Source but ignore generated
*.VC.db
*.opensdf
*.opendb
*.sdf
*.sln
*.suo

# Compiled source
*.com
*.class
*.dll
*.exe
*.o
*.so

# OS generated
.DS_Store
Thumbs.db

# UE5 项目根目录的生成文件
/SGLifeSim.sln

# meshy.ai 本地缓存（如果用工具批处理）
/MeshyCache/
/scripts/__pycache__/

# 个人开发笔记 / 草稿
/scratch/
/notes-private/
```

- [ ] **Step 4：写 `.gitattributes`**

创建 `D:\repos\sg-life-sim\.gitattributes`，内容：

```gitattributes
# Auto detect text files
* text=auto

# UE5 资产用 LFS
*.uasset filter=lfs diff=lfs merge=lfs -text
*.umap filter=lfs diff=lfs merge=lfs -text

# 大型媒体文件
*.fbx filter=lfs diff=lfs merge=lfs -text
*.obj filter=lfs diff=lfs merge=lfs -text
*.glb filter=lfs diff=lfs merge=lfs -text
*.gltf filter=lfs diff=lfs merge=lfs -text
*.psd filter=lfs diff=lfs merge=lfs -text
*.png filter=lfs diff=lfs merge=lfs -text
*.jpg filter=lfs diff=lfs merge=lfs -text
*.wav filter=lfs diff=lfs merge=lfs -text
*.mp3 filter=lfs diff=lfs merge=lfs -text
*.ogg filter=lfs diff=lfs merge=lfs -text
*.mp4 filter=lfs diff=lfs merge=lfs -text

# 强制文本模式（防 Windows CRLF 问题）
*.cpp text eol=lf
*.h text eol=lf
*.md text eol=lf
*.ini text eol=lf
*.json text eol=lf
```

- [ ] **Step 5：初始化 Git LFS**

打开 PowerShell，跑：

```powershell
cd D:\repos\sg-life-sim
git lfs install
git lfs track "*.uasset" "*.umap" "*.fbx" "*.png" "*.wav"
```

预期输出：`Tracking "*.uasset"` 等。

- [ ] **Step 6：写 README.md**

创建 `D:\repos\sg-life-sim\README.md`：

```markdown
# sg-life-sim

新加坡人生模拟经营游戏。一个外来程序员在新加坡用五年时间证明自己的故事。

## 状态
🚧 早期原型阶段（Engine Validation Prototype）

## 文档
- 设计文档：[docs/specs/2026-05-23-sg-life-sim-design.md](docs/specs/2026-05-23-sg-life-sim-design.md)
- 当前实施计划：[docs/plans/2026-05-23-engine-validation-prototype.md](docs/plans/2026-05-23-engine-validation-prototype.md)

## 技术栈
- UE5.6.1
- C++17 + Blueprint
- Git + Git LFS

## 开发环境
- Windows 11
- Visual Studio 2022 Community（含 "Game development with C++" workload）
- UE5.6.1 通过 Epic Games Launcher 安装

## 上手
1. 安装 UE5.6.1 + VS2022 + Git LFS
2. clone 本仓库
3. 双击 `SGLifeSim.uproject` 打开
4. 等待第一次编译（5~10 分钟）
```

- [ ] **Step 7：第一次 commit**

```powershell
cd D:\repos\sg-life-sim
git add .gitignore .gitattributes README.md
git commit -m "chore: add gitignore, gitattributes (with LFS), README"

git add SGLifeSim.uproject Config/ Source/ Content/
git status
```

检查 `git status` 输出，确认：
- `.uasset` / `.umap` 文件以 LFS 形式（应该看到 `Git LFS objects` 提示）
- 没有 `Intermediate/` 或 `Binaries/` 被加入

```powershell
git commit -m "feat: initial UE5.6.1 C++ project scaffolding via Editor"
git log --oneline -5
```

**预期：** 两个 commit，仓库可正常 push（暂不需要 remote，本地存档即可）。

---

## Task 2：TimeSystem C++ 类骨架 + 单元测试

**Files:**
- Create: `Source/SGLifeSim/Public/Systems/TimeBlock.h`
- Create: `Source/SGLifeSim/Public/Systems/TimeSystem.h`
- Create: `Source/SGLifeSim/Private/Systems/TimeSystem.cpp`
- Create: `Source/SGLifeSim/Private/Tests/TimeSystemTest.cpp`
- Modify: `Source/SGLifeSim/SGLifeSim.Build.cs`（添加 AutomationTest 模块依赖）

**目标：** 实现一个纯 C++ 的时间系统，单元测试覆盖核心推进逻辑。这是 spec §6.1 的最小实现。

**为什么先写这个：** TimeSystem 是后续所有系统的基础，且最适合 TDD（无 UE Actor / GameMode 依赖）。

- [ ] **Step 1：在 VS 中打开项目 + 编辑 Build.cs**

双击 `D:\repos\sg-life-sim\SGLifeSim.uproject` → 让它打开 UE5 → 然后从 UE5 菜单 `Tools → Open Visual Studio`。

修改 `Source/SGLifeSim/SGLifeSim.Build.cs`，找到 `PublicDependencyModuleNames.AddRange` 那一行，改成：

```csharp
public class SGLifeSim : ModuleRules
{
    public SGLifeSim(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core",
            "CoreUObject",
            "Engine",
            "InputCore",
            "EnhancedInput",
        });

        PrivateDependencyModuleNames.AddRange(new string[]
        {
            "Slate",
            "SlateCore",
            "UMG",
        });

        // 启用 UE5 自动化测试
        if (Target.bBuildEditor)
        {
            PrivateDependencyModuleNames.AddRange(new string[]
            {
                "UnrealEd",
                "AutomationController",
            });
        }
    }
}
```

- [ ] **Step 2：创建 TimeBlock enum**

新建文件 `Source/SGLifeSim/Public/Systems/TimeBlock.h`：

```cpp
#pragma once

#include "CoreMinimal.h"
#include "TimeBlock.generated.h"

/**
 * 一天分成 5 个时间块。spec §5.2。
 * 用 uint8 是因为 UEnum + UPROPERTY 需要。
 */
UENUM(BlueprintType)
enum class ETimeBlock : uint8
{
    Morning     UMETA(DisplayName = "早"),
    Forenoon    UMETA(DisplayName = "上午"),
    Afternoon   UMETA(DisplayName = "下午"),
    Evening     UMETA(DisplayName = "晚"),
    LateNight   UMETA(DisplayName = "深夜"),
};

/** 一周 7 天。Singapore 习惯用周一为一周第一天。 */
UENUM(BlueprintType)
enum class EWeekday : uint8
{
    Monday,
    Tuesday,
    Wednesday,
    Thursday,
    Friday,
    Saturday,
    Sunday,
};
```

- [ ] **Step 3：先写失败的测试**

新建文件 `Source/SGLifeSim/Private/Tests/TimeSystemTest.cpp`：

```cpp
#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Systems/TimeSystem.h"
#include "Systems/TimeBlock.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTimeSystemAdvancesBlockTest,
    "SGLifeSim.TimeSystem.AdvancesOneBlock",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTimeSystemAdvancesBlockTest::RunTest(const FString& Parameters)
{
    FTimeSystem Sys;
    TestEqual(TEXT("initial block is Morning"), Sys.GetCurrentBlock(), ETimeBlock::Morning);

    Sys.AdvanceBlock();
    TestEqual(TEXT("after one advance: Forenoon"), Sys.GetCurrentBlock(), ETimeBlock::Forenoon);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTimeSystemWrapsToNextDayTest,
    "SGLifeSim.TimeSystem.WrapsToNextDay",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTimeSystemWrapsToNextDayTest::RunTest(const FString& Parameters)
{
    FTimeSystem Sys;
    const int32 InitialDay = Sys.GetDayNumber();

    // 推进 5 次（5 个时间块 = 一整天）
    for (int32 i = 0; i < 5; ++i)
    {
        Sys.AdvanceBlock();
    }

    TestEqual(TEXT("after 5 advances: back to Morning"),
        Sys.GetCurrentBlock(), ETimeBlock::Morning);
    TestEqual(TEXT("day number incremented by 1"),
        Sys.GetDayNumber(), InitialDay + 1);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTimeSystemWeekdayRotatesTest,
    "SGLifeSim.TimeSystem.WeekdayRotates",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTimeSystemWeekdayRotatesTest::RunTest(const FString& Parameters)
{
    FTimeSystem Sys;  // 默认从 Monday 早晨开始
    TestEqual(TEXT("initial weekday: Monday"), Sys.GetWeekday(), EWeekday::Monday);

    // 推进 7 天 = 35 个 block
    for (int32 i = 0; i < 35; ++i)
    {
        Sys.AdvanceBlock();
    }

    TestEqual(TEXT("after 7 days: back to Monday"), Sys.GetWeekday(), EWeekday::Monday);
    return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
```

- [ ] **Step 4：编译并跑测试，确认失败**

在 UE5 编辑器（如果还开着，先关掉）→ 在 VS 中 build SGLifeSim 项目。

**预期：编译失败**，因为 `TimeSystem.h` 还不存在。错误大致是 `cannot open source file "Systems/TimeSystem.h"`。这就是我们要的 RED 阶段。

- [ ] **Step 5：写 TimeSystem.h（最小实现）**

新建文件 `Source/SGLifeSim/Public/Systems/TimeSystem.h`：

```cpp
#pragma once

#include "CoreMinimal.h"
#include "Systems/TimeBlock.h"

/**
 * 时间系统。spec §6.1。
 *
 * 纯数据 + 推进逻辑，零 UE GameplayFramework 依赖，方便单元测试。
 * 后续会被 Blueprint 包装的 GameInstanceSubsystem 持有。
 *
 * 内部用 int32 TotalBlocksSinceStart 单一来源，所有派生量（当前 block / 周几 /
 * 第几天）都从它算出来 —— 避免多状态同步 bug。
 */
class SGLIFESIM_API FTimeSystem
{
public:
    FTimeSystem();

    /** 推进一个时间块。可能跨天 / 跨周。 */
    void AdvanceBlock();

    /** 当前所处的时间块。 */
    ETimeBlock GetCurrentBlock() const;

    /** 当前是游戏内的第几天（从 1 开始）。 */
    int32 GetDayNumber() const;

    /** 当前是周几。 */
    EWeekday GetWeekday() const;

    /** 自游戏开始累计的总时间块数（测试用 / 存档用）。 */
    int32 GetTotalBlocks() const { return TotalBlocksSinceStart; }

private:
    /** 单一来源。所有派生量从这里算。 */
    int32 TotalBlocksSinceStart = 0;

    static constexpr int32 BlocksPerDay = 5;
    static constexpr int32 DaysPerWeek = 7;
};
```

- [ ] **Step 6：写 TimeSystem.cpp（最小实现）**

新建文件 `Source/SGLifeSim/Private/Systems/TimeSystem.cpp`：

```cpp
#include "Systems/TimeSystem.h"

FTimeSystem::FTimeSystem()
    : TotalBlocksSinceStart(0)
{
}

void FTimeSystem::AdvanceBlock()
{
    ++TotalBlocksSinceStart;
}

ETimeBlock FTimeSystem::GetCurrentBlock() const
{
    const int32 BlockIndex = TotalBlocksSinceStart % BlocksPerDay;
    return static_cast<ETimeBlock>(BlockIndex);
}

int32 FTimeSystem::GetDayNumber() const
{
    return 1 + (TotalBlocksSinceStart / BlocksPerDay);
}

EWeekday FTimeSystem::GetWeekday() const
{
    const int32 DayIndex = (GetDayNumber() - 1) % DaysPerWeek;
    return static_cast<EWeekday>(DayIndex);
}
```

- [ ] **Step 7：编译并跑测试，确认通过**

在 VS 中 build。编译通过后，打开 UE5 编辑器 → `Tools → Test Automation` → Session Frontend 弹出 → 在 "Automation" 标签下，左侧树展开 `SGLifeSim → TimeSystem` → 勾选三个测试 → 点 "Start Tests"。

**预期：** 3 个测试全部 PASS（绿色）。

如果失败，看 log，最常见原因是 `TotalBlocksSinceStart` 没初始化或推进逻辑错。

- [ ] **Step 8：Commit**

```powershell
cd D:\repos\sg-life-sim
git add Source/SGLifeSim/SGLifeSim.Build.cs `
        Source/SGLifeSim/Public/Systems/TimeBlock.h `
        Source/SGLifeSim/Public/Systems/TimeSystem.h `
        Source/SGLifeSim/Private/Systems/TimeSystem.cpp `
        Source/SGLifeSim/Private/Tests/TimeSystemTest.cpp
git commit -m "feat(time): add FTimeSystem with block/day/weekday advance + tests"
```

---

## Task 3：TimeSystem 集成到 Blueprint + GameInstanceSubsystem

**Files:**
- Create: `Source/SGLifeSim/Public/Systems/TimeSubsystem.h`
- Create: `Source/SGLifeSim/Private/Systems/TimeSubsystem.cpp`

**目标：** 把纯 C++ 的 `FTimeSystem` 包装成 UE5 `UGameInstanceSubsystem`，让 Blueprint 能调用 + 暴露事件。

**为什么：** spec §10.3 的所有系统会通过 GameInstance 注入。Subsystem 是 UE5 推荐的全局系统模式。

- [ ] **Step 1：创建 TimeSubsystem.h**

新建文件 `Source/SGLifeSim/Public/Systems/TimeSubsystem.h`：

```cpp
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Systems/TimeSystem.h"
#include "TimeSubsystem.generated.h"

/** Time 推进时广播给 UI 等订阅者。 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
    FOnTimeAdvanced, ETimeBlock, NewBlock, int32, DayNumber);

UCLASS()
class SGLIFESIM_API UTimeSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    /** Blueprint 调用：推进一个时间块。 */
    UFUNCTION(BlueprintCallable, Category = "SGLifeSim|Time")
    void AdvanceBlock();

    UFUNCTION(BlueprintPure, Category = "SGLifeSim|Time")
    ETimeBlock GetCurrentBlock() const;

    UFUNCTION(BlueprintPure, Category = "SGLifeSim|Time")
    int32 GetDayNumber() const;

    UFUNCTION(BlueprintPure, Category = "SGLifeSim|Time")
    EWeekday GetWeekday() const;

    /** 订阅这个 delegate 接收时间推进通知。 */
    UPROPERTY(BlueprintAssignable, Category = "SGLifeSim|Time")
    FOnTimeAdvanced OnTimeAdvanced;

private:
    FTimeSystem Time;
};
```

- [ ] **Step 2：创建 TimeSubsystem.cpp**

新建文件 `Source/SGLifeSim/Private/Systems/TimeSubsystem.cpp`：

```cpp
#include "Systems/TimeSubsystem.h"

void UTimeSubsystem::AdvanceBlock()
{
    Time.AdvanceBlock();
    OnTimeAdvanced.Broadcast(Time.GetCurrentBlock(), Time.GetDayNumber());
}

ETimeBlock UTimeSubsystem::GetCurrentBlock() const
{
    return Time.GetCurrentBlock();
}

int32 UTimeSubsystem::GetDayNumber() const
{
    return Time.GetDayNumber();
}

EWeekday UTimeSubsystem::GetWeekday() const
{
    return Time.GetWeekday();
}
```

- [ ] **Step 3：编译 + 在 UE5 中验证 Blueprint 可见**

VS 中 build → 打开 UE5 编辑器。

新建一个测试 Blueprint 验证：Content Drawer → 右键 `Content/Blueprints` → New Folder "Test" → 进去 → 右键 → Blueprint Class → Actor → 命名 `BP_TimeTest`。

打开 `BP_TimeTest` → Event Graph → 右键空白处 → 搜索 "Advance Block"。

**预期：** 能看到 `Advance Block (Time Subsystem)` 节点。如果看不到，说明 UCLASS 宏写错或没编译。

把这个测试 Blueprint 删了（验证用，不留）。

- [ ] **Step 4：Commit**

```powershell
git add Source/SGLifeSim/Public/Systems/TimeSubsystem.h `
        Source/SGLifeSim/Private/Systems/TimeSubsystem.cpp
git commit -m "feat(time): expose TimeSystem to Blueprint via GameInstanceSubsystem"
```

---

## Task 4：主角 Pawn + Mixamo 模型 + WASD 移动（Blueprint）

**Files:**
- Download: `Content/Characters/Player/Mixamo/SK_Player.fbx`（Mixamo）
- Download: `Content/Characters/Player/Mixamo/Anim_Idle.fbx`
- Download: `Content/Characters/Player/Mixamo/Anim_Walk.fbx`
- Create: `Content/Characters/Player/ABP_Player.uasset`（Animation Blueprint）
- Create: `Content/Characters/Player/BP_PlayerCharacter.uasset`

**目标：** 让一个 3D 主角能在场景里用 WASD 走动。

**为什么 Mixamo：** 免费 + 即用 + 自带 rig，避开 meshy 不擅长的"人形 + 动画"。

- [ ] **Step 1：下载 Mixamo 资产**

访问 [mixamo.com](https://mixamo.com) → 用 Adobe 账号登录 → 顶部 Characters tab → 挑一个**亚洲男性角色**（例如 "Liam"，可换其他）→ 点 "Download" → 选：
- Format: **FBX Binary (.fbx)**（Mixamo 没有 "FBX For Unreal" 选项；**千万别选 "FBX For Unity"**，那会改骨骼命名约定导致导入 UE5 出错）
- Pose: **T-pose**
- 下载到 `D:\repos\sg-life-sim\Content\Characters\Player\Mixamo\SK_Player.fbx`（手动建文件夹）

回到 Mixamo → Animations tab → 搜 "Idle" → 选一个站立 idle → "Download" → 设置：
- Format: **FBX Binary (.fbx)**（Mixamo 没有 "FBX For Unreal" 选项；**千万别选 "FBX For Unity"**，那会改骨骼命名约定导致导入 UE5 出错）
- Skin: **Without Skin**（共享主角的 skeleton）
- Frames: **30**
- 下载到 `Content/Characters/Player/Mixamo/Anim_Idle.fbx`

再搜 "Walking" → 选一个普通走路（**勾选 "In Place"**，让 root 不前进）→ 同样设置 → 下载到 `Anim_Walk.fbx`。

- [ ] **Step 2：在 UE5 中导入 Mixamo 模型**

打开 UE5 → Content Drawer → 进入 `Content/Characters/Player/Mixamo/` → 右键 → Import to Content → 选 `SK_Player.fbx`。

弹出导入对话框，关键设置：
- **Skeletal Mesh**: ✅
- **Skeleton**: 留空（让它新建）
- **Import Animations**: ❌ 不勾（动画单独导入）
- **Use T0 As Ref Pose**: ✅
- 其他默认

点 Import All。预期生成：
- `SK_Player`（骨骼网格）
- `SK_Player_Skeleton`
- `SK_Player_PhysicsAsset`
- 若干材质 / 贴图

- [ ] **Step 3：导入 Idle / Walk 动画**

右键导入 `Anim_Idle.fbx`，关键设置：
- **Skeleton**: 选刚才生成的 `SK_Player_Skeleton`
- **Animation**: ✅
- 其他默认

重命名生成的 anim 为 `A_Idle`。

同样导入 `Anim_Walk.fbx`，重命名为 `A_Walk`。

- [ ] **Step 4：创建 Animation Blueprint**

Content Drawer → `Content/Characters/Player/` 文件夹 → 右键 → Animation → Animation Blueprint → 弹窗选 Skeleton: `SK_Player_Skeleton` → 命名 `ABP_Player` → 双击打开。

在 AnimGraph 中：
1. 右键 → 添加一个 "Blend Space" 或更简单：用 "Blendspace 1D"（基于速度）。但 prototype 简化处理：
2. 直接拖一个 `A_Idle`（左下 Asset Browser）到 AnimGraph，连到 "Output Pose"。
3. 编译 + 保存。

（更精细的 idle/walk 混合留给 Plan 2，这里只确保角色能动起来。）

- [ ] **Step 5：创建主角 Pawn Blueprint**

Content Drawer → `Content/Characters/Player/` → 右键 → Blueprint Class → **Character**（注意是 Character 不是 Pawn，自带 CapsuleComponent + MovementComponent） → 命名 `BP_PlayerCharacter` → 双击打开。

在 Components 面板：
1. 选 Mesh（继承自 Character）→ Details 面板 → Skeletal Mesh Asset 设为 `SK_Player`
2. 同 Details 面板 → Anim Class 设为 `ABP_Player`
3. 调整 Mesh 的 Transform：Location 设 `(0, 0, -90)`（让脚踩到 Capsule 底）+ Rotation 设 `(0, 0, -90)`（让前方朝 X+）

- [ ] **Step 6：创建 Input Action + Mapping Context**

Content Drawer → `Content/Blueprints` 下右键 New Folder "Input"。进入。

创建 `IA_Move`：
- 右键 → Input → Input Action → 命名 `IA_Move`
- 双击打开 → **Value Type** 改为 `Axis2D (Vector2D)` → 保存

创建 `IMC_Default`：
- 右键 → Input → Input Mapping Context → 命名 `IMC_Default`
- 双击打开 → 点 `+` 添加一个 Mapping，绑定 Action 为 `IA_Move`
- 在 `IA_Move` 下点 `+` **4 次**，添加 4 个 key 条目，分别按下 W / S / A / D 录入按键
- 给每个按键加 Modifier（点条目右边小箭头展开 → Modifiers 下点 `+`）：

| 按键 | Modifier 1 | Modifier 2 |
|------|-----------|-----------|
| W | `Swizzle Input Axis Values`（YXZ → 让 X 输入流向 Y） | — |
| S | `Swizzle Input Axis Values`（YXZ） | `Negate`（X 轴勾上即可，因 swizzle 后 X 输入到 Y） |
| A | `Negate`（X 轴勾上） | — |
| D | — | — |

**逻辑**：D 默认就是 X+（向右）。A 加 Negate 就是 X-（向左）。W/S 加 Swizzle 让按键值从 X 槽流到 Y 槽，S 再 Negate 就是 Y-（向下）。

保存 `IMC_Default`。

- [ ] **Step 7：在 BP_PlayerCharacter 注册 Mapping Context + 响应 IA_Move**

打开 `BP_PlayerCharacter` → Event Graph → 找到 `Event BeginPlay`（没有就右键添加 Event BeginPlay）→ 拖出执行线 → 连接以下节点链：

```
BeginPlay
  → Get Player Controller (Player Index = 0)
  → Get Local Player (cast Controller's Player to Local Player)
  → Get Enhanced Input Local Player Subsystem
  → Add Mapping Context
       - Mapping Context: IMC_Default
       - Priority: 0
```

加输入响应：右键空白 → 搜 "IA Move" → 选 `EnhancedInputAction IA_Move`（带蓝色 ⚡ 图标的事件节点）。从 `Triggered` 引脚拖出：

```
Triggered
  → Get Action Value (Vector2D)
  → Break Vector2D (得到 X, Y 两个 float)

  分支 1（X 控制左右）：
    → Add Movement Input
       - Target: self
       - World Direction: (1.0, 0.0, 0.0)
       - Scale Value: X

  分支 2（Y 控制前后）：
    → Add Movement Input
       - Target: self
       - World Direction: (0.0, 1.0, 0.0)
       - Scale Value: Y
```

编译保存。

- [ ] **Step 8：测试**

把 `BP_PlayerCharacter` 拖到当前默认 Level（点上面的 Play 之前要有玩家起点 Actor，没有就拖一个 Player Start 进 Level） → 设置 World Settings → Default Pawn Class 设为 `BP_PlayerCharacter`。

点 Play。

**预期：** 看到主角站在场景中，按 WASD 能朝四个方向走（这时镜头还是默认顶视角，没问题，下一个 Task 改）。

- [ ] **Step 9：Commit**

```powershell
git add Content/Characters/Player/
git status  # 验证 .fbx 和 .uasset 都进 LFS
git commit -m "feat(player): add Mixamo-based player character with WASD movement"
```

---

## Task 5：等距俯视 45° 摄像机

**Files:**
- Create: `Content/Blueprints/Camera/BP_IsometricCameraRig.uasset`
- Modify: `Content/Characters/Player/BP_PlayerCharacter.uasset`（挂载摄像机 rig）

**目标：** 让镜头变成 spec §9.1 描述的等距俯视 45°，跟随主角。

- [ ] **Step 1：创建 Camera Rig Actor**

Content Drawer → `Content/Blueprints/Camera/` → 右键 → Blueprint Class → Actor → 命名 `BP_IsometricCameraRig` → 双击。

在 Components：
1. 加一个 **Spring Arm**（搜 "SpringArm" → Add）
   - Details：
     - **Use Pawn Control Rotation**: ❌
     - **Inherit Pitch/Yaw/Roll**: ❌（全部不勾）
     - **Target Arm Length**: `1200`
     - **Socket Offset**: `(0, 0, 0)`
     - **Target Offset**: `(0, 0, 0)`
     - **Camera Lag**: ✅ Enabled, Lag Speed = 10
     - **Rotation**: `(Pitch=-45, Yaw=-45, Roll=0)` —— 等距 45° 的关键
2. 加一个 **Camera**（拖到 Spring Arm 下让其成为子节点）
   - Details:
     - **Projection Mode**: `Orthographic` —— 正交投影，等距俯视的关键
     - **Ortho Width**: `1500` （根据画面调整，1200~2000 之间）

- [ ] **Step 2：在 BP_PlayerCharacter 中挂 Camera Rig**

打开 `BP_PlayerCharacter` → Event Graph → BeginPlay → 加一个 "Spawn Actor from Class"：
- Class: `BP_IsometricCameraRig`
- Spawn Transform: Get Actor Transform
- Spawn Even If Colliding: ✅

把 Return Value 接到一个新 Variable `MyCameraRig`（类型 `BP_IsometricCameraRig`）。

紧接着调用 `MyCameraRig → AttachToActor`：
- Parent: self
- Location Rule: Keep World
- Rotation Rule: Snap to Target

然后获取 Player Controller → `Set View Target with Blend`：
- New View Target: `MyCameraRig`
- Blend Time: 0
- Blend Func: Linear

- [ ] **Step 3：测试**

点 Play。

**预期：** 镜头从右上 45° 俯视看主角，主角走动时镜头平滑跟随。

如果镜头角度不对，回到 `BP_IsometricCameraRig` 调 Spring Arm 的 Rotation Yaw / Pitch。Yaw=-45 是斜俯（spec 要的等距），Yaw=0 是正俯（GTA1 风）。

如果镜头跟随有跳跃感，调 Camera Lag Speed（10~15 比较自然）。

- [ ] **Step 4：截图对比 spec 风格**

把当前画面截图保存到 `D:\repos\sg-life-sim\docs\decisions\screenshots\01-isometric-camera-test.png`。

**关键 review：** 这个角度跟 Disco Elysium / Cult of the Lamb 的镜头感觉一致吗？如果不一致，调 Yaw（建议尝试 -30 / -45 / -60，对比）和 Spring Arm Length（800 / 1200 / 1600）。

- [ ] **Step 5：Commit**

```powershell
git add Content/Blueprints/Camera/ Content/Characters/Player/BP_PlayerCharacter.uasset
git add docs/decisions/screenshots/  # 如果创建了截图文件夹
git commit -m "feat(camera): add isometric 45° camera rig attached to player"
```

---

## Task 6：出租屋场景（L_Apartment Level）

**Files:**
- Create: `Content/Levels/L_Apartment.umap`

**目标：** 搭一个最简的出租屋场景，4 面墙 + 地板 + 几件占位家具，主角能在里面走。

**为什么：** 验证场景构建的工作流是否可持续。先用 UE5 Starter Content，meshy 资产放 Task 7。

- [ ] **Step 1：新建 Level**

File → New Level → 选 "Empty Level" → 立即 Save As：`Content/Levels/L_Apartment`。

- [ ] **Step 2：添加基础环境**

Place Actors 面板（左上角）→ 搜：
- **Floor**：拖 1 个，Scale 设 `(5, 5, 1)`，Location `(0, 0, 0)`
- **Directional Light**：拖 1 个 → Details → Rotation `(Pitch=-45, Yaw=30, Roll=0)`，Intensity `3`
- **Sky Atmosphere**：拖 1 个（默认即可）
- **Sky Light**：拖 1 个，Source Type "Captured Scene"
- **Exponential Height Fog**：可选（房间内可省）

- [ ] **Step 3：搭墙（4 面墙）**

从 Starter Content：Content/StarterContent/Architecture（如果没显示，View Options → Show Engine Content）→ 找 `Wall_400x400` 或类似 → 拖 4 面墙围成约 800x800 的房间。

简化版：直接用 4 个 Cube actor（Place Actors → Cube），Scale 改成墙的比例 `(8, 0.2, 4)` 之类，复制 + 旋转。

- [ ] **Step 4：放占位家具**

从 Starter Content/Props 拖几件进来作为占位：
- 一张床（找 Bed 或用 Cube 替代）
- 一张桌（Table）
- 一把椅子（Chair）

不追求美感，是占位。

- [ ] **Step 5：放 Player Start**

Place Actors → Player Start → 拖到场景中央。

- [ ] **Step 6：World Settings 配置**

Window → World Settings → 设：
- **GameMode Override**: 留空（用 Project Settings 的默认）
- **Default Pawn Class**: `BP_PlayerCharacter`（也可以保留默认，从 Project Settings 走）

更稳的做法：Project Settings → Maps & Modes → Default GameMode → 选一个新建的 BP（下一步）。

- [ ] **Step 7：创建 GameMode Blueprint**

Content/Blueprints/GameMode → 右键 → Blueprint Class → Game Mode Base → 命名 `BP_PrototypeGameMode` → 打开 → Class Defaults：
- **Default Pawn Class**: `BP_PlayerCharacter`
- **Player Controller Class**: 留默认 PlayerController

回到 Project Settings → Maps & Modes → Default GameMode → 选 `BP_PrototypeGameMode`。Editor Startup Map / Game Default Map 都选 `L_Apartment`。

- [ ] **Step 8：保存 + 测试**

Ctrl+S 保存 Level。点 Play。

**预期：** 主角生成在出租屋中央，能用 WASD 在墙内走动，碰到墙会停。

- [ ] **Step 9：Commit**

```powershell
git add Content/Levels/L_Apartment.umap `
        Content/Blueprints/GameMode/ `
        Config/DefaultEngine.ini  # GameMode 改动可能写入
git commit -m "feat(level): add L_Apartment prototype scene with player spawn"
```

---

## Task 7：外部 3D 资产导入流程验证（免费资源路线）

**Files:**
- Create: `Content/ExternalAssets/SM_TestProp_<name>.uasset`（导入的测试资产）
- Create: `docs/decisions/2026-05-23-asset-import-workflow.md`（多源工作流）

**目标：** 验证从免费资产源（Fab / Sketchfab / Kenney）下载 → UE5 导入 → 放进场景的流程，确认对一人开发可持续。

**关键验证：** 资产风格是否能融合 / 导入步骤是否可重复 / 找资产到上场景的速度。

**为什么免费资源 vs meshy.ai（决策记录）：**
2026-05-23 决定 MVP 阶段不用 meshy.ai（要付费 $20/月才能稳定产出，免费版 200 credits 太紧）。改用免费资源管线。spec §11 描述的 meshy 流程**推迟到 Plan 3+**，等真正需要"新加坡独特元素"（ERP gantry / 榴莲 / 组屋阳台等通用 marketplace 找不到的资产）时再启用。届时再评估 meshy 是否值这个钱。

- [ ] **Step 1：选一个免费资产源 + 下载一个测试道具**

下面 3 个源任选一个（推荐 A，跟 UE5 集成最深）。**只做一个**就够验证流程。

**选项 A：Fab Marketplace（UE5 原生，最简单 / 推荐）**

1. UE5 编辑器菜单栏 → **Fab**（顶栏右上有个 Fab 按钮，没有就 Window → Fab）
2. 左侧 filter：勾选 **Free** + **Format: UE5** / **Asset Type: 3D Model**
3. 搜索框输入 `cup` / `table` / `furniture` 等通用词
4. 找到一个免费 PBR 道具 → 点资产卡片 → **Add to My Library** → **Add to Project**
5. 资产直接出现在 Content Drawer，**自动跳过下载 + 导入步骤**（这是 Fab 比 Sketchfab 强的地方）

**选项 B：Sketchfab（量最大，免费 CC0 多）**

1. 访问 [sketchfab.com](https://sketchfab.com) → 顶部 Browse
2. 左侧 filter：**Downloadable: ON** + **License: CC0** （这个 license 商用零顾虑）
3. 搜索 `coffee cup` / `chair` / `table` / `food` 等
4. 注册账号（邮箱，免费）→ 点资产 → Download → 选 **Autoconverted format (.glb)** 或 **Original format**
5. 保存到 `D:\repos\sg-life-sim\Content\ExternalAssets\` 目录（需手动建）

**选项 C：Kenney.nl（最快但卡通风重，CC0）**

1. 访问 [kenney.nl/assets](https://kenney.nl/assets) → 上方 filter 选 "3D"
2. 找一个 Kit（如 "Furniture Kit" / "Food Kit" / "City Kit"）→ Download
3. 解压 zip 到 `D:\repos\sg-life-sim\Content\ExternalAssets\<KitName>\` —— 一个 kit 通常包含几十个 FBX

- [ ] **Step 2：导入 UE5（仅选项 B/C 需要；选项 A 跳过）**

Content Drawer → 进入 `Content/ExternalAssets/` → 右键 → **Import to /Game/ExternalAssets** → 选 FBX 或 glb。

导入对话框（保持默认）：
- **Static Mesh**: ✅
- **Skeletal Mesh**: ❌
- **Import Materials**: ✅
- **Import Textures**: ✅
- **Auto Generate Collision**: ✅
- 其他默认

Import All。重命名生成的 Static Mesh 为 `SM_TestProp_<具体名>`（如 `SM_TestProp_Cup`）。

- [ ] **Step 3：放到 L_Apartment 测试**

打开 `L_Apartment` → 把 `SM_TestProp_Cup` 从 Content Drawer 拖到场景里的桌子上 → 在 Details 面板调整 Scale 让大小合理（不同源导出单位可能不同，cup 大概应该 5~10cm 高）。

- [ ] **Step 4：截图 + 视觉评估**

点 Play → 在场景中走到道具附近 → 截图保存到 `docs/decisions/screenshots/02-asset-import-test.png`（手动建 screenshots 文件夹）。

**评估清单（写到 Step 5 的文档里）：**
- [ ] 拓扑质量看上去 OK（等距俯视下不出戏）
- [ ] 材质 / 贴图导入正确（不是粉红色 missing material）
- [ ] 风格跟 Starter Content 占位家具能融合（或后期可通过 toon shader 统一）
- [ ] 文件大小可接受（< 10MB）
- [ ] 整个流程（找 → 下 → 导 → 放）耗时 < 15 分钟

- [ ] **Step 5：写流程文档**

创建 `D:\repos\sg-life-sim\docs\decisions\2026-05-23-asset-import-workflow.md`：

````markdown
# 3D 资产导入流程（MVP 阶段）

## 当前策略

MVP 阶段（Plan 1~2）使用**免费资源**管线，不用 meshy.ai。
等真正需要新加坡独特资产（Plan 3+）再重新评估付费工具。

## 资源源优先级

1. **Fab Marketplace**（UE5 内置）—— 首选，免下载零配置
2. **Sketchfab CC0**（sketchfab.com）—— 量最大，需手动下载
3. **Kenney.nl**（kenney.nl/assets）—— 整套 kit，卡通风
4. **Quixel Megascans**（UE5 内 Bridge）—— 真实材质 / 植物 / 石头，不适合人造道具
5. **Polygon Asia 资产包**（约 $50 买断）—— 等明确需要东南亚城市感时考虑
6. **meshy.ai**（$20/月）—— Plan 3+ 评估，目前 not in scope

## 命名约定

- Static Mesh: `SM_TestProp_<name>` (prototype) / `SM_<Category>_<Name>` (production)
- 例：`SM_Food_KopitiamCup`, `SM_Furniture_Sofa`, `SM_Vehicle_Toyota`

## 导入设置（统一）

- Static Mesh: ✅
- Import Materials: ✅
- Import Textures: ✅
- Auto Generate Collision: ✅
- 单位：UE5 默认 cm

## 已知 quirks

- Sketchfab glb 文件单位有时是 m，导入后 Scale 设 100 才正确
- Kenney 资产风格非常卡通，跟其他源风格混用要后期 toon shader 拉齐
- Fab 的免费资产经常变化，看到合适的尽快 add to library

## 何时升级到付费工具的触发条件

如果以下任一发生，重新评估是否启用 meshy.ai / 买 Polygon Asia 包：
- 连续 3 次找不到需要的资产
- 找到的资产无法风格统一（toon shader 救不回来）
- 项目进入 Plan 3+ 并开始铺新加坡特色道具

## 本次测试评估

- 来源: [Fab / Sketchfab / Kenney]
- 资产: [name]
- 截图: docs/decisions/screenshots/02-asset-import-test.png
- 评分（1~5）:
  - 拓扑: []
  - 材质: []
  - 风格融合: []
  - 整体可用度: []
- 流程耗时: [] 分钟
- 决策: [可用 / 需调整 / 换源重试]
````

填入实际数据。

- [ ] **Step 6：Commit**

```powershell
cd D:\repos\sg-life-sim
git add Content/ExternalAssets/ docs/decisions/ Content/Levels/L_Apartment.umap
git commit -m "feat(assets): validate external asset import workflow (Fab/Sketchfab)"
```

---

## Task 8：NPC Actor + 交互提示

**Files:**
- Create: `Source/SGLifeSim/Public/Interactables/InteractableInterface.h`
- Create: `Content/Blueprints/Interactables/BP_InteractableNPC.uasset`
- Modify: `Content/Characters/Player/BP_PlayerCharacter.uasset`（添加 E 键交互检测）

**目标：** 在 L_Apartment 放一个 NPC，主角走近时显示 "[E] 对话" 提示，按 E 触发对话事件（对话框 widget 在 Task 9 实现）。

- [ ] **Step 1：定义 InteractableInterface（C++）**

新建 `Source/SGLifeSim/Public/Interactables/InteractableInterface.h`：

```cpp
#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "InteractableInterface.generated.h"

UINTERFACE(MinimalAPI, Blueprintable)
class UInteractableInterface : public UInterface
{
    GENERATED_BODY()
};

class SGLIFESIM_API IInteractableInterface
{
    GENERATED_BODY()

public:
    /** 玩家按下交互键时调用。Blueprint 中实现具体行为。 */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "SGLifeSim|Interaction")
    void OnInteract(AActor* Interactor);

    /** 玩家走近时显示的提示文本。 */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "SGLifeSim|Interaction")
    FText GetInteractionPrompt() const;
};
```

VS Build。

- [ ] **Step 2：创建 NPC Blueprint**

Content Drawer → `Content/Blueprints/Interactables/` → 右键 → Blueprint Class → Actor → 命名 `BP_InteractableNPC` → 打开。

Class Settings → Interfaces 面板 → Add → 选 `InteractableInterface`。

Components：
- 加 SkeletalMeshComponent → 设 Mesh 为 `SK_Player`（暂复用主角 mesh，prototype 占位）+ Anim Class 设 `ABP_Player`
- 加 SphereCollision → Sphere Radius `200`（交互检测范围）+ Collision Preset "OverlapAll"

Event Graph：
- 实现 Interface 函数 `On Interact (Interactor)`：暂时只打印 `Print String: "Hello from NPC"`（Task 9 改为弹对话框）
- 实现 `Get Interaction Prompt`：Return Value `"[E] 对话"`

- [ ] **Step 3：在 BP_PlayerCharacter 加交互检测**

打开 `BP_PlayerCharacter` → Components → 加 SphereCollision → 命名 `InteractionDetector` → Sphere Radius `120`。

Event Graph：
- `OnComponentBeginOverlap (InteractionDetector)` → 拖出 Other Actor → DoesImplementInterface (InteractableInterface) → True → 存到变量 `CurrentInteractable`（类型 Actor）→ 通过 `Cast To InteractableInterface` 调 `GetInteractionPrompt` → 用 PrintString 显示（Task 9 改成 UMG widget）
- `OnComponentEndOverlap` → 如果 Other Actor 等于 `CurrentInteractable` → 清空 `CurrentInteractable`

- [ ] **Step 4：添加 E 键 Input Action**

`Content/Blueprints/Input/` → 右键 → Input → Input Action → 命名 `IA_Interact` → Value Type `Digital (bool)`。

打开 `IMC_Default` → + 加 mapping → 选 `IA_Interact` → key 设 E。

`BP_PlayerCharacter` Event Graph：
- `EnhancedInputAction IA_Interact (Triggered)` → Branch (CurrentInteractable IsValid?) → True → Cast `CurrentInteractable` to InteractableInterface → 调 `OnInteract`，传 self 作为 Interactor

- [ ] **Step 5：放 NPC 到 L_Apartment + 测试**

打开 `L_Apartment` → 把 `BP_InteractableNPC` 拖到场景中（离 Player Start 几米远）。

Play。走近 NPC → 按 E → 屏幕显示 "Hello from NPC"。

**预期：** 走近时（步骤 3 的 PrintString）显示交互提示，按 E 触发对话事件。

- [ ] **Step 6：Commit**

```powershell
git add Source/SGLifeSim/Public/Interactables/ `
        Content/Blueprints/Interactables/ `
        Content/Blueprints/Input/ `
        Content/Characters/Player/BP_PlayerCharacter.uasset `
        Content/Levels/L_Apartment.umap
git commit -m "feat(interaction): add InteractableInterface + NPC with E-key trigger"
```

---

## Task 9：UMG 对话框 Widget

**Files:**
- Create: `Content/UI/W_DialogueBox.uasset`
- Modify: `Content/Blueprints/Interactables/BP_InteractableNPC.uasset`（OnInteract 改为弹对话框）
- Modify: `Content/Characters/Player/BP_PlayerCharacter.uasset`（管理对话状态）

**目标：** 按 E 不再 PrintString，而是弹出一个 UMG 对话框，显示 NPC 的一句话，按 Space 关闭。

- [ ] **Step 1：创建 Dialogue Widget**

Content Drawer → `Content/UI/` → 右键 → User Interface → Widget Blueprint → User Widget → 命名 `W_DialogueBox` → 打开。

在 Designer 模式：
1. 拖 Canvas Panel（默认就有）
2. 拖一个 Border 到 Canvas，Anchor 设到底部居中，Size 约 1200x300，Background Color 半透明黑 `(0.05, 0.05, 0.05, 0.85)`
3. 在 Border 里放 Vertical Box
4. Vertical Box 里放：
   - TextBlock：命名 `SpeakerName`，字号 24，颜色白
   - TextBlock：命名 `DialogueText`，字号 20，颜色白，自动换行 ✅
   - TextBlock：命名 `HintText`，字号 14，颜色灰，文本 "[Space] 继续"

把 `SpeakerName` 和 `DialogueText` 在 Details 中勾选 "Is Variable"（让 Graph 能引用）。

- [ ] **Step 2：在 Widget Graph 加输入响应**

切到 Graph → 加变量 `bIsActive` (bool, default false)。

Construct 事件 → Set Input Mode UI Only → Set Show Mouse Cursor False（保留键盘聚焦）。

加一个函数 `ShowDialogue(Speaker FText, Line FText)`：
- 设 SpeakerName.Text = Speaker
- 设 DialogueText.Text = Line
- 设 bIsActive = true

加一个事件 `OnSpaceKeyPressed`（暂用变通：直接在 widget 监听不简单，改用：让 Player Controller 监听按键，然后通过引用调 widget）。

更稳的方案：让对话由 PlayerController 管理，widget 只是显示。

简化 prototype 方案：
- Widget 加一个 OnKeyDown override → 判断 Key 是 SpaceBar → 调用 RemoveFromParent 自销毁 → 同时通知 PlayerController 恢复 Game Input Mode

- [ ] **Step 3：override OnKeyDown**

W_DialogueBox Graph → Functions 面板下点 Override → OnKeyDown → 实现：
- Input: InKeyEvent
- Get Key from InKeyEvent → equals "SpaceBar"?
- True 分支：
  - Remove from Parent
  - Get Player Controller (0) → Set Input Mode Game Only → Set Show Mouse Cursor False
  - Return Value: Handled
- False 分支：Return Value: Unhandled

注意：OnKeyDown 需要 widget 有 keyboard focus → 在 Construct 中 调用 `SetKeyboardFocus`。

- [ ] **Step 4：修改 NPC OnInteract 以弹 widget**

打开 `BP_InteractableNPC` → 找到 `OnInteract` 实现（Task 8 的 PrintString） → 删了 → 改为：
- Create Widget (Class: `W_DialogueBox`, Owning Player: Get Player Controller 0)
- 调返回的 widget 的 `ShowDialogue(Speaker: "邻居 Ah Hua", Line: "你怎么搬来这种小破组屋了？刚来吧？")`
- Add to Viewport
- SetKeyboardFocus on widget（让它能收按键）

- [ ] **Step 5：测试**

Play → 走近 NPC → 按 E。

**预期：**
- 对话框从底部弹出
- 显示 "邻居 Ah Hua: 你怎么搬来这种小破组屋了？刚来吧？"
- 按 Space → 对话框消失 → 主角恢复 WASD 移动

如果按 Space 没反应：可能 widget 没拿到焦点，检查 Step 3 / Step 4 的 SetKeyboardFocus。

- [ ] **Step 6：Commit**

```powershell
git add Content/UI/W_DialogueBox.uasset `
        Content/Blueprints/Interactables/BP_InteractableNPC.uasset
git commit -m "feat(dialogue): add UMG dialogue box widget triggered by NPC interact"
```

---

## Task 10：HUD Widget（显示日期 + 时间块 + 推进按钮）

**Files:**
- Create: `Content/UI/W_HUD.uasset`
- Modify: `Content/Blueprints/PlayerController/BP_PlayerController.uasset`（或在 GameMode 里创建并 Add to Viewport）

**目标：** 屏幕左上角显示 "Day X, [时间块名]"，加一个按钮（或 T 键）推进时间块。订阅 TimeSubsystem.OnTimeAdvanced 自动更新。

- [ ] **Step 1：创建 HUD Widget**

Content Drawer → `Content/UI/` → 右键 → Widget Blueprint → User Widget → 命名 `W_HUD` → 打开。

Designer：
1. 拖一个 Canvas Panel
2. 加一个 Horizontal Box（Anchor 左上，Position 20, 20）
3. 在 Horizontal Box 里放：
   - TextBlock：命名 `TimeText`，字号 22，文本 "Day 1, 早晨" （Is Variable ✅）
   - Spacer
   - Button：命名 `AdvanceButton`，里面放 TextBlock "推进时间块" （Is Variable ✅）

- [ ] **Step 2：创建 PlayerController Blueprint**

`Content/Blueprints/PlayerController/` → 右键 → Blueprint Class → Player Controller → 命名 `BP_PlayerController` → 打开。

Event BeginPlay：
- Create Widget (`W_HUD`, Owning Player: self) → Add to Viewport
- 把 widget 引用存到变量 `HUDWidget`

回到 `BP_PrototypeGameMode` 类设置 → **Player Controller Class** 设为 `BP_PlayerController`。

- [ ] **Step 3：HUD Widget 订阅 TimeSubsystem**

打开 `W_HUD` → Graph → Construct 事件：
- Get Game Instance → Get Subsystem (Class: `TimeSubsystem`) → 调 OnTimeAdvanced 的 Bind Event → 创建一个 Custom Event `OnTimeUpdated(NewBlock ETimeBlock, DayNumber int32)`
- 同时立即调用一次 `UpdateDisplay`（确保第一次显示正确）

实现 `OnTimeUpdated`：
- 调 UpdateDisplay

实现自定义函数 `UpdateDisplay`：
- Get Subsystem TimeSubsystem
- Get Current Block / Get Day Number / Get Weekday
- 把它们 Format Text 成 `"Day {Day}, {Weekday} {Block}"`
- 设 `TimeText.Text` 为这个

- [ ] **Step 4：按钮触发推进**

`W_HUD` Designer → 选 `AdvanceButton` → Details → Events → On Clicked → Add → 在 Graph 里实现：
- Get Game Instance → Get Subsystem TimeSubsystem → Advance Block

- [ ] **Step 5：（可选）T 键也能推进**

`Content/Blueprints/Input/` → 加 `IA_AdvanceTime` (Digital bool) → 在 `IMC_Default` 里绑 T 键。

`BP_PlayerCharacter` → 加 EnhancedInputAction IA_AdvanceTime → Triggered → Get Game Instance → Get Subsystem TimeSubsystem → Advance Block。

- [ ] **Step 6：测试**

Play → 左上角看到 "Day 1, Monday 早" → 点按钮 / 按 T → 文本变 "Day 1, Monday 上午" → 继续点 5 次 → "Day 2, Tuesday 早"。

**预期：** 时间块循环 + 跨天 + 跨周。

- [ ] **Step 7：Commit**

```powershell
git add Content/UI/W_HUD.uasset `
        Content/Blueprints/PlayerController/ `
        Content/Blueprints/Input/IA_AdvanceTime.uasset `
        Content/Blueprints/Input/IMC_Default.uasset `
        Content/Characters/Player/BP_PlayerCharacter.uasset
git commit -m "feat(ui): add HUD showing day/time-block with advance button"
```

---

## Task 11：食阁场景（L_HawkerCenter） + 场景跳转菜单

**Files:**
- Create: `Content/Levels/L_HawkerCenter.umap`
- Create: `Content/UI/W_LocationMenu.uasset`
- Modify: `Content/Blueprints/Input/IMC_Default.uasset`（绑 M 键）
- Modify: `Content/Characters/Player/BP_PlayerCharacter.uasset`（M 键开菜单）

**目标：** 主角按 M 弹一个菜单，列出 "出租屋" / "食阁" 两个选项，选中后 OpenLevel 切换。

- [ ] **Step 1：创建 L_HawkerCenter**

File → New Level → Empty Level → Save As `Content/Levels/L_HawkerCenter`。

复制 L_Apartment 的环境光照设置（Directional Light / Sky Atmosphere / Sky Light）。

放几张 Cube 当作"食阁桌椅"，加 Player Start，地板用一个大 Cube + 不同颜色材质区分。

放一个 `BP_InteractableNPC` 让玩家也能在食阁试对话。

Save。

- [ ] **Step 2：创建 Location Menu Widget**

`Content/UI/` → 右键 → Widget Blueprint → User Widget → 命名 `W_LocationMenu` → 打开。

Designer：
1. Canvas → 一个半透明全屏 Border（背景灰 alpha 0.7）
2. 中央放 Vertical Box
3. Vertical Box 里放 2 个 Button：
   - `BtnApartment` 文本 "回出租屋"
   - `BtnHawker` 文本 "去食阁"
4. 最下面再加一个 Button `BtnCancel` 文本 "取消"

- [ ] **Step 3：Widget Graph 实现按钮**

Construct：Set Input Mode UI Only + Show Mouse Cursor。

BtnApartment OnClicked：
- Remove from Parent
- Set Input Mode Game Only + Hide Mouse Cursor
- Open Level (by Name: "L_Apartment")

BtnHawker OnClicked：同上但 Open Level "L_HawkerCenter"。

BtnCancel OnClicked：
- Remove from Parent
- Set Input Mode Game Only + Hide Mouse Cursor

- [ ] **Step 4：M 键开菜单**

`IMC_Default` → 加 mapping → `IA_OpenLocationMenu` (Digital bool) 绑 M。

`IA_OpenLocationMenu` 资产单独建（如 `IA_OpenLocationMenu`）。

`BP_PlayerCharacter` → EnhancedInputAction IA_OpenLocationMenu → Triggered：
- Create Widget (`W_LocationMenu`) → Add to Viewport → Set Keyboard Focus

- [ ] **Step 5：测试**

Play → 在出租屋按 M → 菜单弹出 → 点 "去食阁" → 加载食阁场景 → 同样可按 M 回出租屋。

**预期：**
- 场景切换 < 3 秒（prototype 阶段，资产少）
- 切换后时间继续从原值开始（**注意：** OpenLevel 默认会重置 GameInstance 之外的所有状态，所以 TimeSubsystem 必须是 GameInstance 子系统 —— 这正是我们 Task 3 选 GameInstanceSubsystem 的原因。验证 HUD 上的 day/time 保持不变 = 系统设计正确）

- [ ] **Step 6：Commit**

```powershell
git add Content/Levels/L_HawkerCenter.umap `
        Content/UI/W_LocationMenu.uasset `
        Content/Blueprints/Input/ `
        Content/Characters/Player/BP_PlayerCharacter.uasset
git commit -m "feat(level): add L_HawkerCenter + menu-based location switching"
```

---

## Task 12：完整 Playthrough + 验证结果记录

**Files:**
- Create: `docs/decisions/2026-05-23-engine-validation-outcome.md`

**目标：** 跑通完整 playthrough，记录验证结果，决定是否进入 Plan 2。

- [ ] **Step 1：完整 playthrough**

按以下脚本走一遍，每步验证：

1. 双击 `SGLifeSim.uproject` → 编辑器开启时间记录到 `_____ 秒`
2. 点 Play → 出生在 L_Apartment
3. 看到 HUD 显示 "Day 1, Monday 早"
4. WASD 移动 → 镜头跟随 → 等距俯视感觉对
5. 走近 NPC → 按 E → 对话框弹出
6. 按 Space → 对话框消失，恢复 WASD
7. 按 T 推进时间块 → HUD 更新到 "上午"
8. 连续按 T 5 次 → HUD 更新到 "Day 2, Tuesday 早"
9. 按 M → 菜单弹出
10. 点 "去食阁" → 加载 L_HawkerCenter → HUD 时间保持 "Day 2, Tuesday 早" ✅
11. 食阁里找 meshy 测试杯子 → 截图保存
12. 按 M 回出租屋

- [ ] **Step 2：写验证结果文档**

创建 `D:\repos\sg-life-sim\docs\decisions\2026-05-23-engine-validation-outcome.md`：

```markdown
# Engine Validation Prototype 验证结果

> **日期：** 2026-MM-DD
> **执行人：** [your name]
> **关联 plan：** docs/plans/2026-05-23-engine-validation-prototype.md
> **关联 spec：** docs/specs/2026-05-23-sg-life-sim-design.md

## 验证目标清单

- [ ] UE5.6.1 项目能在本机正常构建 + 运行
  - 实测：[]
- [ ] 编辑器冷启动 < 90 秒
  - 实测：[] 秒
- [ ] 等距俯视 45° 镜头在屏幕上看起来跟 Cult of the Lamb / Disco Elysium 一个调
  - 实测：[1-5 评分，附截图]
- [ ] 主角能在场景中用 WASD 走动 + 触发交互
  - 实测：[OK / 有问题：____]
- [ ] 时间块能从 HUD 推进（早 → 上午 → ... → 深夜 → 下一天）
  - 实测：[]
- [ ] 对话框能正确弹出 / 关闭 / 显示一句话
  - 实测：[]
- [ ] 两个场景能通过菜单切换（无明显加载卡顿）
  - 实测：[切换耗时 _ 秒]
- [ ] 至少 1 个 meshy 生成的道具被正确导入并放进场景，且不出戏
  - 实测：[1-5 评分]
- [ ] 上述全部 push 到 git 仓库
  - 实测：[]

## 关键回答的问题

### 1. UE5 编辑器性能可接受吗？

[]

### 2. 等距 45° 是 spec §9.1 想要的视觉吗？

[]

### 3. meshy 资产能正常融合吗？

[]

### 4. C++ + Blueprint 工作流对一人开发高效吗？

[]

### 5. 你愿意继续在 UE5 全职工作 6+ 个月吗？

[]

## 决策

- [ ] **GO** — 继续 UE5，开始写 Plan 2（核心系统骨架）
- [ ] **PIVOT** — 切换 Unity 6，重写 Plan 1（用同样的验证清单）
- [ ] **HOLD** — 还需要再做更多验证才能决定，下一步：____

## 后续

如果 GO：进入 brainstorming 或直接 writing-plans 生成 Plan 2。

如果 PIVOT：估计损失约 1~2 周（Plan 1 的代码 + 知识），但 Unity 6 上重做约 3~5 天可以达到同样原型水平。

## 经验教训

[在做这个 plan 过程中学到的、值得记入 future plans 的教训]

- []
- []
```

填入实测数据。

- [ ] **Step 3：Commit + 庆祝**

```powershell
git add docs/decisions/2026-05-23-engine-validation-outcome.md
git commit -m "docs: complete engine validation outcome - decision logged"
git log --oneline
```

预期看到约 12 个 commit（每个 Task 一个 + 个别 Task 多个）。

- [ ] **Step 4：找人聊聊验证结果**

跟 Claude 复盘，决定下一步：
- 如果 GO → "我要写 Plan 2，进入 brainstorming 或直接 writing-plans"
- 如果 PIVOT → "我们要换 Unity 6 重做 Plan 1"
- 如果 HOLD → "我有一个疑问 _____ 需要再讨论"

---

## 自检：本 plan 的设计假设

**已验证（spec 第 §X 节支持）：**
- Plan 1 涵盖 spec §5.1 macro loop 的最小验证（时间 + 场景 + 对话）✅
- 时间系统设计符合 spec §6.1 ✅
- 等距俯视 45° 符合 spec §9.1 ✅
- meshy 流程符合 spec §11.1 ✅

**未涵盖（明确推迟到 Plan 2+）：**
- 经济系统（spec §6.2）
- 关系系统（spec §6.3）
- 进度 / 解锁系统（spec §6.4）
- 失败终局（spec §6.5）
- 真实场景内容（spec §7.1 列的 8~10 个场景）
- 关键 NPC 故事弧（spec §7.2）
- 节日 / 季节系统
- 存档系统（仅依赖 GameInstance 内存状态）
- 音乐 / 音效

**已知风险：**
- UMG 对话框的输入焦点处理在 UE5 中比较烦，如果 Step 9 的 widget 拿不到键盘按键，备用方案：让 PlayerController 监听 Space → 调 widget.RemoveFromParent
- meshy 风格可能与 Mixamo 角色不融合，prototype 阶段不追求完美，记录在 outcome 文档 → Plan 2 处理（toon shader 全局统一）
- Mixamo 角色面部俯视下看不出来，但靠近时可能"AI 生成感"。Plan 2 可考虑 Synty Studios / Marketplace 卡通模型替代

---

## Definition of Done

本 plan 完成 = 所有 Task 1~12 的 checkbox 都打勾 = `2026-05-23-engine-validation-outcome.md` 文档有完整填写的决策。
