// Fill out your copyright notice in the Description page of Project Settings.


#include "Input/GasInputBufferComponent.h"

#include "ToolMenusEditor.h"
#include "Tests/ToolMenusTestUtilities.h"

// Sets default values for this component's properties
UGasInputBufferComponent::UGasInputBufferComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UGasInputBufferComponent::BufferInput(FGameplayTag InputTag)
{
	if (!InputTag.IsValid())return;
	const float Now=GetWorld()->GetTimeSeconds();

	for (FBufferedInput& Entry : BufferedInputs)
	{
		if (Entry.InputTag==InputTag)
		{

			//对于当前输入，如果已经存在的话，我们更新它的时间即可
			Entry.TimerBuffered=Now;
			return;
		}
	}
	//不存在就新建
	FBufferedInput NewEntry;
	NewEntry.InputTag=InputTag;
	NewEntry.TimerBuffered=Now;
	BufferedInputs.Add(NewEntry);
}

bool UGasInputBufferComponent::HasValidBuffer(FGameplayTag InputTag)
{
	for (const FBufferedInput& Entry : BufferedInputs)
	{
		if (Entry.InputTag==InputTag&&IsEntryValid(Entry))
		{
			return true;
		}
	}
	return false;
}

bool UGasInputBufferComponent::ConsumeBuffer(FGameplayTag InputTag)
{
	for (int32 i=0;i<BufferedInputs.Num();i++)
	{
		if (BufferedInputs[i].InputTag==InputTag&&IsEntryValid(BufferedInputs[i]))
		{
			BufferedInputs.RemoveAt(i);
			return true;
		}
	}
	return false;
}

void UGasInputBufferComponent::ClearBuffer(FGameplayTag InputTag)
{
	BufferedInputs.RemoveAll([InputTag](const FBufferedInput& Entry)
	{
		return Entry.InputTag==InputTag;
	});
}

void UGasInputBufferComponent::ClearAllBuffers()
{
	BufferedInputs.Empty();
}

bool UGasInputBufferComponent::IsEntryValid(const FBufferedInput& Entry) const
{

	//tag本身是否合法以及时间是否超时
	if (!Entry.InputTag.IsValid())return false;
	const float Now=GetWorld()->GetTimeSeconds();
	return (Now-Entry.TimerBuffered)<DefaultBufferDuration;
}


