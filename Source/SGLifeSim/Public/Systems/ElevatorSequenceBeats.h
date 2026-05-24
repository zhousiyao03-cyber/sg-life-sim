#pragma once

#include "CoreMinimal.h"

/** 电梯演出的一个节拍类型（Director 据此执行对应动作）。 */
enum class EElevatorBeat : uint8
{
	Enter,          // 锁玩家、门关、灯正常
	Ding,           // 「叮」+ 楼层数字乱跳
	FlickerStart,   // 顶灯开始闪
	BlackOut,       // 灯灭全黑一拍
	LightsBackDoorOpens, // 灯回、门缓缓开、门外空黑楼道
	DroneFootsteps, // 低频 drone + 稀疏脚步
	GhostReveal,    // 灯爆闪：女鬼贴脸出现
	GhostGone,      // 灯灭 → 再亮女鬼已消失
	ScareStinger,   // 尖锐音效 + 屏幕骤暗
	DoorCloseReset, // 门关、灯恢复正常
	Exit,           // 通知 Subsystem 送回（末节点）
};

/** 一个节拍：在 TimeSeconds 时触发 Beat。 */
struct FElevatorBeat
{
	float TimeSeconds = 0.f;
	EElevatorBeat Beat = EElevatorBeat::Enter;

	FElevatorBeat() = default;
	FElevatorBeat(float InTime, EElevatorBeat InBeat) : TimeSeconds(InTime), Beat(InBeat) {}
};

/**
 * 电梯演出节拍表（Plan 24）。纯数据 / 纯函数，零 UE 依赖，可单测。
 * Director 在 BeginPlay 取这张表，用 FTimerManager 按 TimeSeconds 排定时回调执行每个节拍。
 */
class SGLIFESIM_API FElevatorSequenceBeats
{
public:
	/** 返回完整节拍表（按时间升序）。 */
	static TArray<FElevatorBeat> GetBeats();

	/** 整段演出总时长（= 末节点 Exit 的时间）。 */
	static float GetTotalDuration();
};
