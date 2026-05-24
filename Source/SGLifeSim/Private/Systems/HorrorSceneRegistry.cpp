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

	case EHorrorScene::Subway:
		D.LevelName = FName(TEXT("L_SubwayHorror"));
		D.SanityCost = 18;
		D.CodexEntry = EHorrorEvent::MrtNoReflection;
		D.AftermathText = FText::FromString(
			TEXT("地铁到站，门一开你几乎是冲出去的。站台空荡荡，可你不敢回头看那节车厢。"));
		break;

	case EHorrorScene::Corridor:
		D.LevelName = FName(TEXT("L_CorridorHorror"));
		D.SanityCost = 16;
		D.CodexEntry = EHorrorEvent::NeighbourEmptyFlat;
		D.AftermathText = FText::FromString(
			TEXT("你冲进自家屋里反锁了门。隔壁那间，明明空了半年——拖椅子的声音，停了。"));
		break;

	case EHorrorScene::Mall:
		D.LevelName = FName(TEXT("L_MallHorror"));
		D.SanityCost = 17;
		D.CodexEntry = EHorrorEvent::MallAfterHours;
		D.AftermathText = FText::FromString(
			TEXT("保安终于来开了门。你说不出话，只是回头看了眼那道——此刻已经停住的——扶梯。"));
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
