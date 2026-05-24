#include "Systems/WeaponSubsystem.h"
#include "Systems/WeaponSystem.h"

int32 UWeaponSubsystem::GetMagSize() const
{
	return FWeaponSystem::GetDef(CurrentWeapon).MagSize;
}

bool UWeaponSubsystem::HasGun() const
{
	return FWeaponSystem::CanFireKind(CurrentWeapon);
}

void UWeaponSubsystem::EquipWeapon(EWeaponKind Kind)
{
	CurrentWeapon = Kind;
	AmmoInMag = FWeaponSystem::GetDef(Kind).MagSize; // 换枪满弹
	OnWeaponChanged.Broadcast(CurrentWeapon, AmmoInMag, GetMagSize());
}

void UWeaponSubsystem::Reload()
{
	if (!FWeaponSystem::CanFireKind(CurrentWeapon)) { return; }
	AmmoInMag = FWeaponSystem::Reload(CurrentWeapon);
	OnWeaponChanged.Broadcast(CurrentWeapon, AmmoInMag, GetMagSize());
}

void UWeaponSubsystem::RestoreFromSave(EWeaponKind Kind, int32 InAmmoInMag)
{
	CurrentWeapon = Kind;
	AmmoInMag = FMath::Clamp(InAmmoInMag, 0, FWeaponSystem::GetDef(Kind).MagSize);
	OnWeaponChanged.Broadcast(CurrentWeapon, AmmoInMag, GetMagSize());
}

bool UWeaponSubsystem::TryFire(int32& OutDamage, float& OutRange, int32& OutWantedPerShot)
{
	OutDamage = 0; OutRange = 0.f; OutWantedPerShot = 0;
	if (!FWeaponSystem::CanFire(CurrentWeapon, AmmoInMag))
	{
		return false;
	}

	const FWeaponDef Def = FWeaponSystem::GetDef(CurrentWeapon);
	OutDamage = Def.Damage;
	OutRange = Def.Range;
	OutWantedPerShot = Def.WantedPerShot;

	AmmoInMag = FWeaponSystem::ConsumeRound(AmmoInMag);
	OnWeaponChanged.Broadcast(CurrentWeapon, AmmoInMag, GetMagSize());
	return true;
}
