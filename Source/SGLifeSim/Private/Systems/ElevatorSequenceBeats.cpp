#include "Systems/ElevatorSequenceBeats.h"

TArray<FElevatorBeat> FElevatorSequenceBeats::GetBeats()
{
	// 约 14 秒的锁视角演出。时间见设计文档时间线表。
	return {
		{ 0.0f,  EElevatorBeat::Enter },
		{ 1.0f,  EElevatorBeat::Ding },
		{ 3.0f,  EElevatorBeat::FlickerStart },
		{ 5.0f,  EElevatorBeat::BlackOut },
		{ 5.5f,  EElevatorBeat::LightsBackDoorOpens },
		{ 7.0f,  EElevatorBeat::DroneFootsteps },
		{ 8.0f,  EElevatorBeat::GhostReveal },
		{ 8.8f,  EElevatorBeat::GhostGone },
		{ 11.0f, EElevatorBeat::ScareStinger },
		{ 12.0f, EElevatorBeat::DoorCloseReset },
		{ 14.0f, EElevatorBeat::Exit },
	};
}

float FElevatorSequenceBeats::GetTotalDuration()
{
	const TArray<FElevatorBeat> Beats = GetBeats();
	return Beats.Num() > 0 ? Beats.Last().TimeSeconds : 0.f;
}
