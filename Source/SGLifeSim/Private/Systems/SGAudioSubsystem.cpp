#include "Systems/SGAudioSubsystem.h"

#include "Sound/SoundBase.h"
#include "Kismet/GameplayStatics.h"

void USGAudioSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// 引擎自带合成音，运行时可用，作占位「哔」。无则保持空（静默，不报错）。
	PlaceholderBeep = LoadObject<USoundBase>(nullptr, TEXT("/Engine/EngineSounds/1kSineTonePing.1kSineTonePing"));
}

float USGAudioSubsystem::PlaceholderPitch(ESGSound Sound) const
{
	// 同一个哔声，靠音高区分语义（高=尖锐攻击，低=沉闷/环境）。
	switch (Sound)
	{
	case ESGSound::Gunshot:     return 1.8f;
	case ESGSound::NpcHit:      return 1.5f;
	case ESGSound::Reload:      return 1.3f;
	case ESGSound::UIClick:     return 1.4f;
	case ESGSound::CarEnter:    return 0.9f;
	case ESGSound::CarEngine:   return 0.5f;
	case ESGSound::Footstep:    return 0.7f;
	case ESGSound::Punch:       return 1.1f;
	case ESGSound::PlayerHurt:  return 0.8f;
	case ESGSound::PlayerDeath: return 0.4f;
	case ESGSound::Horror:      return 0.3f;
	default:                    return 1.0f;
	}
}

USoundBase* USGAudioSubsystem::ResolveSound(ESGSound Sound) const
{
	// 优先真音。
	if (const TObjectPtr<USoundBase>* Found = Cues.Find(Sound))
	{
		if (*Found) { return *Found; }
	}
	// 脚步/引擎这类高频/循环音用占位哔会很吵，无真音时宁可静默，等资产接入。
	if (Sound == ESGSound::Footstep || Sound == ESGSound::CarEngine)
	{
		return nullptr;
	}

	// 其余（开火/换弹/受伤…低频离散音）回退占位哔（若开关开且占位音可用）。
	if (bUsePlaceholderBeeps)
	{
		return PlaceholderBeep;
	}
	return nullptr;
}

void USGAudioSubsystem::PlayCue2D(ESGSound Sound, float VolumeMultiplier, float PitchMultiplier)
{
	USoundBase* S = ResolveSound(Sound);
	if (!S) { return; }

	const bool bPlaceholder = (S == PlaceholderBeep);
	const float Pitch = PitchMultiplier * (bPlaceholder ? PlaceholderPitch(Sound) : 1.f);
	// 占位哔降一点音量免得刺耳。
	const float Vol = VolumeMultiplier * (bPlaceholder ? 0.3f : 1.f);
	UGameplayStatics::PlaySound2D(this, S, Vol, Pitch);
}

void USGAudioSubsystem::PlayCueAtLocation(ESGSound Sound, const FVector& Location, float VolumeMultiplier, float PitchMultiplier)
{
	USoundBase* S = ResolveSound(Sound);
	if (!S) { return; }

	const bool bPlaceholder = (S == PlaceholderBeep);
	const float Pitch = PitchMultiplier * (bPlaceholder ? PlaceholderPitch(Sound) : 1.f);
	const float Vol = VolumeMultiplier * (bPlaceholder ? 0.3f : 1.f);

	UWorld* World = GetWorld();
	if (!World) { return; }
	UGameplayStatics::PlaySoundAtLocation(World, S, Location, Vol, Pitch);
}

void USGAudioSubsystem::SetCue(ESGSound Sound, USoundBase* Cue)
{
	Cues.Add(Sound, Cue);
}
