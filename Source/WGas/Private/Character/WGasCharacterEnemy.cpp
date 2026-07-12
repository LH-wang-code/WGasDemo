// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/WGasCharacterEnemy.h"

#include "AbilitySystem/WGasAbilitySystemFunctionLibrary.h"
#include "AbilitySystem/WGasAttributeSet.h"
#include "AI/WGasAIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "MotionWarpingComponent.h"
#include "WGasGameplayTags.h"
#include "AbilitySystem/WGasAbilitySystemComponent.h"
#include "Animation/AnimInstance.h"
#include "AIController.h"
#include "BrainComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
AWGasCharacterEnemy::AWGasCharacterEnemy()
{
	MotionWarpingComponent = CreateDefaultSubobject<UMotionWarpingComponent>(TEXT("MotionWarpingComponent"));

	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->bOrientRotationToMovement = false;
		Movement->bUseControllerDesiredRotation = false;
		Movement->RotationRate = FRotator(0.f, 720.f, 0.f);
	}
}

void AWGasCharacterEnemy::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	WGasAIController = Cast<AWGasAIController>(NewController);
	if (WGasAIController && BehaviorTree)
	{
		WGasAIController->RunBehaviorTree(BehaviorTree);
		AddCharacterAbilities();
	}
}

void AWGasCharacterEnemy::BeginPlay()
{
	Super::BeginPlay();
	if (const UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		CachedDefaultMaxWalkSpeed = Movement->MaxWalkSpeed;
	}
	InitAbilityActorInfo();
}

void AWGasCharacterEnemy::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!bPoiseBrokenRotationLocked)
	{
		return;
	}

	const FRotator LockedRotation(CachedPoiseBrokenRotation.Pitch, CachedPoiseBrokenRotation.Yaw, CachedPoiseBrokenRotation.Roll);
	if (!GetActorRotation().Equals(LockedRotation, 0.5f))
	{
		SetActorRotation(LockedRotation);
	}

	if (AAIController* AIC = Cast<AAIController>(GetController()))
	{
		AIC->SetControlRotation(LockedRotation);
		AIC->ClearFocus(EAIFocusPriority::Gameplay);
		AIC->ClearFocus(EAIFocusPriority::Move);
	}
}

void AWGasCharacterEnemy::InitAbilityActorInfo()
{
	AbilitySystemComponent->InitAbilityActorInfo(this, this);
	InitializeDefaultAttributes();
	if (const UWGasAttributeSet* AS = Cast<UWGasAttributeSet>(AttributeSet))
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] Init Attr | HP: %.1f/%.1f  Mana: %.1f/%.1f  Poise: %.1f/%.1f  Stamina: %.1f/%.1f"),
			*GetName(),
			AS->GetHealth(), AS->GetMaxHealth(),
			AS->GetMana(), AS->GetMaxMana(),
			AS->GetPoise(), AS->GetMaxPoise(),
			AS->GetStamina(), AS->GetMaxStamina());
	}
	BindPoiseBrokenDelegates();
}

void AWGasCharacterEnemy::InitializeDefaultAttributes() const
{
	UWGasAbilitySystemFunctionLibrary::InitializeDefaultCharacterClassInfo(this, CharacterClass, Level, AbilitySystemComponent);
}

USkeletalMeshComponent* AWGasCharacterEnemy::GetWeaponTraceMesh() const
{
	if (Weapon && Weapon->GetSkinnedAsset())
	{
		return Weapon;
	}
	return GetMesh();
}

UBlackboardComponent* AWGasCharacterEnemy::GetBossBlackboard()
{
	if (AAIController* AIC = Cast<AAIController>(GetController()))
	{
		return AIC->GetBlackboardComponent();
	}
	return nullptr;
}

void AWGasCharacterEnemy::SetPoiseBrokenBlackboard(bool bBroken)
{
	if (UBlackboardComponent* BB = GetBossBlackboard())
	{
		BB->SetValueAsBool(PoiseBrokenBlackboardKey, bBroken);
		UE_LOG(LogTemp, Warning, TEXT("[%s] Blackboard %s = %s"),
			*GetName(), *PoiseBrokenBlackboardKey.ToString(), bBroken ? TEXT("true") : TEXT("false"));
		return;
	}
	UE_LOG(LogTemp, Error, TEXT("[%s] Failed to set %s: no AIController/Blackboard"),
		*GetName(), *PoiseBrokenBlackboardKey.ToString());
}

void AWGasCharacterEnemy::StopBossMovement()
{
	if (AAIController* AIC = Cast<AAIController>(GetController()))
	{
		AIC->StopMovement();
	}
	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->StopMovementImmediately();
	}
}

void AWGasCharacterEnemy::LockBossMovementForPoiseBroken()
{
	StopBossMovement();
	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		if (!bMovementLockedForPoise)
		{
			CachedMovementMode = Movement->MovementMode;
			bMovementLockedForPoise = true;
		}
		Movement->DisableMovement();
	}
	if (USkeletalMeshComponent* MeshComp = GetMesh())
	{
		if (UAnimInstance* Anim = MeshComp->GetAnimInstance())
		{
			Anim->SetRootMotionMode(ERootMotionMode::IgnoreRootMotion);
		}
	}
}

void AWGasCharacterEnemy::UnlockBossMovementForPoiseBroken()
{
	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		EMovementMode ModeToRestore = bMovementLockedForPoise ? CachedMovementMode.GetValue() : MOVE_Walking;
		if (ModeToRestore == MOVE_None)
		{
			ModeToRestore = MOVE_Walking;
		}
		Movement->SetMovementMode(ModeToRestore);
		if (Movement->MaxWalkSpeed <= KINDA_SMALL_NUMBER)
		{
			Movement->MaxWalkSpeed = CachedDefaultMaxWalkSpeed;
		}
		bMovementLockedForPoise = false;
	}
	if (USkeletalMeshComponent* MeshComp = GetMesh())
	{
		if (UAnimInstance* Anim = MeshComp->GetAnimInstance())
		{
			Anim->SetRootMotionMode(ERootMotionMode::RootMotionFromEverything);
		}
	}
}

void AWGasCharacterEnemy::PauseBossBrainForPoiseBroken()
{
	if (AAIController* AIC = Cast<AAIController>(GetController()))
	{
		AIC->StopMovement();
		AIC->ClearFocus(EAIFocusPriority::Gameplay);
		AIC->ClearFocus(EAIFocusPriority::Move);
	}

	if (bBrainPausedForPoise)
	{
		return;
	}
	if (AAIController* AIC = Cast<AAIController>(GetController()))
	{
		if (UBrainComponent* Brain = AIC->BrainComponent)
		{
			Brain->StopLogic(TEXT("PoiseBroken"));
			bBrainPausedForPoise = true;
		}
	}
}

void AWGasCharacterEnemy::LockBossRotationForPoiseBroken()
{
	CachedPoiseBrokenRotation = GetActorRotation();
	bPoiseBrokenRotationLocked = true;

	if (!bRotationLockedForPoise)
	{
		bCachedUseControllerRotationYawForPoise = bUseControllerRotationYaw;
		bRotationLockedForPoise = true;
	}
	bUseControllerRotationYaw = false;

	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		bCachedUseControllerDesiredRotationForPoise = Movement->bUseControllerDesiredRotation;
		bCachedOrientRotationToMovementForPoise = Movement->bOrientRotationToMovement;
		CachedRotationRateForPoise = Movement->RotationRate;
		Movement->bUseControllerDesiredRotation = false;
		Movement->bOrientRotationToMovement = false;
		Movement->RotationRate = FRotator::ZeroRotator;
	}

	if (AAIController* AIC = Cast<AAIController>(GetController()))
	{
		AIC->SetControlRotation(CachedPoiseBrokenRotation);
	}
}

void AWGasCharacterEnemy::UnlockBossRotationForPoiseBroken()
{
	bPoiseBrokenRotationLocked = false;

	if (bRotationLockedForPoise)
	{
		bUseControllerRotationYaw = bCachedUseControllerRotationYawForPoise;
		bRotationLockedForPoise = false;
	}

	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->bUseControllerDesiredRotation = bCachedUseControllerDesiredRotationForPoise;
		Movement->bOrientRotationToMovement = bCachedOrientRotationToMovementForPoise;
		Movement->RotationRate = CachedRotationRateForPoise;
	}
}

void AWGasCharacterEnemy::ResumeBossBrainForPoiseBroken()
{
	if (AAIController* AIC = Cast<AAIController>(GetController()))
	{
		AIC->StopMovement();

		if (UBrainComponent* Brain = AIC->BrainComponent)
		{
			Brain->ResumeLogic(TEXT("PoiseBroken"));
			if (UBehaviorTreeComponent* BTComp = Cast<UBehaviorTreeComponent>(Brain))
			{
				if (!BTComp->IsRunning())
				{
					BTComp->RestartLogic();
				}
			}
		}
		else if (BehaviorTree)
		{
			AIC->RunBehaviorTree(BehaviorTree);
		}
	}
	bBrainPausedForPoise = false;
}

void AWGasCharacterEnemy::EnterPoiseBroken()
{
	UWGasAbilitySystemComponent* ASC = Cast<UWGasAbilitySystemComponent>(AbilitySystemComponent);
	if (!ASC)
	{
		return;
	}
	const FWGasGameplayTags& WGasTags = FWGasGameplayTags::Get();
	if (WGasTags.State_Boss_PoiseBroken.IsValid()
		&& ASC->HasMatchingGameplayTag(WGasTags.State_Boss_PoiseBroken))
	{
		return;
	}
	if (WGasTags.State_Boss_Attacking.IsValid())
	{
		FGameplayTagContainer CancelTags;
		CancelTags.AddTag(WGasTags.State_Boss_Attacking);
		ASC->CancelAbilities(&CancelTags);
	}
	if (CombatComponent)
	{
		CombatComponent->EndWeaponSweep();
	}
	StopBossMovement();
	PauseBossBrainForPoiseBroken();
	LockBossMovementForPoiseBroken();
	LockBossRotationForPoiseBroken();
	if (USkeletalMeshComponent* MeshComp = GetMesh())
	{
		if (UAnimInstance* AnimInstance = MeshComp->GetAnimInstance())
		{
			AnimInstance->Montage_Stop(0.2f);
		}
	}
	if (PoiseBrokenEffectClass)
	{
		FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
		Context.AddSourceObject(this);
		const FGameplayEffectSpecHandle SpecHandle =
			ASC->MakeOutgoingSpec(PoiseBrokenEffectClass, 1.f, Context);
		PoiseBrokenEffectHandle = ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
	}
	if (PoiseRegenEffectClass)
	{
		FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
		Context.AddSourceObject(this);
		const FGameplayEffectSpecHandle SpecHandle =
			ASC->MakeOutgoingSpec(PoiseRegenEffectClass, 1.f, Context);
		PoiseRegenEffectHandle = ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
	}
	if (PoiseBrokenMontage)
	{
		PlayAnimMontage(PoiseBrokenMontage);
		if (USkeletalMeshComponent* MeshComp = GetMesh())
		{
			if (UAnimInstance* Anim = MeshComp->GetAnimInstance())
			{
				FOnMontageEnded EndDelegate;
				EndDelegate.BindUObject(this, &AWGasCharacterEnemy::OnPoiseBrokenMontageEnded);
				Anim->Montage_SetEndDelegate(EndDelegate, PoiseBrokenMontage);
			}
		}
	}
	SetPoiseBrokenBlackboard(true);
	bPoiseExitPending = false;
}

void AWGasCharacterEnemy::ExitPoiseBroken()
{
	UWGasAbilitySystemComponent* ASC = Cast<UWGasAbilitySystemComponent>(AbilitySystemComponent);
	if (ASC)
	{
		if (PoiseBrokenEffectHandle.IsValid())
		{
			ASC->RemoveActiveGameplayEffect(PoiseBrokenEffectHandle);
			PoiseBrokenEffectHandle.Invalidate();
		}
		if (PoiseRegenEffectHandle.IsValid())
		{
			ASC->RemoveActiveGameplayEffect(PoiseRegenEffectHandle);
			PoiseRegenEffectHandle.Invalidate();
		}
		const FGameplayTag& PoiseBrokenTag = FWGasGameplayTags::Get().State_Boss_PoiseBroken;
		if (PoiseBrokenTag.IsValid())
		{
			FGameplayTagContainer WGasTags;
			WGasTags.AddTag(PoiseBrokenTag);
			ASC->RemoveActiveEffectsWithGrantedTags(WGasTags);
		}
	}

	SetPoiseBrokenBlackboard(false);
	UnlockBossRotationForPoiseBroken();
	UnlockBossMovementForPoiseBroken();
	ResumeBossBrainForPoiseBroken();
}

void AWGasCharacterEnemy::BindPoiseBrokenDelegates()
{
	if (bPoiseBrokenDelegatesBound)
	{
		return;
	}
	if (UWGasAttributeSet* AS = Cast<UWGasAttributeSet>(AttributeSet))
	{
		AS->OnPoiseBroken.AddUObject(this, &AWGasCharacterEnemy::EnterPoiseBroken);
		AS->OnPoiseRecovered.AddUObject(this, &AWGasCharacterEnemy::RequestPoiseBrokenExit);
		bPoiseBrokenDelegatesBound = true;
	}
}

void AWGasCharacterEnemy::RequestPoiseBrokenExit()
{
	if (bPoiseExitPending)
	{
		return;
	}
	bPoiseExitPending = true;
	UWGasAbilitySystemComponent* ASC = Cast<UWGasAbilitySystemComponent>(AbilitySystemComponent);
	if (ASC && PoiseRegenEffectHandle.IsValid())
	{
		ASC->RemoveActiveGameplayEffect(PoiseRegenEffectHandle);
		PoiseRegenEffectHandle.Invalidate();
	}
	USkeletalMeshComponent* MeshComp = GetMesh();
	UAnimInstance* Anim = MeshComp ? MeshComp->GetAnimInstance() : nullptr;
	if (Anim && PoiseBrokenMontage && Anim->Montage_IsPlaying(PoiseBrokenMontage))
	{
		Anim->Montage_JumpToSection(PoiseBrokenEndSection, PoiseBrokenMontage);
		return;
	}
	bPoiseExitPending = false;
	ExitPoiseBroken();
}

void AWGasCharacterEnemy::OnPoiseBrokenMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (Montage != PoiseBrokenMontage)
	{
		return;
	}

	// 等 End 段自然播完再解锁；JumpToSection 触发的 Interrupted 不能提前 Exit
	if (bInterrupted)
	{
		return;
	}

	bPoiseExitPending = false;
	ExitPoiseBroken();
}
