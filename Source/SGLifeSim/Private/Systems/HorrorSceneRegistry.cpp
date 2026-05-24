#include "Systems/HorrorSceneRegistry.h"

FHorrorSceneDef FHorrorSceneRegistry::GetSceneDef(EHorrorScene Scene)
{
	FHorrorSceneDef D;
	switch (Scene)
	{
	case EHorrorScene::Elevator:
		D.LevelName = FName(TEXT("L_ElevatorHorror"));
		D.SanityCost = 20; // 场景演出比纯文案重击。
		D.CodexEntry = EHorrorEvent::ElevatorGhostFloor;
		D.AftermathText = FText::FromString(
			TEXT("你瘫坐在自家门口，冷汗浸透后背。电梯早已停在一楼，门好端端地关着。"));
		break;

	case EHorrorScene::None:
	default:
		break;
	}
	return D;
}

bool FHorrorSceneRegistry::IsHorrorSceneLevel(const FString& LevelName)
{
	// LevelName 可能带 PIE 前缀 / 路径，按子串匹配关卡短名。
	for (int32 i = 1; i < (int32)EHorrorScene::Count; ++i)
	{
		const FHorrorSceneDef Def = GetSceneDef((EHorrorScene)i);
		if (!Def.LevelName.IsNone() && LevelName.Contains(Def.LevelName.ToString()))
		{
			return true;
		}
	}
	return false;
}
