// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/WGasCharacterEnemy.h"

void AWGasCharacterEnemy::BeginPlay()
{
	Super::BeginPlay();
	InitAbilityActorInfo();
	
}

void AWGasCharacterEnemy::InitAbilityActorInfo()
{
	AbilitySystemComponent->InitAbilityActorInfo(this, this);
	
}
