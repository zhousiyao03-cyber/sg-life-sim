#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"

#include "Systems/WeaponSystem.h"
#include "Systems/WeaponTypes.h"

#if WITH_DEV_AUTOMATION_TESTS

/**
 * 枪械纯核心（C 块 GTA）：徒手不能开火，有枪按弹匣开火/换弹，弹尽即停。
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FWeaponSystemTest,
	"SGLifeSim.Combat.WeaponSystem",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FWeaponSystemTest::RunTest(const FString& Parameters)
{
	using W = FWeaponSystem;

	// 徒手不能开火。
	TestFalse(TEXT("none can't fire kind"), W::CanFireKind(EWeaponKind::None));
	TestFalse(TEXT("none can't fire"), W::CanFire(EWeaponKind::None, 99));

	// 手枪参数合理。
	const FWeaponDef Pistol = W::GetDef(EWeaponKind::Pistol);
	TestEqual(TEXT("pistol mag 12"), Pistol.MagSize, 12);
	TestTrue(TEXT("pistol does damage"), Pistol.Damage > 0);
	TestTrue(TEXT("pistol raises wanted"), Pistol.WantedPerShot > 0);

	// 步枪比手枪强（伤害更高、弹匣更大）。
	const FWeaponDef Rifle = W::GetDef(EWeaponKind::Rifle);
	TestTrue(TEXT("rifle harder hitting"), Rifle.Damage > Pistol.Damage);
	TestTrue(TEXT("rifle bigger mag"), Rifle.MagSize > Pistol.MagSize);

	// 有弹能开火，没弹不能。
	TestTrue(TEXT("pistol with ammo fires"), W::CanFire(EWeaponKind::Pistol, 1));
	TestFalse(TEXT("pistol empty can't fire"), W::CanFire(EWeaponKind::Pistol, 0));

	// 开火消耗一发，打到 0 就停。
	int32 Ammo = W::Reload(EWeaponKind::Pistol);
	TestEqual(TEXT("reload fills mag"), Ammo, 12);
	int32 Shots = 0;
	while (W::CanFire(EWeaponKind::Pistol, Ammo))
	{
		Ammo = W::ConsumeRound(Ammo);
		++Shots;
	}
	TestEqual(TEXT("fired exactly mag size"), Shots, 12);
	TestEqual(TEXT("empty after"), Ammo, 0);

	// 换弹补满。
	Ammo = W::Reload(EWeaponKind::Pistol);
	TestEqual(TEXT("reload back to full"), Ammo, 12);

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
