

#include "Character/EnemyComponent/BossAIPauseComponent.h"

#include "AIController.h"
#include "Animation/AnimInstance.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BrainComponent.h"
#include "Character/WGasCharacterEnemy.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Navigation/PathFollowingComponent.h"

UBossAIPauseComponent::UBossAIPauseComponent()
{

	PrimaryComponentTick.bCanEverTick = false;

}

void UBossAIPauseComponent::Initialize(AWGasCharacterEnemy* InOwner, UBehaviorTree* InBehaviorTree)
{
	OwnerEnemy=InOwner;
	BehaviorTree=InBehaviorTree;
	CacheDefaultMovement();
}

void UBossAIPauseComponent::CacheDefaultMovement()
{
	AWGasCharacterEnemy* Enemy=OwnerEnemy.Get();
	if (!Enemy)return;

	bDefaultUseControllerRotationYaw=Enemy->bUseControllerRotationYaw;
	if (const UCharacterMovementComponent* Movement=Enemy->GetCharacterMovement())
	{
		CachedDefaultMaxWalkSpeed=Movement->MaxWalkSpeed;
		bDefaultUseControllerDesiredRotation=Movement->bUseControllerDesiredRotation;
		bDefaultOrientRotationToMovement=Movement->bOrientRotationToMovement;
		DefaultRotationRate=Movement->RotationRate;
	}
}

void UBossAIPauseComponent::StopMovement()
{
	AWGasCharacterEnemy* Enemy = OwnerEnemy.Get();
	if (!Enemy)return;
	if (AAIController* AIC = Cast<AAIController>(Enemy->GetController()))
	{
		AIC->StopMovement();
	}
	if (UCharacterMovementComponent* Movement = Enemy->GetCharacterMovement())
	{
		Movement->StopMovementImmediately();
	}
}

void UBossAIPauseComponent::CleanupPathAndFocus()
{
	AWGasCharacterEnemy* Enemy = OwnerEnemy.Get();
	if (!Enemy)return;
	if (AAIController* AIC=Cast<AAIController>(Enemy->GetController()))
	{
		if (UPathFollowingComponent* PathFollowing=AIC->GetPathFollowingComponent())
		{
			if (PathFollowing->GetStatus()!=EPathFollowingStatus::Idle)
			{
				PathFollowing->AbortMove(*Enemy,FPathFollowingResultFlags::UserAbort | FPathFollowingResultFlags::MovementStop);
			}
		}
		AIC->StopMovement();
		AIC->ClearFocus(EAIFocusPriority::Gameplay);
		AIC->ClearFocus(EAIFocusPriority::Move);
	}
	if (UCharacterMovementComponent* Movement=Enemy->GetCharacterMovement())
	{
		Movement->StopMovementImmediately();
		Movement->Velocity=FVector::ZeroVector;
	}
}

void UBossAIPauseComponent::ResetDefaultMovement()
{
	AWGasCharacterEnemy* Enemy = OwnerEnemy.Get();
	if (!Enemy)return;
	Enemy->bUseControllerRotationYaw = bDefaultUseControllerRotationYaw;
	if (UCharacterMovementComponent* Movement = Enemy->GetCharacterMovement())
	{
		Movement->bOrientRotationToMovement = bDefaultOrientRotationToMovement;
		Movement->bUseControllerDesiredRotation = bDefaultUseControllerDesiredRotation;
		Movement->RotationRate = DefaultRotationRate;
	}
}

bool UBossAIPauseComponent::PauseBrain(FName Reason)
{
	AWGasCharacterEnemy* Enemy = OwnerEnemy.Get();
	if (!Enemy)
	{
		return false;
	}
	if (AAIController* AIC = Cast<AAIController>(Enemy->GetController()))
	{
		AIC->StopMovement();
		AIC->ClearFocus(EAIFocusPriority::Gameplay);
		AIC->ClearFocus(EAIFocusPriority::Move);
	}
	if (ActiveBrainPauseReason != NAME_None)
	{
		return false;
	}
	if (AAIController* AIC = Cast<AAIController>(Enemy->GetController()))
	{
		if (UBrainComponent* Brain = AIC->BrainComponent)
		{
			Brain->StopLogic(Reason.ToString());
			ActiveBrainPauseReason = Reason;
			return true;
		}
	}
	return false;
}

void UBossAIPauseComponent::ResumeBrain(FName Reason, bool bRestartBehaviorTree)
{
	if (ActiveBrainPauseReason != Reason)return;
	AWGasCharacterEnemy* Enemy = OwnerEnemy.Get();
	if (!Enemy)
	{
		ActiveBrainPauseReason = NAME_None;
		return;
	}
	AAIController* AIC = Cast<AAIController>(Enemy->GetController());
	if (!AIC)
	{
		ActiveBrainPauseReason = NAME_None;
		return;
	}
	UBrainComponent* Brain = AIC->BrainComponent;
	if (Brain)
	{
		Brain->ResumeLogic(Reason.ToString());
	}

	if (bRestartBehaviorTree)
	{
		if (UBehaviorTreeComponent* BTComp=Cast<UBehaviorTreeComponent>(Brain))
		{
			if (UWorld* World=GetWorld())
			{
				TWeakObjectPtr<UBehaviorTreeComponent>WeakBTComp=BTComp;
				World->GetTimerManager().SetTimerForNextTick([WeakBTComp]()
				{
					if (WeakBTComp.IsValid())
					{
						WeakBTComp->RestartLogic();
					}
				});
			}
			else
			{
				BTComp->RestartLogic();
			}
		}
		else if (UBehaviorTree* BT = BehaviorTree.Get())
		{
			AIC->RunBehaviorTree(BT);
		}
	}
	ActiveBrainPauseReason = NAME_None;
}

bool UBossAIPauseComponent::IsBrainPaused(FName Reason) const
{
	return ActiveBrainPauseReason == Reason;
}

void UBossAIPauseComponent::DisableMovement()
{
	AWGasCharacterEnemy* Enemy = OwnerEnemy.Get();
	if (!Enemy)return;
	StopMovement();
	if (UCharacterMovementComponent* Movement=Enemy->GetCharacterMovement())
	{
		if (!bMovementDisabled)
		{
			CachedMovementMode=Movement->MovementMode;
			bMovementDisabled=true;
		}
		Movement->DisableMovement();
	}
	if (USkeletalMeshComponent* MeshComp = Enemy->GetMesh())
	{
		if (UAnimInstance* Anim = MeshComp->GetAnimInstance())
		{
			CachedRootMotionMode = Anim->RootMotionMode;
			bRootMotionModeCached = true;
			Anim->SetRootMotionMode(ERootMotionMode::IgnoreRootMotion);
		}
	}
}

void UBossAIPauseComponent::RestoreMovement()
{
	AWGasCharacterEnemy* Enemy = OwnerEnemy.Get();
	if (!Enemy)return;
	if (UCharacterMovementComponent* Movement=Enemy->GetCharacterMovement())
	{
		EMovementMode ModeToRestore=bMovementDisabled?CachedMovementMode.GetValue():MOVE_Walking;
		if (ModeToRestore == MOVE_None)
		{
			ModeToRestore = MOVE_Walking;
		}
		Movement->SetMovementMode(ModeToRestore);
		if (Movement->MaxWalkSpeed <= KINDA_SMALL_NUMBER)
		{
			Movement->MaxWalkSpeed = CachedDefaultMaxWalkSpeed;
		}
		bMovementDisabled = false;
	}
	if (USkeletalMeshComponent* MeshComp = Enemy->GetMesh())
	{
		if (UAnimInstance* Anim = MeshComp->GetAnimInstance())
		{
			if (bRootMotionModeCached)
			{
				Anim->SetRootMotionMode(CachedRootMotionMode);
			}
			else
			{
				Anim->SetRootMotionMode(ERootMotionMode::RootMotionFromMontagesOnly);
			}
			bRootMotionModeCached = false;
		}
	}
}

void UBossAIPauseComponent::SetBlackboardBool(FName Key, bool bValue)
{
	if (UBlackboardComponent* BB = GetBlackboard())
	{
		BB->SetValueAsBool(Key, bValue);
	}
}

void UBossAIPauseComponent::SetBlackboardInt(FName Key, int32 Value)
{
	if (UBlackboardComponent* BB = GetBlackboard())
	{
		BB->SetValueAsInt(Key, Value);
	}
}

UBlackboardComponent* UBossAIPauseComponent::GetBlackboard() const
{
	AWGasCharacterEnemy* Enemy = OwnerEnemy.Get();
	if (!Enemy)
	{
		return nullptr;
	}
	if (AAIController* AIC = Cast<AAIController>(Enemy->GetController()))
	{
		return AIC->GetBlackboardComponent();
	}
	return nullptr;
}

void UBossAIPauseComponent::ClearPauseReason(FName Reason)
{
	if (ActiveBrainPauseReason == Reason)
	{
		ActiveBrainPauseReason = NAME_None;
	}
}
