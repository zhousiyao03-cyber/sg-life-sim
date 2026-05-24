#pragma once

#include "CoreMinimal.h"
#include "Systems/SGDialogueTypes.h"

/**
 * 对话内容（数据）。Plan 11。
 *
 * 把示例/正式对话树集中在这里构建，供 UDialogueSubsystem 注册、也供测试校验。
 * 与子系统解耦 —— 纯数据，便于单测 ValidateTree。后续可换 DataTable / 资产。
 */
namespace SGDialogueContent
{
	/** 邻居 Ah Hua 的多分支对话弧（组屋老邻居）。 */
	SGLIFESIM_API FDialogueTree BuildAhHuaTree();

	/** 食阁阿姨 Ah Mei（卖鸡饭，唠叨但热心）。 */
	SGLIFESIM_API FDialogueTree BuildAhMeiTree();

	/** 出租屋楼下保安 Uncle Lim（热心，好感高了请喝 kopi）。 */
	SGLIFESIM_API FDialogueTree BuildUncleLimTree();

	/** 同事 Wei（食阁午饭碰到，给职场建议，好感够了帮内推）。 */
	SGLIFESIM_API FDialogueTree BuildColleagueWeiTree();

	/** 公司经理 Mr Tan（上司，职场压力来源；好感够了透露升职门道）。 */
	SGLIFESIM_API FDialogueTree BuildManagerTanTree();

	/** 全部对话树。 */
	SGLIFESIM_API TArray<FDialogueTree> BuildAllTrees();
}
