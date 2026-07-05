// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Data/CharacterClassInfo.h"

FCharacterClassDefaultInfo UCharacterClassInfo::GetInfoWithECharacterClass(ECharacterClass CharacterClass)
{
	return CharacterClassInfo.FindChecked(CharacterClass);
}
