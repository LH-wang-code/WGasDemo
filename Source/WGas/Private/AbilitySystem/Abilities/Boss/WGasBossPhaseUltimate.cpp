#include "AbilitySystem/Abilities/Boss/WGasBossPhaseUltimate.h"

#include "Character/WGasCharacterEnemy.h"
#include "WGasGameplayTags.h"

UWGasBossPhaseUltimate::UWGasBossPhaseUltimate()
{
	// 父类 WGasBossMeleeAttack 会 Block PhaseTransition/Invulnerable，
	// 但转阶段大招正是在这两个 Tag 生效期间播放，必须允许激活。
	bInterruptibleByParry = false;
	const FWGasGameplayTags& Tags = FWGasGameplayTags::Get();
	if (Tags.State_Boss_PhaseTransition.IsValid())
	{
		ActivationBlockedTags.RemoveTag(Tags.State_Boss_PhaseTransition);
	}
	if (Tags.State_Boss_Invulnerable.IsValid())
	{
		ActivationBlockedTags.RemoveTag(Tags.State_Boss_Invulnerable);
	}
}

void UWGasBossPhaseUltimate::FinishAttack(bool bWasCancelled)
{
	if (AWGasCharacterEnemy* Enemy = Cast<AWGasCharacterEnemy>(GetAvatarActorFromActorInfo()))
	{
		Enemy->OnPhaseUltimateEnded();
	}

	Super::FinishAttack(bWasCancelled);
}

void UWGasBossPhaseUltimate::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	// Montage 未配置或激活失败时 FinishAttack 不会执行，在此兜底
	if (AWGasCharacterEnemy* Enemy = Cast<AWGasCharacterEnemy>(GetAvatarActorFromActorInfo()))
	{
		Enemy->OnPhaseUltimateEnded();
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
