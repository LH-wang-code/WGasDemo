// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/WGasLockOnComponent.h"

#include "AbilitySystem/WGasAbilitySystemComponent.h"
#include "AbilitySystem/WGasAttributeSet.h"
#include "Character/WGasCharacterEnemy.h"
#include "Character/WGasCharacterHero.h"
#include "Engine/OverlapResult.h"



void UWGasLockOnComponent::BeginPlay() 
{
	Super::BeginPlay();
}
UWGasLockOnComponent::UWGasLockOnComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}
void UWGasLockOnComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (bIsLockedOn)
	{
		UpdateLockOn(DeltaTime);
	}
}

void UWGasLockOnComponent::ToggleLockOn()
{
	if (bIsLockedOn)
	{
		ClearLockOn();
		return;
	}
	if (AActor* Target = FindBestTarget())
	{
		LockTarget = Target;
		bIsLockedOn = true;
		// if (AWGasCharacterHero* Hero = Cast<AWGasCharacterHero>(GetOwner()))
		// {
		//     if (auto* ASC = Cast<UWGasAbilitySystemComponent>(Hero->GetAbilitySystemComponent()))
		//     {
		//         ASC->AddLooseGameplayTag(FWGasGameplayTags::Get().State_LockOn);
		//     }
		// }
	}
}

void UWGasLockOnComponent::UpdateLockOn(float DeltaTime)
{
	if (!bIsLockedOn)
	{
		return;
	}
	AWGasCharacterHero* Hero = Cast<AWGasCharacterHero>(GetOwner());
	if (!Hero)
	{
		ClearLockOn();
		return;
	}
	APlayerController* PC = Cast<APlayerController>(Hero->GetController());
	if (!PC)
	{
		ClearLockOn();
		return;
	}
	AActor* Target = LockTarget.Get();
	if (!IsValidLockTarget(Target))
	{
		ClearLockOn();
		return;
	}
	const FVector HeroLoc = Hero->GetActorLocation();
	const FVector TargetLoc = GetLockOnLocation(Target);
	if (FVector::DistSquared(HeroLoc, TargetLoc) > FMath::Square(MaxLockDistance))
	{
		ClearLockOn();
		return;
	}
	FVector CameraLoc = HeroLoc;
	if (APlayerCameraManager* PCM = PC->PlayerCameraManager)
	{
		CameraLoc = PCM->GetCameraLocation();
	}
	FRotator DesiredControlRot = (TargetLoc - CameraLoc).Rotation();
	DesiredControlRot.Roll = 0.f;
	DesiredControlRot.Pitch = FMath::ClampAngle(DesiredControlRot.Pitch, -35.f, 35.f);
	PC->SetControlRotation(FMath::RInterpTo(
		PC->GetControlRotation(),
		DesiredControlRot,
		DeltaTime,
		LockInterpSpeed));
}


AActor* UWGasLockOnComponent::FindBestTarget()
{
	AWGasCharacterHero* Hero = Cast<AWGasCharacterHero>(GetOwner());
	if (!Hero || !GetWorld())
	{
		return nullptr;
	}
	APlayerController* PC = Cast<APlayerController>(Hero->GetController());
	if (!PC)
	{
		return nullptr;
	}
	//获取前向向量和玩家位置
	const FVector HeroLoc = Hero->GetActorLocation();
	const FVector Forward = PC->GetControlRotation().Vector().GetSafeNormal2D();
	//范围
	const float MinDot = FMath::Cos(FMath::DegreesToRadians(MaxLockHalfAngle));

	//球形检测pawn类型
	TArray<FOverlapResult> Overlaps;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(LockOnSearch), false, Hero);
	FCollisionObjectQueryParams ObjectQuery(ECC_Pawn);

	GetWorld()->OverlapMultiByObjectType(
	Overlaps,
	HeroLoc,
	FQuat::Identity,
	ObjectQuery,
	FCollisionShape::MakeSphere(MaxLockDistance),
	Params);
	AActor* BestTarget = nullptr;
	float BestDot = MinDot;
	for (const FOverlapResult& Overlap : Overlaps)
	{
		AActor* Candidate = Overlap.GetActor();
		if (!IsValidLockTarget(Candidate))
		{
			continue;
		}
		const FVector ToTarget = GetLockOnLocation(Candidate) - HeroLoc;
		if (ToTarget.IsNearlyZero())
		{
			continue;
		}

		const float Dot = FVector::DotProduct(Forward, ToTarget.GetSafeNormal2D());
		if (Dot <= BestDot)
		{
			continue;  // 不在扇形内，或不如当前最佳
		}
		if (BestTarget && FMath::IsNearlyEqual(Dot, BestDot))
		{
			//距离相近就比较他们的距离
			const float NewDistSq = ToTarget.SizeSquared();
			const float OldDistSq = FVector::DistSquared(HeroLoc, GetLockOnLocation(BestTarget));
			if (NewDistSq >= OldDistSq)
			{
				continue;
			}
		}
		BestDot = Dot;
		BestTarget = Candidate;
	}
	return BestTarget;
}

void UWGasLockOnComponent::ClearLockOn()
{
	//接触锁定状态
	bIsLockedOn = false;
	LockTarget = nullptr;
	if (AWGasCharacterHero* Hero = Cast<AWGasCharacterHero>(GetOwner()))
	{
		if (UWGasAbilitySystemComponent* ASC = Cast<UWGasAbilitySystemComponent>(Hero->GetAbilitySystemComponent()))
		{
			// ASC->SetLooseGameplayTagCount(FWGasGameplayTags::Get().State_LockOn, 0);
		}
	}
}

FVector UWGasLockOnComponent::GetLockOnLocation(const AActor* Target) const
{
	if (!Target)
	{
		return FVector::ZeroVector;
	}
	if (const ACharacter* Character = Cast<ACharacter>(Target))
	{
		if (const USkeletalMeshComponent* Mesh = Character->GetMesh())
		{
			if (Mesh->DoesSocketExist(LockOnSocketName))
			{
				return Mesh->GetSocketLocation(LockOnSocketName);
			}
		}
	}
	return Target->GetActorLocation() + FVector(0.f, 0.f, LockOnHeightOffset);
}

bool UWGasLockOnComponent::IsValidLockTarget(const AActor* Target) const
{
	if (!IsValid(Target))
	{
		return false;
	}
	//TODO :combat接口下检测IsFriend
	const AWGasCharacterEnemy* Enemy = Cast<AWGasCharacterEnemy>(Target);
	if (!Enemy)
	{
		return false;
	}
	if (const UWGasAttributeSet* AS = Cast<UWGasAttributeSet>(Enemy->GetAttributeSet()))
	{
		if (AS->GetHealth() <= 0.f)
		{
			return false;
		}
	}
	return true;
}



