#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/Boss/WGasBossMeleeAttack.h"
#include "WGasBossPhaseUltimate.generated.h"

UCLASS()
class WGAS_API UWGasBossPhaseUltimate : public UWGasBossMeleeAttack
{
	GENERATED_BODY()

public:
	UWGasBossPhaseUltimate();

protected:
	virtual void FinishAttack(bool bWasCancelled) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
};
