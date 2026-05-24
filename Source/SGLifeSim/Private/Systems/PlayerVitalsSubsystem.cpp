#include "Systems/PlayerVitalsSubsystem.h"
#include "Systems/PlayerVitalsSystem.h"
#include "Systems/EconomySubsystem.h"
#include "Systems/EconomyTypes.h"
#include "Systems/WantedSubsystem.h"
#include "Systems/SGAudioSubsystem.h"

#include "Engine/GameInstance.h"

void UPlayerVitalsSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	Health = FPlayerVitalsSystem::MaxHealth;
}

int32 UPlayerVitalsSubsystem::GetMaxHealth() const
{
	return FPlayerVitalsSystem::MaxHealth;
}

bool UPlayerVitalsSubsystem::IsDead() const
{
	return FPlayerVitalsSystem::IsDead(Health);
}

void UPlayerVitalsSubsystem::ApplyDamage(int32 Damage)
{
	if (FPlayerVitalsSystem::IsDead(Health)) { return; } // 已死，等重生流程
	Health = FPlayerVitalsSystem::ApplyDamage(Health, Damage);
	OnHealthChanged.Broadcast(Health, FPlayerVitalsSystem::MaxHealth);

	// 受伤音（E 块）。死亡音在 Die() 里。
	if (Damage > 0 && !FPlayerVitalsSystem::IsDead(Health))
	{
		if (UGameInstance* GI = GetGameInstance())
		{
			if (USGAudioSubsystem* Audio = GI->GetSubsystem<USGAudioSubsystem>())
			{
				Audio->PlayCue2D(ESGSound::PlayerHurt);
			}
		}
	}

	if (FPlayerVitalsSystem::IsDead(Health))
	{
		Die();
	}
}

void UPlayerVitalsSubsystem::Heal(int32 Amount)
{
	const int32 Old = Health;
	Health = FPlayerVitalsSystem::Heal(Health, Amount);
	if (Health != Old)
	{
		OnHealthChanged.Broadcast(Health, FPlayerVitalsSystem::MaxHealth);
	}
}

void UPlayerVitalsSubsystem::SetHealth(int32 NewHealth)
{
	Health = FPlayerVitalsSystem::Clamp(NewHealth);
	OnHealthChanged.Broadcast(Health, FPlayerVitalsSystem::MaxHealth);
}

void UPlayerVitalsSubsystem::Die()
{
	UGameInstance* GI = GetGameInstance();

	// 医院费：先扣现金，不够再从银行扣（尽力而为，扣多少不阻断重生）。
	if (GI)
	{
		if (UEconomySubsystem* Econ = GI->GetSubsystem<UEconomySubsystem>())
		{
			if (!Econ->TryWithdraw(ECurrencyAccount::Cash, HospitalFeeCents, TEXT("HospitalFee")))
			{
				Econ->TryWithdraw(ECurrencyAccount::Bank, HospitalFeeCents, TEXT("HospitalFee"));
			}
		}

		// 死了警察就不追了：清通缉。
		if (UWantedSubsystem* Wanted = GI->GetSubsystem<UWantedSubsystem>())
		{
			Wanted->ClearWanted();
		}

		// 死亡音（E 块）。
		if (USGAudioSubsystem* Audio = GI->GetSubsystem<USGAudioSubsystem>())
		{
			Audio->PlayCue2D(ESGSound::PlayerDeath);
		}
	}

	// 血量回满。
	Health = FPlayerVitalsSystem::MaxHealth;
	OnHealthChanged.Broadcast(Health, FPlayerVitalsSystem::MaxHealth);

	// 先广播死亡（玩家角色据此传送回出生点），再广播重生完成（弹气泡）。
	OnPlayerDied.Broadcast();
	OnPlayerRespawned.Broadcast(HospitalFeeCents);
}
