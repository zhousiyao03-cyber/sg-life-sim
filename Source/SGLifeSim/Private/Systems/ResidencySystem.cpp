#include "Systems/ResidencySystem.h"

bool FResidencySystem::IsOnWorkPermit() const
{
	return Status == EResidencyStatus::WorkPermit_EP
		|| Status == EResidencyStatus::WorkPermit_SP;
}

bool FResidencySystem::ApplyForPR()
{
	if (!IsOnWorkPermit())
	{
		return false;
	}
	PriorPermit = Status;
	Status = EResidencyStatus::PR_Applying;
	return true;
}

bool FResidencySystem::ResolvePRApplication(bool bApproved)
{
	if (Status != EResidencyStatus::PR_Applying)
	{
		return false;
	}
	if (bApproved)
	{
		Status = EResidencyStatus::PR;
	}
	else
	{
		Status = PriorPermit;  // 退回申请前的工作准证
		++PRRejectionCount;
	}
	return true;
}

bool FResidencySystem::Naturalize()
{
	if (Status != EResidencyStatus::PR)
	{
		return false;
	}
	Status = EResidencyStatus::Citizen;
	return true;
}

void FResidencySystem::RestoreState(EResidencyStatus InStatus, int32 InRejections)
{
	Status = InStatus;
	PriorPermit = EResidencyStatus::WorkPermit_EP;
	PRRejectionCount = FMath::Max(0, InRejections);
}
