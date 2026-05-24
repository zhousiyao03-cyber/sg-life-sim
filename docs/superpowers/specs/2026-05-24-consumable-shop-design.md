# 即时消费品商店（Consumable Shop）设计

日期：2026-05-24
状态：已定，待实现

## 目标

给「钱」一个真正的去处。当前商场 `Shopping` 活动只是 `-$80/+心情`，钱攒了没意义。
做一个**即时消费品商店**：花钱买消费品，当场结算效果（不进背包）。最轻的、与现有数据驱动模式一致的消费深化。

## 范围（YAGNI）

- **只做即时消费品**：买 = 当场扣钱 + 当场加属性/理智。不做持久背包、不做携带物、不做使用时机判定。
- 商品效果呼应现有系统，尤其和恐怖系统联动（护身符回理智 = 对抗恐惧螺旋的又一手段）。
- UMG 购物面板留到编辑器实操阶段；本期落地**纯核心 + 子系统 + 测试**，活动菜单先留直接 `TryPurchase` 占位入口，逻辑与美术解耦。

## 架构（沿用 纯核心 + 子系统壳 + 数据驱动 模式）

### 纯核心 `FShopSystem`（可单测，零 UE 子系统依赖）

- `enum class EShopItem : uint8`（BlueprintType）：`HotKopi / WarmJacket / Amulet / Snacks` + `Count`
- `struct FShopItemDef`：`FText Title; int64 PriceCents; int32 AttrDelta[Count]; int32 SanityDelta;`
- `static FShopItemDef GetItemDef(EShopItem)`：商品表（单一真相源）
- `static bool CanAfford(const FShopItemDef&, int64 BalanceCents)`：`BalanceCents >= PriceCents`
  - 经济允许透支（`Charge` 返回 void），所以「钱够不够」由商店核心自己判，不靠经济兜。

### 子系统壳 `UShopSubsystem : UGameInstanceSubsystem`

- `bool TryPurchase(EShopItem)`：
  1. 查现金余额（EconomySubsystem `GetBalance(Cash)`），`CanAfford` 不过 → 广播失败、返回 false
  2. 扣钱（`GetEconomy().Charge(Cash, Price, "Shop")`）
  3. 加属性（PlayerStateSubsystem `ModifyAttribute`）
  4. 回理智（SanitySubsystem `Restore`，仅正值）
  5. 广播成功（带商品标题，供 HUD 提示）、返回 true
- `OnPurchase`（FText 标题）/ `OnPurchaseFailed`（FText 原因）两条动态多播委托。

## 商品表（呼应现有属性 / 恐怖系统）

| 物品 | 价格 | 效果 |
|---|---|---|
| HotKopi 好咖啡 | $4 | +精力 |
| Snacks 零食 | $8 | +心情 |
| WarmJacket 保暖外套 | $60 | +健康 |
| Amulet 护身符 | $40 | +理智（对抗恐惧螺旋） |

价格用「分」存（int64），与经济系统一致。

## 测试

`FShopSystemTest`（纯核心）：
- 每个非 Count 商品都有标题、价格 > 0
- `CanAfford`：余额 = 价格 → true；余额 < 价格 → false
- 护身符 SanityDelta > 0（恐怖联动不回归）

`FShopIntegrationTest`（InitializeStandalone）：
- 钱够：TryPurchase 成功，现金减少 = 价格，对应属性上升
- 钱不够：TryPurchase 失败，现金不变

## 与既有约束的一致性

- 纯核心可单测，子系统壳薄，数据表是单一真相源（同 Activity / HorrorEvent / HorrorScene 模式）。
- 逻辑与美术解耦：本期不碰 UMG，购物面板留编辑器实操。
- 加商品只改 `GetItemDef` 一处。
