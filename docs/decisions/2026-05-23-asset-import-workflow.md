# 3D 资产导入流程（MVP 阶段）

> 关联 plan：docs/plans/2026-05-23-engine-validation-prototype.md（Task 7）

## 当前策略

MVP 阶段（Plan 1~2）使用**免费资源**管线，不用 meshy.ai（见 ADR 0002）。
等真正需要新加坡独特资产（Plan 3+）再重新评估付费工具。

## 资源源优先级

1. **Fab Marketplace**（UE5 内置）—— 首选，免下载零配置（Add to Project 直接进 Content）
2. **Sketchfab CC0**（sketchfab.com）—— 量最大，需手动下载 .glb
3. **Kenney.nl**（kenney.nl/assets）—— 整套 kit，卡通风
4. **Quixel Megascans**（UE5 内 Bridge）—— 真实材质 / 植物 / 石头
5. **Polygon Asia 资产包**（约 $50 买断）—— 等明确需要东南亚城市感时考虑
6. **meshy.ai**（$20/月）—— Plan 3+ 评估，目前 not in scope

## 命名约定

- Skeletal Mesh: `SK_<Name>`（如 `SK_Player`）
- Static Mesh: `SM_TestProp_<name>`（原型）/ `SM_<Category>_<Name>`（生产）
- 动画: `A_<Name>`（如 `A_Idle` / `A_Walk`）

## 导入设置（统一）

**Static Mesh**：Static Mesh ✅ / Import Materials ✅ / Import Textures ✅ / Auto Generate Collision ✅；单位 UE5 默认 cm。

**Skeletal Mesh（Mixamo）**：见 Task 4 实测——经 MCP 自动化时 `manage_asset import` 不暴露 skeleton 参数，
动画 FBX 必须走 Python `FbxImportUI`（`mesh_type_to_import=FBXIT_ANIMATION` + 指定 `skeleton`），
否则各动画会各自新建骨骼，与角色骨骼对不上。

## 已知 quirks

- Sketchfab glb 单位有时是 m，导入后 Scale 设 100 才正确
- Kenney 资产风格很卡通，跨源混用需后期 toon shader 拉齐
- Fab 免费资产经常变化，看到合适的尽快 Add to Library
- Mixamo 导入会附带解出 `.fbm` 贴图临时目录，已在 .gitignore 忽略

## 本次测试评估（Plan 1）

- **来源**：本轮为 Claude 自动化会话完成，**外部资产下载需浏览器交互登录**
  （Fab/Sketchfab 账号），不在自动化能力范围内 —— 这是 Task 7 中唯一需要人手的 5 分钟步骤。
- **已验证的导入管线**：Mixamo 角色 `SK_Player` + `A_Idle`/`A_Walk` 全部经自动化导入成功
  （Skeletal Mesh + Animation 路线已实测打通，见 Task 4 / commit c350c79）。
- **占位道具**：`L_Rental` 里用引擎 BasicShapes 摆了 `SM_TestProp_DeskLamp` 等占位家具，
  验证「道具放进场景 + 等距视角下不出戏」的工作流位点。
- **待人手补**：按上面「资源源优先级 1（Fab）」流程，挑一个免费 PBR 道具 Add to Project，
  替换占位 cylinder，截图存 `docs/decisions/screenshots/02-asset-import-test.png` 并回填下面评分。

| 维度 | 评分(1~5) | 备注 |
|------|-----------|------|
| 拓扑 | _ | 等距俯视下是否出戏 |
| 材质 | _ | 是否正确（非粉红 missing） |
| 风格融合 | _ | 与 Mixamo 角色是否协调 |
| 整体可用度 | _ | |
| 流程耗时 | _ 分钟 | 找→下→导→放 |

## 何时升级到付费工具

以下任一发生即重评 meshy.ai / Polygon Asia：连续 3 次找不到需要的资产 / 找到的无法风格统一 /
项目进入 Plan 3+ 开始铺新加坡特色道具。
