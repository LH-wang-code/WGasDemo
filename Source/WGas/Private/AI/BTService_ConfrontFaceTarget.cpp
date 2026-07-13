#include "AI/BTService_ConfrontFaceTarget.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Navigation/PathFollowingComponent.h"

UBTService_ConfrontFaceTarget::UBTService_ConfrontFaceTarget()
{
	NodeName = TEXT("Confront Face Target");
	Interval = 0.f;
	RandomDeviation = 0.f;
	bNotifyTick = true;
	bCreateNodeInstance = false;

	TargetActorKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_ConfrontFaceTarget, TargetActorKey), AActor::StaticClass());
}

void UBTService_ConfrontFaceTarget::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);

	if (const UBlackboardData* BBAsset = GetBlackboardAsset())
	{
		TargetActorKey.ResolveSelectedKey(*BBAsset);
	}
}

void UBTService_ConfrontFaceTarget::OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	if (AAIController* AIController = OwnerComp.GetAIOwner())
	{
		AIController->ClearFocus(EAIFocusPriority::Gameplay);
	}

	Super::OnCeaseRelevant(OwnerComp, NodeMemory);
}

void UBTService_ConfrontFaceTarget::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);
	ApplyFaceRotation(OwnerComp, DeltaSeconds);
}

void UBTService_ConfrontFaceTarget::ApplyFaceRotation(UBehaviorTreeComponent& OwnerComp, float DeltaSeconds) const
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	ACharacter* Character = AIController ? Cast<ACharacter>(AIController->GetPawn()) : nullptr;
	UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
	if (!Character || !Blackboard || !AIController || !TargetActorKey.IsSet())
	{
		return;
	}

	if (Blackboard->GetValueAsBool(TEXT("bPoiseBroken")))
	{
		return;
	}

	AActor* Target = Cast<AActor>(Blackboard->GetValueAsObject(TargetActorKey.SelectedKeyName));
	if (!IsValid(Target))
	{
		return;
	}

	// 正在 Move To 时不要 SetFocus，避免和寻路抢控制权
	if (AIController->GetMoveStatus() != EPathFollowingStatus::Idle)
	{
		return;
	}

	if (UCharacterMovementComponent* Movement = Character->GetCharacterMovement())
	{
		Movement->bOrientRotationToMovement = false;
		Movement->bUseControllerDesiredRotation = false;
		Movement->RotationRate = FRotator(0.f, YawInterpSpeed, 0.f);
	}

	AIController->bAllowStrafe = true;
	AIController->SetFocus(Target, EAIFocusPriority::Gameplay);
	AIController->UpdateControlRotation(DeltaSeconds, true);
}
