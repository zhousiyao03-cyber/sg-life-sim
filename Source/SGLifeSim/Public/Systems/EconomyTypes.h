#pragma once

#include "CoreMinimal.h"
#include "EconomyTypes.generated.h"

/**
 * 资金账户。spec §6.2。
 *
 * 新加坡特色：除现金/银行外，CPF（公积金）分三个子账户：
 * - OA (Ordinary Account)   普通户，可买房/投资
 * - SA (Special Account)    特别户，养老/退休
 * - MA (MediSave Account)   医疗户
 * 用 uint8 是因为 UEnum + UPROPERTY 需要（后续 Subsystem/存档要暴露给 BP）。
 */
UENUM(BlueprintType)
enum class ECurrencyAccount : uint8
{
	Cash    UMETA(DisplayName = "现金"),
	Bank    UMETA(DisplayName = "银行"),
	CPF_OA  UMETA(DisplayName = "公积金-普通户"),
	CPF_SA  UMETA(DisplayName = "公积金-特别户"),
	CPF_MA  UMETA(DisplayName = "公积金-医疗户"),

	Count   UMETA(Hidden)  // 账户总数，用于遍历，不在 UI 显示
};

/**
 * 一笔资金流水。spec §6.2（记账 / 存档 / UI 用）。
 *
 * 金额以「分」(cents) 存（int64），避免浮点累积误差；显示时再除以 100。
 * 正数 = 入账，负数 = 出账。
 */
USTRUCT(BlueprintType)
struct FMoneyTransaction
{
	GENERATED_BODY()

	/** 影响的账户。 */
	UPROPERTY(BlueprintReadOnly, Category = "Economy")
	ECurrencyAccount Account = ECurrencyAccount::Cash;

	/** 金额（分）。正入账，负出账。 */
	UPROPERTY(BlueprintReadOnly, Category = "Economy")
	int64 AmountCents = 0;

	/** 流水原因标签（如 "Salary" / "Rent" / "CPF" / "Withdraw"）。 */
	UPROPERTY(BlueprintReadOnly, Category = "Economy")
	FName Reason;

	FMoneyTransaction() = default;

	FMoneyTransaction(ECurrencyAccount InAccount, int64 InAmountCents, FName InReason)
		: Account(InAccount)
		, AmountCents(InAmountCents)
		, Reason(InReason)
	{
	}
};
