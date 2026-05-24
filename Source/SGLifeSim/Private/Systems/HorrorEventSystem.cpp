#include "Systems/HorrorEventSystem.h"

FHorrorEventDef FHorrorEventSystem::GetEventDef(EHorrorEvent Event)
{
	FHorrorEventDef D;
	switch (Event)
	{
	case EHorrorEvent::CorridorLights:
		D.Title = FText::FromString(TEXT("组屋走廊的感应灯一盏盏灭了。你站在原地，听见自己的心跳。"));
		D.MoodDelta = -3; D.Weight = 10;
		break;
	case EHorrorEvent::ElevatorGhostFloor:
		D.Title = FText::FromString(TEXT("电梯没人按，却在 4 楼停了下来。门开了，空无一人。"));
		D.MoodDelta = -4; D.Weight = 8;
		break;
	case EHorrorEvent::NeighbourEmptyFlat:
		D.Title = FText::FromString(TEXT("隔壁那间空了半年的屋，半夜传来拖椅子的声音。"));
		D.MoodDelta = -4; D.Weight = 8;
		break;
	case EHorrorEvent::MrtNoReflection:
		D.Title = FText::FromString(TEXT("末班地铁，车窗映出整节车厢——只有你对面那个座位，没有倒影。"));
		D.MoodDelta = -6; D.Weight = 6;
		break;
	case EHorrorEvent::ChangiHospitalTale:
		D.Title = FText::FromString(TEXT("有人讲起旧樟宜医院的事，越讲声音越低，最后谁都没敢接话。"));
		D.MoodDelta = -3; D.Weight = 6;
		break;
	case EHorrorEvent::DeportationNightmare:
		D.Title = FText::FromString(TEXT("你梦见准证被吊销、被押上飞机。惊醒时一身冷汗，下意识去摸护照。"));
		D.MoodDelta = -5; D.HealthDelta = -3; D.Weight = 7;
		break;

	// —— 鬼月限定 ——
	case EHorrorEvent::ZhiQianTaboo:
		D.Title = FText::FromString(TEXT("楼下烧了一整夜的冥纸还没灭。记得：别踩，也别踢。"));
		D.MoodDelta = -2; D.Weight = 14; D.bGhostMonthOnly = true;
		break;
	case EHorrorEvent::KopiWarning:
		D.Title = FText::FromString(TEXT("咖啡店阿伯压低声音：『七月了，晚上别乱讲话，听到有人叫你，别回头。』"));
		D.MoodDelta = -2; D.Weight = 10; D.bGhostMonthOnly = true;
		break;
	case EHorrorEvent::Pontianak:
		D.Title = FText::FromString(TEXT("组屋楼下飘过一团白影，一阵鸡蛋花的香味。你屏住呼吸，直到它消失在黑暗里。"));
		D.MoodDelta = -8; D.HealthDelta = -4; D.Weight = 4; D.bGhostMonthOnly = true;
		break;

	case EHorrorEvent::None:
	default:
		break;
	}
	return D;
}

EHorrorEvent FHorrorEventSystem::PickEvent(FRandomStream& Stream, bool bGhostMonth)
{
	// 组装候选池（含权重）：None + 所有当前可入池的事件。
	struct FCandidate { EHorrorEvent Event; int32 Weight; };
	TArray<FCandidate> Pool;

	Pool.Add({ EHorrorEvent::None, GetNoneWeight(bGhostMonth) });

	int32 Total = GetNoneWeight(bGhostMonth);
	for (int32 i = 1; i < (int32)EHorrorEvent::Count; ++i)
	{
		const EHorrorEvent E = (EHorrorEvent)i;
		const FHorrorEventDef Def = GetEventDef(E);
		if (Def.bGhostMonthOnly && !bGhostMonth)
		{
			continue; // 非鬼月不入池
		}
		Pool.Add({ E, Def.Weight });
		Total += Def.Weight;
	}

	if (Total <= 0)
	{
		return EHorrorEvent::None;
	}

	int32 Roll = Stream.RandRange(0, Total - 1);
	for (const FCandidate& C : Pool)
	{
		if (Roll < C.Weight)
		{
			return C.Event;
		}
		Roll -= C.Weight;
	}
	return EHorrorEvent::None;
}
