// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/WGasCharacterHero.h"

#include "MotionWarpingComponent.h"
#include "WGasGameplayTags.h"
#include "AbilitySystem/WGasAbilitySystemComponent.h"
#include "AbilitySystem/WGasAbilitySystemFunctionLibrary.h"
#include "AbilitySystem/WGasAttributeSet.h"
#include "AbilitySystem/Abilities/WGasMeleeAttack.h"
#include "Animation/WGasAnimLayerInterface.h"
#include "Camera/CameraComponent.h"
#include "Character/WGasLockOnComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Player/WGasPlayerController.h"
#include "UI/HUD/WGasHUD.h"

AWGasCharacterHero::AWGasCharacterHero()
{
	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>("SpringArmComponent");
	SpringArmComponent->SetupAttachment(GetRootComponent());
	SpringArmComponent->TargetArmLength = 400.f;
	SpringArmComponent->bUsePawnControlRotation = true;
	SpringArmComponent->bInheritPitch = true;
	SpringArmComponent->bInheritYaw = true;
	SpringArmComponent->bInheritRoll = false;
	SpringArmComponent->SetUsingAbsoluteRotation(false);
	SpringArmComponent->bDoCollisionTest = false;
	SpringArmComponent->TargetOffset = FVector(0.f, 0.f, 100.f); 
	CameraComponent = CreateDefaultSubobject<UCameraComponent>("CameraComponent");
	CameraComponent->SetupAttachment(SpringArmComponent, USpringArmComponent::SocketName);
	CameraComponent->bUsePawnControlRotation = false;


	LockOnComponent=CreateDefaultSubobject<UWGasLockOnComponent>("LockOnComponent");

	InputBufferComponent = CreateDefaultSubobject<UGasInputBufferComponent>("InputBufferComponent");
	bUseControllerRotationYaw = false;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;

	UCharacterMovementComponent* Movement = GetCharacterMovement();
	Movement->bOrientRotationToMovement = false;
	Movement->bUseControllerDesiredRotation = false;

	MotionWarpingComponent = CreateDefaultSubobject<UMotionWarpingComponent>(TEXT("MotionWarpingComponent"));
	
	ApplyMovementSpeed();
}

void AWGasCharacterHero::ToggleWalkRun()
{
	if (!bIsRunning)
	{
		if (const UWGasAttributeSet* AS = Cast<UWGasAttributeSet>(AttributeSet))
		{
			if (AS->GetStamina() <= 0.f)
			{
				return; // 体力不够，不切跑
			}
		}
	}
	bIsRunning = !bIsRunning;
	ApplyMovementSpeed();
	if (!bIsRunning)
	{
		ClearRunningTag();
	}
}

void AWGasCharacterHero::ForceWalk()
{
	if (!bIsRunning)
	{
		return; 
	}
	bIsRunning = false;
	ApplyMovementSpeed();
	ClearRunningTag();
}

void AWGasCharacterHero::UpdateRunningTag(const FVector2D& MoveInput)
{
	UWGasAbilitySystemComponent* ASC = Cast<UWGasAbilitySystemComponent>(AbilitySystemComponent);
	if (!ASC)
	{
		return;
	}

	const UWGasAttributeSet* AS = Cast<UWGasAttributeSet>(AttributeSet);
	if (bIsRunning && AS && AS->GetStamina() <= 0.f)
	{
		ForceWalk();
		return;
	}

	const bool bShouldConsumeStamina = bIsRunning
		&& !MoveInput.IsNearlyZero()
		&& AS
		&& AS->GetStamina() > 0.f;
	const FGameplayTag& RunningTag = FWGasGameplayTags::Get().State_Running;
	const bool bHasRunningTag = ASC->HasMatchingGameplayTag(RunningTag);

	if (bShouldConsumeStamina)
	{
		if (!bHasRunningTag)
		{
			ASC->AddLooseGameplayTag(RunningTag);
		}
	}
	else if (bHasRunningTag)
	{
		ClearRunningTag();
	}
}

void AWGasCharacterHero::ClearRunningTag()
{
	if (UWGasAbilitySystemComponent* ASC = Cast<UWGasAbilitySystemComponent>(AbilitySystemComponent))
	{
		const FGameplayTag& RunningTag = FWGasGameplayTags::Get().State_Running;
		ASC->SetLooseGameplayTagCount(RunningTag, 0);
	}
}

void AWGasCharacterHero::OnDodgeIFrameSuccess_Implementation()
{
	GrantMomentum(MomentumGainOnPerfectDodge);
}

void AWGasCharacterHero::GrantMomentum(float Amount)
{
	if (AbilitySystemComponent && Amount > 0.f)
	{
		UWGasAbilitySystemFunctionLibrary::AddMomentum(AbilitySystemComponent, Amount);
	}
}

void AWGasCharacterHero::NotifyParrySuccess()
{
	GrantMomentum(MomentumGainOnParrySuccess);
}

void AWGasCharacterHero::NotifyHitReact(float DamageAmount)
{
	if (DamageAmount <= 0.f) return;
	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	const FWGasGameplayTags& WGasTags = FWGasGameplayTags::Get();
	if (ASC)
	{
		if (WGasTags.State_Invulnerable.IsValid() && ASC->HasMatchingGameplayTag(WGasTags.State_Invulnerable)) return;
		if (WGasTags.State_HitReact.IsValid() && ASC->HasMatchingGameplayTag(WGasTags.State_HitReact)) return;
		if (WGasTags.State_Parry_Window.IsValid() && ASC->HasMatchingGameplayTag(WGasTags.State_Parry_Window)) return;
	}
	if (UWGasMeleeAttack* Melee = UWGasMeleeAttack::GetActiveMeleeAttack(ASC))
	{
		Melee->TryCancelFromCancelablePhase();
	}
	if (ASC && WGasTags.State_HitReact.IsValid())
	{
		ASC->AddLooseGameplayTag(WGasTags.State_HitReact);
	}

	const float Duration = PlayAnimMontage(HitReactMontage);
	if (Duration <= 0.f)
	{
		ASC->RemoveLooseGameplayTag(WGasTags.State_HitReact);
		return;
	}
	if (UAnimInstance* Anim = GetMesh()->GetAnimInstance())
	{
		FOnMontageEnded EndDelegate;
		EndDelegate.BindUObject(this, &AWGasCharacterHero::OnHitReactMontageEnded);
		Anim->Montage_SetEndDelegate(EndDelegate, HitReactMontage);
	}
}
void AWGasCharacterHero::OnHitReactMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (Montage != HitReactMontage) return;
	EndHitReactFromAnimation();
}
void AWGasCharacterHero::EndHitReactFromAnimation()
{
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
	{
		ASC->RemoveLooseGameplayTag(FWGasGameplayTags::Get().State_HitReact);
	}
}

bool AWGasCharacterHero::IsLockOnActive() const
{
	return LockOnComponent && LockOnComponent->IsLockedOn();
}

AActor* AWGasCharacterHero::GetLockOnTargetActor() const
{
	if (!LockOnComponent || !LockOnComponent->IsLockedOn())
	{
		return nullptr;
	}
	return LockOnComponent->GetLockTarget();
}

FVector AWGasCharacterHero::GetLockOnTargetLocation() const
{
	if (!LockOnComponent || !LockOnComponent->IsLockedOn())
	{
		return FVector::ZeroVector;
	}
	return LockOnComponent->GetCurrentLockOnLocation();
}

void AWGasCharacterHero::ApplyMovementSpeed()
{
	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->MaxWalkSpeed = bIsRunning ? RunSpeed : WalkSpeed;
	}
}

void AWGasCharacterHero::HandleDeathExtras()
{
	Super::HandleDeathExtras();
	if (AWGasPlayerController* PC = Cast<AWGasPlayerController>(GetController()))
	{
		DisableInput(PC);
	}
}

void AWGasCharacterHero::BindStaminaDepletedDelegate()
{
	if (bStaminaDepletedBound) return;
	if (UWGasAttributeSet* AS = Cast<UWGasAttributeSet>(AttributeSet))
	{
		AS->OnStaminaDepleted.AddUObject(this, &AWGasCharacterHero::ForceWalk);
		bStaminaDepletedBound = true;
	}
}
void AWGasCharacterHero::PossessedBy(AController* NewController)
{
	//角色被控制时添加能力
	Super::PossessedBy(NewController);
	InitAbilityActorInfo();
	AddCharacterAbilities();
}

void AWGasCharacterHero::InitAbilityActorInfo()
{
	Super::InitAbilityActorInfo();

	AWGasPlayerController* PC=Cast<AWGasPlayerController>(GetController());
	if (!PC)return;
	auto InitHUD=[this,PC]()
	{
		if (AWGasHUD* WGasHUD=Cast<AWGasHUD>(PC->GetHUD()))
		{
			WGasHUD->InitOverlay(PC,AbilitySystemComponent,AttributeSet);
			
		}
	};

	if (AWGasHUD* WGasHUD=Cast<AWGasHUD>(PC->GetHUD()))
	{
		InitHUD();
	}
	else
	{
		FTimerHandle Handle;
		GetWorldTimerManager().SetTimer(
			Handle,
			InitHUD,
			0.0f,   
			false
		);
	}
	InitializeDefaultAttributes();
	BindStaminaDepletedDelegate();

	//这里处理的不好，先暂时这样放着
	if (!bRunStaminaEffectsApplied && StaminaCostGE)
	{
		ApplyEffectToSelf(StaminaCostGE, 1.f);
		bRunStaminaEffectsApplied = true;
	}
	
}
