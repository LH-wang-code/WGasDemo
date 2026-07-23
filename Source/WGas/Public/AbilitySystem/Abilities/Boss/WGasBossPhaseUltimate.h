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
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

	UPROPERTY(EditDefaultsOnly, Category = "Phase Ultimate|Weapon")
	FName Phase2GreatswordSocket =TEXT("Socket_Greatsword_Phase2");
protected:
	virtual void FinishAttack(bool bWasCancelled) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

	// 时间轴：从 Montage 开始计算
	UPROPERTY(EditDefaultsOnly, Category = "Phase Ultimate|Timeline")
	float AscendDelay = 0.9f;

	UPROPERTY(EditDefaultsOnly, Category = "Phase Ultimate|Timeline")
	float SummonSwordDelay = 1.35f;

	UPROPERTY(EditDefaultsOnly, Category = "Phase Ultimate|Timeline")
	float TelegraphDelay = 1.65f;

	UPROPERTY(EditDefaultsOnly, Category = "Phase Ultimate|Timeline")
	float DiveDelay = 2.65f;

	// 移动参数
	UPROPERTY(EditDefaultsOnly, Category = "Phase Ultimate|Movement")
	float AscendHeight = 900.f;

	UPROPERTY(EditDefaultsOnly, Category = "Phase Ultimate|Movement")
	float AscendDuration = 0.4f;

	UPROPERTY(EditDefaultsOnly, Category = "Phase Ultimate|Movement")
	float DiveDuration = 0.55f;

	// 范围伤害；Damage / DamageType / DamageEffectClass 继续使用父类已有配置
	UPROPERTY(EditDefaultsOnly, Category = "Phase Ultimate|Impact")
	float ImpactRadius = 800.f;

	UPROPERTY(EditDefaultsOnly, Category = "Phase Ultimate|Impact")
	float ImpactPoiseDamage = 35.f;

	// 落地后替换到 Boss 原 Weapon 组件上的第二阶段大剑
	UPROPERTY(EditDefaultsOnly, Category = "Phase Ultimate|Weapon")
	TObjectPtr<USkeletalMesh> Phase2GreatswordMesh;

	// 这三个都可以是只有 Niagara / Decal、会自动销毁的蓝图 Actor
	UPROPERTY(EditDefaultsOnly, Category = "Phase Ultimate|VFX")
	TSubclassOf<AActor> SummonSwordVisualClass;

	UPROPERTY(EditDefaultsOnly, Category = "Phase Ultimate|VFX")
	TSubclassOf<AActor> ImpactTelegraphClass;

	UPROPERTY(EditDefaultsOnly, Category = "Phase Ultimate|VFX")
	TSubclassOf<AActor> ImpactVisualClass;

private:
	void StartPhaseSequence();

	void BeginAscend();
	void UpdateAscend();

	void SummonSecondWeapon();
	void ShowImpactTelegraph();

	void BeginDive();
	void UpdateDive();

	void TriggerImpact();
	void ApplyImpactDamage();

	void ClearPhaseSequence();

	FVector AscendStartLocation = FVector::ZeroVector;
	FVector AirLocation = FVector::ZeroVector;
	FVector DiveStartLocation = FVector::ZeroVector;
	FVector ImpactLocation = FVector::ZeroVector;

	float AscendElapsed = 0.f;
	float DiveElapsed = 0.f;

	bool bAscending = false;
	bool bDiving = false;
	bool bImpactTriggered = false;

	TWeakObjectPtr<AActor> ImpactTelegraphActor;

	FTimerHandle AscendStartTimer;
	FTimerHandle SummonSwordTimer;
	FTimerHandle TelegraphTimer;
	FTimerHandle DiveStartTimer;
	FTimerHandle AscendTickTimer;
	FTimerHandle DiveTickTimer;
};
