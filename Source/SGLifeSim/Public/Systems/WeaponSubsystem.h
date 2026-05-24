#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Systems/WeaponTypes.h"
#include "WeaponSubsystem.generated.h"

/** 弹药/武器变化广播（HUD 显示「手枪 9/12」）。 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnWeaponChanged, EWeaponKind, Kind, int32, AmmoInMag, int32, MagSize);

/**
 * 武器系统壳（C 块 GTA）。包 FWeaponSystem 纯核心，跨关卡保留当前武器与弹匣。
 *
 * 玩家默认徒手。捡到/买到枪 → EquipWeapon。开火由玩家角色做射线命中，
 * 命中后调本系统 TryFire 判定弹药、扣子弹；伤害与涨通缉的数值由这里给出。
 */
UCLASS()
class SGLIFESIM_API UWeaponSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "SGLifeSim|Weapon")
	EWeaponKind GetWeapon() const { return CurrentWeapon; }

	UFUNCTION(BlueprintPure, Category = "SGLifeSim|Weapon")
	int32 GetAmmoInMag() const { return AmmoInMag; }

	UFUNCTION(BlueprintPure, Category = "SGLifeSim|Weapon")
	int32 GetMagSize() const;

	UFUNCTION(BlueprintPure, Category = "SGLifeSim|Weapon")
	bool HasGun() const;

	/** 装备武器（换枪即满弹）。 */
	UFUNCTION(BlueprintCallable, Category = "SGLifeSim|Weapon")
	void EquipWeapon(EWeaponKind Kind);

	/** 换弹（补满弹匣）。 */
	UFUNCTION(BlueprintCallable, Category = "SGLifeSim|Weapon")
	void Reload();

	/** 读档恢复武器与弹匣。 */
	UFUNCTION(BlueprintCallable, Category = "SGLifeSim|Weapon")
	void RestoreFromSave(EWeaponKind Kind, int32 InAmmoInMag);

	/**
	 * 扣动扳机：能开火则消耗一发并返回 true（玩家角色据此做射线+命中伤害）；
	 * 没枪或弹匣空返回 false。OutDamage 填本枪每发伤害，OutRange 填射程。
	 */
	bool TryFire(int32& OutDamage, float& OutRange, int32& OutWantedPerShot);

	UPROPERTY(BlueprintAssignable, Category = "SGLifeSim|Weapon")
	FOnWeaponChanged OnWeaponChanged;

private:
	EWeaponKind CurrentWeapon = EWeaponKind::None;
	int32 AmmoInMag = 0;
};
