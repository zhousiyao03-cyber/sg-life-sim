#include "Systems/ResidencySubsystem.h"

bool UResidencySubsystem::ApplyForPR()
{
	const bool bOk = Residency.ApplyForPR();
	if (bOk) { OnResidencyChanged.Broadcast(Residency.GetStatus()); }
	return bOk;
}

bool UResidencySubsystem::ResolvePRApplication(bool bApproved)
{
	const bool bOk = Residency.ResolvePRApplication(bApproved);
	if (bOk) { OnResidencyChanged.Broadcast(Residency.GetStatus()); }
	return bOk;
}

bool UResidencySubsystem::Naturalize()
{
	const bool bOk = Residency.Naturalize();
	if (bOk) { OnResidencyChanged.Broadcast(Residency.GetStatus()); }
	return bOk;
}
