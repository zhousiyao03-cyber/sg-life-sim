#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Math/RandomStream.h"
#include "Systems/WeatherTypes.h"
#include "Systems/TimeBlock.h"
#include "WeatherSubsystem.generated.h"

/** 天气变化广播（HUD 显示 / DayNightController 调雾 / 降雨粒子开关）。 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWeatherChanged, EWeather, Weather);

/**
 * 天气系统壳（H 块 GTA）。包 FWeatherSystem 纯核心，跨关卡保留当前天气。
 *
 * 订阅时间推进，每过一天加权随机换一次天气（注入可复现 RandomStream）。
 * 不直接碰场景雾/光——只持有天气状态并广播；由 DayNightController 在打光时
 * 把天气雾叠加到时间块基础雾上（避免两个系统抢着写 fog）。
 */
UCLASS()
class SGLIFESIM_API UWeatherSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	UFUNCTION(BlueprintPure, Category = "SGLifeSim|Weather")
	EWeather GetWeather() const { return CurrentWeather; }

	/** 当前天气对应的额外雾密度（DayNightController 叠加用）。 */
	UFUNCTION(BlueprintPure, Category = "SGLifeSim|Weather")
	float GetWeatherFogDensity() const;

	UFUNCTION(BlueprintPure, Category = "SGLifeSim|Weather")
	bool IsRaining() const;

	/** 强制设天气（调试 / 剧情）。 */
	UFUNCTION(BlueprintCallable, Category = "SGLifeSim|Weather")
	void SetWeather(EWeather NewWeather);

	/** 掷一次新天气（每天换 / 手动）。 */
	UFUNCTION(BlueprintCallable, Category = "SGLifeSim|Weather")
	void RollNewWeather();

	UPROPERTY(BlueprintAssignable, Category = "SGLifeSim|Weather")
	FOnWeatherChanged OnWeatherChanged;

private:
	/** 订阅时间推进，跨天时换天气。 */
	UFUNCTION()
	void HandleTimeAdvanced(ETimeBlock NewBlock, int32 DayNumber);

	EWeather CurrentWeather = EWeather::Clear;
	int32 LastSeenDay = 1;
	FRandomStream WeatherStream;
};
