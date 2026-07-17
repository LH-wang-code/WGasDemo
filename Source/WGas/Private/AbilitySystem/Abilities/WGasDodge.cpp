// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/Abilities/WGasDodge.h"

#include "Animation/AnimInstance.h"
#include "Animation/WGasAnimLayerInterface.h"
#include "Character/WGasCharacterBase.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "TimerManager.h"
#include "WGasGameplayTags.h"

UWGasDodge::UWGasDodge()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	const FWGasGameplayTags& Tags = FWGasGameplayTags::Get();
	if (Tags.State_Dodge.IsValid())
	{
		ActivationBlockedTags.AddTag(Tags.State_Dodge);
	}
	if (Tags.State_Attacking_Lighting.IsValid())
	{
		ActivationBlockedTags.AddTag(Tags.State_Attacking_Lighting);
	}
}

FVector UWGasDodge::GetDodgeDirection(const AWGasCharacterBase* Character) const
{
	if (!Character)
	{
		return FVector::ForwardVector;
	}

	FVector DodgeDir = FVector::ZeroVector;

	if (const UCharacterMovementComponent* MoveComp = Character->GetCharacterMovement())
	{
		DodgeDir = MoveComp->GetPendingInputVector();
		if (DodgeDir.IsNearlyZero())
		{
			DodgeDir = MoveComp->GetLastInputVector();
		}
	}

	if (DodgeDir.IsNearlyZero())
	{
		DodgeDir = Character->GetLastMovementInputVector();
	}

	DodgeDir.Z = 0.f;

	if (!DodgeDir.Normalize())
	{
		DodgeDir = -Character->GetActorForwardVector();
		DodgeDir.Z = 0.f;
		DodgeDir.Normalize();
	}

	return DodgeDir;
}

void UWGasDodge::ApplyDodgeMovementBoost(AWGasCharacterBase* Character, const FVector& DodgeDir)
{
	if (!Character || bDodgeMovementBoostApplied || !bBoostDodgeMovementSpeed)
	{
		return;
	}

	CachedDodgeDirection = DodgeDir;
	bDodgeMovementBoostApplied = true;

	if (UCharacterMovementComponent* Movement = Character->GetCharacterMovement())
	{
		CachedMaxWalkSpeed = Movement->MaxWalkSpeed;
		Movement->MaxWalkSpeed = DodgeMaxWalkSpeed;
	}

	if (bApplyDodgeDirectionInput)
	{
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().SetTimer(
				DodgeMovementTickHandle,
				this,
				&UWGasDodge::TickDodgeMovement,
				0.016f,
				true);
		}
	}
}

void UWGasDodge::RestoreDodgeMovementBoost(AWGasCharacterBase* Character)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(DodgeMovementTickHandle);
	}

	if (!Character || !bDodgeMovementBoostApplied)
	{
		return;
	}

	if (UCharacterMovementComponent* Movement = Character->GetCharacterMovement())
	{
		Movement->MaxWalkSpeed = CachedMaxWalkSpeed;

		if (bStopVelocityOnDodgeEnd && Movement->IsMovingOnGround())
		{
			Movement->StopMovementImmediately();
		}
	}

	bDodgeMovementBoostApplied = false;
	CachedDodgeDirection = FVector::ZeroVector;
	CachedMaxWalkSpeed = 0.f;
}

void UWGasDodge::EnableDodgeRootMotion(AWGasCharacterBase* Character)
{
	if (!bUseDodgeAnimationRootMotion || bRootMotionModeCached || !Character)
	{
		return;
	}

	USkeletalMeshComponent* Mesh = Character->GetMesh();
	if (!Mesh)
	{
		return;
	}

	if (UAnimInstance* Anim = Mesh->GetAnimInstance())
	{
		CachedRootMotionMode = Anim->RootMotionMode;
		bRootMotionModeCached = true;
		Anim->SetRootMotionMode(ERootMotionMode::RootMotionFromEverything);
	}
}

void UWGasDodge::RestoreDodgeRootMotion(AWGasCharacterBase* Character)
{
	if (!bRootMotionModeCached)
	{
		return;
	}

	if (Character)
	{
		if (USkeletalMeshComponent* Mesh = Character->GetMesh())
		{
			if (UAnimInstance* Anim = Mesh->GetAnimInstance())
			{
				Anim->SetRootMotionMode(CachedRootMotionMode);
			}
		}
	}

	bRootMotionModeCached = false;
	CachedRootMotionMode = ERootMotionMode::RootMotionFromMontagesOnly;
}

void UWGasDodge::TickDodgeMovement()
{
	if (bDodgeEndHandled)
	{
		return;
	}

	if (AWGasCharacterBase* Character = GetWGasCharacterFromActorInfo())
	{
		if (!CachedDodgeDirection.IsNearlyZero())
		{
			Character->AddMovementInput(CachedDodgeDirection, 1.f);
		}
	}
}

void UWGasDodge::FinishDodge()
{
	if (bDodgeEndHandled)
	{
		return;
	}
	bDodgeEndHandled = true;

	AWGasCharacterBase* Character = GetWGasCharacterFromActorInfo();
	SetDodgeAnimationStateOnCharacter(Character, false);
	RestoreDodgeMovementBoost(Character);
	RestoreDodgeRootMotion(Character);

	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
	{
		if (FWGasGameplayTags::Get().State_Dodge.IsValid())
		{
			ASC->RemoveLooseGameplayTag(FWGasGameplayTags::Get().State_Dodge);
		}
	}

	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, false);
}

void UWGasDodge::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const bool bReplicateEndAbility,
	const bool bWasCancelled)
{
	// Cancellation can bypass the AnimBP callback. Always return the AnimInstance
	// to its previous root-motion mode and clear the movement override.
	if (!bDodgeEndHandled)
	{
		bDodgeEndHandled = true;

		AWGasCharacterBase* Character = GetWGasCharacterFromActorInfo();
		SetDodgeAnimationStateOnCharacter(Character, false);
		RestoreDodgeMovementBoost(Character);
		RestoreDodgeRootMotion(Character);

		if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
		{
			const FGameplayTag& DodgeTag = FWGasGameplayTags::Get().State_Dodge;
			if (DodgeTag.IsValid())
			{
				ASC->RemoveLooseGameplayTag(DodgeTag);
			}
		}
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UWGasDodge::EndDodgeActivePhase()
{
	FinishDodge();
}

void UWGasDodge::EndDodgeMovementPhase()
{
	RestoreDodgeMovementBoost(GetWGasCharacterFromActorInfo());
}

void UWGasDodge::SetDodgeAnimationStateOnCharacter(AWGasCharacterBase* Character, const bool bIsDodging) const
{
	if (!Character)
	{
		return;
	}

	USkeletalMeshComponent* Mesh = Character->GetMesh();
	if (!Mesh)
	{
		return;
	}

	UAnimInstance* Anim = Mesh->GetAnimInstance();
	if (!Anim || !Anim->GetClass()->ImplementsInterface(UWGasAnimLayerInterface::StaticClass()))
	{
		return;
	}

	IWGasAnimLayerInterface::Execute_SetDodgeAnimationState(Anim, bIsDodging);
}

void UWGasDodge::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	AWGasCharacterBase* Character = GetWGasCharacterFromActorInfo();
	if (!Character)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	bDodgeEndHandled = false;

	const FVector DodgeDir = GetDodgeDirection(Character);

	if (UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get())
	{
		if (FWGasGameplayTags::Get().State_Dodge.IsValid())
		{
			ASC->AddLooseGameplayTag(FWGasGameplayTags::Get().State_Dodge);
		}
	}

	EnableDodgeRootMotion(Character);
	if (!bUseDodgeAnimationRootMotion)
	{
		ApplyDodgeMovementBoost(Character, DodgeDir);
	}
	SetDodgeAnimationStateOnCharacter(Character, true);
}
