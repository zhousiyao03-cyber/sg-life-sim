#include "World/SGDayNightController.h"
#include "Systems/TimeSubsystem.h"
#include "Systems/WeatherSubsystem.h"

#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/GameInstance.h"
#include "Kismet/GameplayStatics.h"

ASGDayNightController::ASGDayNightController()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ASGDayNightController::BeginPlay()
{
	Super::BeginPlay();

	// 抓场景里的光照 Actor（关卡建好时放的那几个；没有就保持空、对应步骤跳过）。
	if (UWorld* World = GetWorld())
	{
		TArray<AActor*> Found;
		UGameplayStatics::GetAllActorsOfClass(World, ADirectionalLight::StaticClass(), Found);
		if (Found.Num() > 0) { Sun = Cast<ADirectionalLight>(Found[0]); }

		Found.Reset();
		UGameplayStatics::GetAllActorsOfClass(World, ASkyLight::StaticClass(), Found);
		if (Found.Num() > 0) { Sky = Cast<ASkyLight>(Found[0]); }

		Found.Reset();
		UGameplayStatics::GetAllActorsOfClass(World, AExponentialHeightFog::StaticClass(), Found);
		if (Found.Num() > 0) { Fog = Cast<AExponentialHeightFog>(Found[0]); }
	}

	// 订阅时间推进 + 立刻按当前时间块上一次光照（进关卡就对）。
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UTimeSubsystem* Time = GI->GetSubsystem<UTimeSubsystem>())
		{
			Time->OnTimeAdvanced.AddUniqueDynamic(this, &ASGDayNightController::HandleTimeAdvanced);
			ApplyLighting(Time->GetCurrentBlock());
		}
	}
}

void ASGDayNightController::HandleTimeAdvanced(ETimeBlock NewBlock, int32 /*DayNumber*/)
{
	ApplyLighting(NewBlock);
}

void ASGDayNightController::ApplyLighting(ETimeBlock Block)
{
	// 每个时间块一套：太阳俯仰角(pitch) / 强度 / 颜色，天空光强度，雾密度。
	// pitch 负值=太阳在地平线上方往下照；正值（深夜）=沉到地平线下→近乎无直射。
	float SunPitch = -45.f;
	float SunYaw = -30.f;
	float SunIntensity = 6.f;
	FLinearColor SunColor = FLinearColor::White;
	float SkyIntensity = 1.f;
	float FogDensity = 0.02f;
	FLinearColor FogColor = FLinearColor(0.5f, 0.6f, 0.7f);

	switch (Block)
	{
	case ETimeBlock::Morning:   // 晨：太阳低、东升、暖
		SunPitch = -15.f; SunYaw = 80.f;  SunIntensity = 4.f;
		SunColor = FLinearColor(1.0f, 0.8f, 0.6f);
		SkyIntensity = 0.8f; FogDensity = 0.04f; FogColor = FLinearColor(0.8f, 0.75f, 0.7f);
		break;
	case ETimeBlock::Forenoon:  // 上午：高、白、强
		SunPitch = -55.f; SunYaw = 20.f;  SunIntensity = 8.f;
		SunColor = FLinearColor(1.0f, 0.98f, 0.95f);
		SkyIntensity = 1.2f; FogDensity = 0.015f;
		break;
	case ETimeBlock::Afternoon: // 下午：高偏西、白暖
		SunPitch = -50.f; SunYaw = -40.f; SunIntensity = 7.f;
		SunColor = FLinearColor(1.0f, 0.95f, 0.85f);
		SkyIntensity = 1.1f; FogDensity = 0.02f;
		break;
	case ETimeBlock::Evening:   // 黄昏：低、西沉、橙红
		SunPitch = -8.f;  SunYaw = -85.f; SunIntensity = 3.f;
		SunColor = FLinearColor(1.0f, 0.5f, 0.3f);
		SkyIntensity = 0.6f; FogDensity = 0.05f; FogColor = FLinearColor(0.6f, 0.45f, 0.4f);
		break;
	case ETimeBlock::LateNight: // 深夜：太阳沉到地平线下、冷蓝极弱、浓雾 —— 恐怖时刻
		SunPitch = 10.f;  SunYaw = -100.f; SunIntensity = 0.4f;
		SunColor = FLinearColor(0.5f, 0.6f, 0.9f);
		SkyIntensity = 0.15f; FogDensity = 0.12f; FogColor = FLinearColor(0.05f, 0.07f, 0.12f);
		break;
	}

	if (Sun)
	{
		if (UDirectionalLightComponent* C = Cast<UDirectionalLightComponent>(Sun->GetLightComponent()))
		{
			C->SetMobility(EComponentMobility::Movable);
			C->SetIntensity(SunIntensity);
			C->SetLightColor(SunColor);
		}
		Sun->SetActorRotation(FRotator(SunPitch, SunYaw, 0.f));
	}

	if (Sky)
	{
		if (USkyLightComponent* C = Sky->GetLightComponent())
		{
			C->SetMobility(EComponentMobility::Movable);
			C->SetIntensity(SkyIntensity);
			C->RecaptureSky(); // 太阳变了重捕环境光
		}
	}

	if (Fog)
	{
		// 叠加天气雾（H 块）：时间块基础雾 + 当前天气额外雾（雨/雾天更浓）。
		float FinalFogDensity = FogDensity;
		if (UGameInstance* GI = GetGameInstance())
		{
			if (UWeatherSubsystem* Weather = GI->GetSubsystem<UWeatherSubsystem>())
			{
				FinalFogDensity += Weather->GetWeatherFogDensity();
			}
		}
		if (UExponentialHeightFogComponent* C = Fog->GetComponent())
		{
			C->SetFogDensity(FinalFogDensity);
			C->SetFogInscatteringColor(FogColor);
		}
	}
}
