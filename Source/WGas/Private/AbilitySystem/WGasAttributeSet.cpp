// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/WGasAttributeSet.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "GameplayEffectExtension.h"
#include "WGasGameplayTags.h"
#include "GameFramework/Character.h"

UWGasAttributeSet::UWGasAttributeSet()
{
	const FWGasGameplayTags& GameplayTags = FWGasGameplayTags::Get();
	TagsToAttributes.Add(GameplayTags.Attribute_Vital_MaxHealth,GetMaxHealthAttribute);
	TagsToAttributes.Add(GameplayTags.Attribute_Vital_MaxMana,GetMaxManaAttribute);
	TagsToAttributes.Add(GameplayTags.Attribute_Vital_MaxPoise,GetMaxPoiseAttribute);
	TagsToAttributes.Add(GameplayTags.Attribute_Vital_MaxStamina,GetMaxStaminaAttribute);
	TagsToAttributes.Add(GameplayTags.Attribute_Vital_MaxMomentum,GetMaxMomentumAttribute);

	InitMomentum(0.f);
	InitMaxMomentum(100.f);
}
//改属性前处理
void UWGasAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);
	if (Attribute==GetHealthAttribute())
	{
		NewValue=FMath::Clamp(NewValue,0.f,GetMaxHealth());
	}
	if (Attribute == GetMaxHealthAttribute())
	{

		UE_LOG(LogTemp, Warning, TEXT("MaxHealth: %f"), NewValue);

	}
	if (Attribute == GetManaAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxMana());

		UE_LOG(LogTemp, Warning, TEXT("Mana: %f"), NewValue);

	}
	if (Attribute == GetMaxManaAttribute())
	{
		UE_LOG(LogTemp, Warning, TEXT("MaxMana: %f"), NewValue);
	}
	if (Attribute == GetMomentumAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxMomentum());
	}
	if (Attribute == GetMaxMomentumAttribute())
	{
		UE_LOG(LogTemp, Warning, TEXT("MaxMomentum: %f"), GetMaxMomentum());
	}
}

void UWGasAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);
	FEffectProperties Props;
	SetEffectProperties(Data,Props);

	//TODO:判断作用的角色是否死亡
	UE_LOG(LogTemp, Warning, TEXT("PostExecute Attribute: %s  Magnitude: %.1f"),
		 *Data.EvaluatedData.Attribute.GetName(),
		 Data.EvaluatedData.Magnitude);
	if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		const FWGasGameplayTags& WGasTags = FWGasGameplayTags::Get();
		UAbilitySystemComponent* TargetASC = Props.TargetASC ? Props.TargetASC : GetOwningAbilitySystemComponent();
		if (WGasTags.State_Invulnerable.IsValid() && TargetASC
			&& TargetASC->HasMatchingGameplayTag(WGasTags.State_Invulnerable)
			&& Data.EvaluatedData.Magnitude < 0.f)
		{
			SetHealth(GetHealth() - Data.EvaluatedData.Magnitude);
		}
		SetHealth(FMath::Clamp(GetHealth(), 0.0f, GetMaxHealth()));
	}
	if (Data.EvaluatedData.Attribute == GetManaAttribute())
	{
		SetMana(FMath::Clamp(GetMana(), 0.0f, GetMaxMana()));
	}
	if (Data.EvaluatedData.Attribute == GetPoiseAttribute())
	{
		SetPoise(FMath::Clamp(GetPoise(), 0.0f, GetMaxPoise()));
	}
	if (Data.EvaluatedData.Attribute==GetStaminaAttribute())
	{
		SetStamina(FMath::Clamp(GetStamina(), 0.0f, GetMaxStamina()));
		
	}

	
	if (Data.EvaluatedData.Attribute == GetIncomingDamageAttribute())
	{
		const FWGasGameplayTags& WGasTags = FWGasGameplayTags::Get();
		UAbilitySystemComponent* TargetASC = Props.TargetASC ? Props.TargetASC : GetOwningAbilitySystemComponent();
		if (WGasTags.State_Invulnerable.IsValid() && TargetASC
			&& TargetASC->HasMatchingGameplayTag(WGasTags.State_Invulnerable))
		{
			SetIncomingDamage(0.f);
			return;
		}

		const float Damage=GetIncomingDamage();
		SetIncomingDamage(0.f);
		const float NewHealth = GetHealth() - Damage;
		SetHealth(FMath::Clamp(NewHealth, 0.f, GetMaxHealth()));
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red,
				FString::Printf(TEXT("[%s] Took %.0f! HP %.0f -> %.0f"),
					*GetOwningActor()->GetName(),
					Damage, GetHealth() + Damage, GetHealth()));
		}
	}

	if (Data.EvaluatedData.Attribute == GetIncomingPoiseDamageAttribute())
	{
		const float PoiseDamage = GetIncomingPoiseDamage();
		SetIncomingPoiseDamage(0.f);
		// 破韧期间只回不涨
		if (PoiseDamage > 0.f && !bPoiseBrokenActive)
		{
			const float NewPoise = GetPoise() - PoiseDamage;
			SetPoise(FMath::Clamp(NewPoise, 0.f, GetMaxPoise()));
		}
	}
}

void UWGasAttributeSet::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue)
{
	Super::PostAttributeChange(Attribute, OldValue, NewValue);
	if (Attribute == GetStaminaAttribute())
	{
		if (OldValue > 0.f && NewValue <= 0.f)
		{
			OnStaminaDepleted.Broadcast();
		}
	}
	if (Attribute == GetPoiseAttribute())
	{
		if (!bPoiseBrokenActive && OldValue > 0.f && NewValue <= 0.f)
		{
			bPoiseBrokenActive = true;
			OnPoiseBroken.Broadcast();
		}
		const float maxPoise = GetMaxPoise();
		if (bPoiseBrokenActive && maxPoise > 0.f && NewValue >= maxPoise)
		{
			bPoiseBrokenActive = false;
			OnPoiseRecovered.Broadcast();
		}
	}
	if (Attribute == GetMomentumAttribute() || Attribute == GetMaxMomentumAttribute())
	{
		if (Attribute == GetMomentumAttribute()
			&& GetMaxMomentum() > 0.f
			&& OldValue < GetMaxMomentum()
			&& NewValue >= GetMaxMomentum())
		{
			OnMomentumFilled.Broadcast();
		}
	}
}

void UWGasAttributeSet::SetEffectProperties(const FGameplayEffectModCallbackData& Data, FEffectProperties& Props) const
{
	Props.EffectContextHandle = Data.EffectSpec.GetContext();
	Props.SourceASC = Props.EffectContextHandle.GetOriginalInstigatorAbilitySystemComponent();
	if (IsValid(Props.SourceASC) && Props.SourceASC->AbilityActorInfo.IsValid() && Props.SourceASC->AbilityActorInfo->AvatarActor.IsValid())
	{
		Props.SourceAvatarActor = Props.SourceASC->AbilityActorInfo->AvatarActor.Get();
		Props.SourceController = Props.SourceASC->AbilityActorInfo->PlayerController.Get();
		if (Props.SourceController == nullptr && Props.SourceAvatarActor != nullptr)
		{
			if (APawn* Pawn = Cast<APawn>(Props.SourceAvatarActor))
			{
				Props.SourceController = Pawn->GetController();
			}
		}
		if (Props.SourceController)
		{
			Props.SourceCharacter = Cast<ACharacter>(Props.SourceController->GetPawn());
		}
	}
	if (Data.Target.AbilityActorInfo.IsValid() && Data.Target.AbilityActorInfo->AvatarActor.IsValid())
	{
		Props.TargetAvatarActor= Data.Target.AbilityActorInfo->AvatarActor.Get();
		Props.TargetController = Data.Target.AbilityActorInfo->PlayerController.Get();
		Props.TargetCharacter = Cast<ACharacter>(Props.TargetAvatarActor);
		Props.TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Props.TargetAvatarActor);
	}
	
}
