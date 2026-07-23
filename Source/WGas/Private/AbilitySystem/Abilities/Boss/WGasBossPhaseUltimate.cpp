#include "AbilitySystem/Abilities/Boss/WGasBossPhaseUltimate.h"

#include "AbilitySystemComponent.h"
#include "Animation/AnimInstance.h"
#include "Character/WGasCharacterEnemy.h"
#include "Character/WGasCharacterHero.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/OverlapResult.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "WGasGameplayTags.h"
#include "Kismet/GameplayStatics.h"

UWGasBossPhaseUltimate::UWGasBossPhaseUltimate()
{
    bInterruptibleByParry = false;
    bStopMovementOnAttack = false;
    bFaceTargetOnAttack = false;
    const FWGasGameplayTags& Tags = FWGasGameplayTags::Get();
    ActivationBlockedTags.RemoveTag(Tags.State_Boss_PhaseTransition);
    ActivationBlockedTags.RemoveTag(Tags.State_Boss_Invulnerable);
}

void UWGasBossPhaseUltimate::ActivateAbility(const FGameplayAbilitySpecHandle Handle,const FGameplayAbilityActorInfo* ActorInfo,const FGameplayAbilityActivationInfo ActivationInfo,const FGameplayEventData* TriggerEventData)
{
    // 转阶段的结束时机由坠地冲击决定，不能沿用父类“Montage 播完就结束”的逻辑。
    // 否则咆哮 Montage 的长度会和升空/坠地时间轴脱节，留下无敌与转阶段状态。
    if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    BeginBossMeleeAttack();

    if (AttackMontage)
    {
        if (AWGasCharacterEnemy* Boss = Cast<AWGasCharacterEnemy>(GetAvatarActorFromActorInfo()))
        {
            if (USkeletalMeshComponent* Mesh = Boss->GetMesh())
            {
                if (UAnimInstance* AnimInstance = Mesh->GetAnimInstance())
                {
                    AnimInstance->Montage_Play(AttackMontage);
                }
            }
        }
    }

    StartPhaseSequence();
}

void UWGasBossPhaseUltimate::StartPhaseSequence()
{
    ClearPhaseSequence();
    AWGasCharacterEnemy* Boss =Cast<AWGasCharacterEnemy>(GetAvatarActorFromActorInfo());
    UWorld* World = Boss ? Boss->GetWorld() : nullptr;
    if (!World)
    {
        return;
    }
    bImpactTriggered = false;
    World->GetTimerManager().SetTimer(AscendStartTimer,this,&UWGasBossPhaseUltimate::BeginAscend,AscendDelay,false);
    World->GetTimerManager().SetTimer(SummonSwordTimer,this,&UWGasBossPhaseUltimate::SummonSecondWeapon,SummonSwordDelay,false);
    World->GetTimerManager().SetTimer(TelegraphTimer,this,&UWGasBossPhaseUltimate::ShowImpactTelegraph,TelegraphDelay,false);
    World->GetTimerManager().SetTimer(DiveStartTimer,this,&UWGasBossPhaseUltimate::BeginDive,DiveDelay,false);
}

void UWGasBossPhaseUltimate::BeginAscend()
{
    AWGasCharacterEnemy* Boss =Cast<AWGasCharacterEnemy>(GetAvatarActorFromActorInfo());
    UWorld* World = Boss ? Boss->GetWorld() : nullptr;
    if (!Boss || !World)
    {
        return;
    }
    if (UCharacterMovementComponent* Movement =Boss->GetCharacterMovement())
    {
        Movement->StopMovementImmediately();
        Movement->SetMovementMode(MOVE_Flying);
    }
    AscendStartLocation = Boss->GetActorLocation();
    AirLocation = AscendStartLocation + FVector(0.f, 0.f, AscendHeight);
    AscendElapsed = 0.f;
    bAscending = true;
    World->GetTimerManager().SetTimer(AscendTickTimer,this,&UWGasBossPhaseUltimate::UpdateAscend,1.f / 60.f,true);
}

void UWGasBossPhaseUltimate::UpdateAscend()
{
    AWGasCharacterEnemy* Boss =Cast<AWGasCharacterEnemy>(GetAvatarActorFromActorInfo());
    UWorld* World = Boss ? Boss->GetWorld() : nullptr;
    if (!Boss || !World)
    {
        ClearPhaseSequence();
        return;
    }
    AscendElapsed += World->GetDeltaSeconds();
    const float RawAlpha =FMath::Clamp(AscendElapsed / AscendDuration, 0.f, 1.f);

    const float Alpha = FMath::InterpEaseOut(0.f, 1.f, RawAlpha, 2.f);
    Boss->SetActorLocation(FMath::Lerp(AscendStartLocation, AirLocation, Alpha),false);

    if (RawAlpha >= 1.f)
    {
        bAscending = false;
        World->GetTimerManager().ClearTimer(AscendTickTimer);
    }
}

void UWGasBossPhaseUltimate::SummonSecondWeapon()
{
    AWGasCharacterEnemy* Boss =Cast<AWGasCharacterEnemy>(GetAvatarActorFromActorInfo());

    UWorld* World = Boss ? Boss->GetWorld() : nullptr;
    if (!Boss || !World)
    {
        return;
    }
    if (SummonSwordVisualClass)
    {
        FActorSpawnParameters Params;
        Params.Owner = Boss;
        World->SpawnActor<AActor>(SummonSwordVisualClass,Boss->GetActorLocation() + FVector(0.f, 0.f, 120.f),Boss->GetActorRotation(),Params);
    }

    if (Phase2GreatswordMesh)
    {
        if (USkeletalMeshComponent* WeaponMesh =Boss->GetWeaponTraceMesh())
        {
            WeaponMesh->SetSkeletalMesh(Phase2GreatswordMesh);
            WeaponMesh->AttachToComponent(Boss->GetMesh(),FAttachmentTransformRules::SnapToTargetNotIncludingScale,Phase2GreatswordSocket);
            WeaponMesh->SetVisibility(true, true);
        }
    }
}

void UWGasBossPhaseUltimate::ShowImpactTelegraph()
{
    AWGasCharacterEnemy* Boss =Cast<AWGasCharacterEnemy>(GetAvatarActorFromActorInfo());

    UWorld* World = Boss ? Boss->GetWorld() : nullptr;
    if (!Boss || !World)
    {
        return;
    }
    ACharacter* Player = UGameplayStatics::GetPlayerCharacter(this, 0);
    if (!Player)
    {
        return;
    }
    const FVector PlayerLocation = Player->GetActorLocation();

    FHitResult Hit;
    FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(BossPhaseImpactTrace),false,Boss);

    const FVector TraceStart = PlayerLocation + FVector(0.f, 0.f, 2000.f);
    const FVector TraceEnd = PlayerLocation - FVector(0.f, 0.f, 2000.f);

    if (World->LineTraceSingleByChannel(Hit,TraceStart,TraceEnd,ECC_Visibility,QueryParams))
    {
        ImpactLocation = Hit.ImpactPoint;
    }
    else
    {
        ImpactLocation = PlayerLocation;
    }

    if (ImpactTelegraphClass)
    {
        FActorSpawnParameters Params;
        Params.Owner = Boss;
        ImpactTelegraphActor = World->SpawnActor<AActor>(ImpactTelegraphClass,ImpactLocation,FRotator::ZeroRotator,Params);
    }
}

void UWGasBossPhaseUltimate::BeginDive()
{
    AWGasCharacterEnemy* Boss =Cast<AWGasCharacterEnemy>(GetAvatarActorFromActorInfo());
    UWorld* World = Boss ? Boss->GetWorld() : nullptr;
    if (!Boss || !World)
    {
        return;
    }
    DiveStartLocation = Boss->GetActorLocation();
    FVector FlatDirection = ImpactLocation - DiveStartLocation;
    FlatDirection.Z = 0.f;
    if (!FlatDirection.IsNearlyZero())
    {
        Boss->SetActorRotation(FlatDirection.Rotation());
    }
    DiveElapsed = 0.f;
    bDiving = true;
    World->GetTimerManager().SetTimer(DiveTickTimer,this,&UWGasBossPhaseUltimate::UpdateDive,1.f / 60.f,true);
}

void UWGasBossPhaseUltimate::UpdateDive()
{
    AWGasCharacterEnemy* Boss =Cast<AWGasCharacterEnemy>(GetAvatarActorFromActorInfo());
    UWorld* World = Boss ? Boss->GetWorld() : nullptr;
    if (!Boss || !World)
    {
        ClearPhaseSequence();
        return;
    }
    DiveElapsed += World->GetDeltaSeconds();
    const float RawAlpha =FMath::Clamp(DiveElapsed / DiveDuration, 0.f, 1.f);
    const float Alpha = FMath::InterpEaseIn(0.f, 1.f, RawAlpha, 2.f);
    const FVector LandingLocation =ImpactLocation + FVector(0.f, 0.f, 40.f);
    Boss->SetActorLocation(FMath::Lerp(DiveStartLocation, LandingLocation, Alpha),false);
    if (RawAlpha >= 1.f)
    {
        bDiving = false;
        World->GetTimerManager().ClearTimer(DiveTickTimer);
        TriggerImpact();
    }
}
void UWGasBossPhaseUltimate::TriggerImpact()
{
    if (bImpactTriggered)
    {
        return;
    }
    bImpactTriggered = true;
    AWGasCharacterEnemy* Boss =Cast<AWGasCharacterEnemy>(GetAvatarActorFromActorInfo());
    UWorld* World = Boss ? Boss->GetWorld() : nullptr;
    if (!Boss || !World)
    {
        return;
    }
    if (UCharacterMovementComponent* Movement =Boss->GetCharacterMovement())
    {
        Movement->SetMovementMode(MOVE_Walking);
    }
    if (ImpactTelegraphActor.IsValid())
    {
        ImpactTelegraphActor->Destroy();
        ImpactTelegraphActor.Reset();
    }
    ApplyImpactDamage();
    if (ImpactVisualClass)
    {
        FActorSpawnParameters Params;
        Params.Owner = Boss;
        World->SpawnActor<AActor>(ImpactVisualClass,ImpactLocation,FRotator::ZeroRotator,Params);
    }

    // 坠地结算完成才进入二阶段：FinishAttack -> OnPhaseUltimateEnded -> EnterPhase2。
    EndBossMeleeAttack(false);
}

void UWGasBossPhaseUltimate::ApplyImpactDamage()
{
    AWGasCharacterEnemy* Boss =Cast<AWGasCharacterEnemy>(GetAvatarActorFromActorInfo());
    UWorld* World = Boss ? Boss->GetWorld() : nullptr;
    if (!Boss || !World)
    {
        return;
    }
    FCollisionObjectQueryParams ObjectQuery;
    ObjectQuery.AddObjectTypesToQuery(ECC_Pawn);
    FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(BossPhaseImpactOverlap),false,Boss);
    TArray<FOverlapResult> Overlaps;
    const bool bHitAnything = World->OverlapMultiByObjectType(Overlaps,ImpactLocation,FQuat::Identity,ObjectQuery,FCollisionShape::MakeSphere(ImpactRadius),QueryParams);
    if (!bHitAnything)
    {
        return;
    }
    TSet<AActor*> DamagedActors;
    for (const FOverlapResult& Overlap : Overlaps)
    {
        AActor* TargetActor = Overlap.GetActor();
        if (!TargetActor || TargetActor == Boss|| DamagedActors.Contains(TargetActor))
        {
            continue;
        }
        if (!Cast<AWGasCharacterHero>(TargetActor))
        {
            continue;
        }
        DamagedActors.Add(TargetActor);
        CauseDamage(TargetActor);
    }
}

void UWGasBossPhaseUltimate::ClearPhaseSequence()
{
    AWGasCharacterEnemy* Boss =Cast<AWGasCharacterEnemy>(GetAvatarActorFromActorInfo());
    UWorld* World = Boss ? Boss->GetWorld() : nullptr;
    if (World)
    {
        World->GetTimerManager().ClearTimer(AscendStartTimer);
        World->GetTimerManager().ClearTimer(SummonSwordTimer);
        World->GetTimerManager().ClearTimer(TelegraphTimer);
        World->GetTimerManager().ClearTimer(DiveStartTimer);
        World->GetTimerManager().ClearTimer(AscendTickTimer);
        World->GetTimerManager().ClearTimer(DiveTickTimer);
    }
    bAscending = false;
    bDiving = false;
    if (ImpactTelegraphActor.IsValid())
    {
        ImpactTelegraphActor->Destroy();
        ImpactTelegraphActor.Reset();
    }
    if (Boss)
    {
        if (UCharacterMovementComponent* Movement =Boss->GetCharacterMovement())
        {
            if (Movement->MovementMode == MOVE_Flying)
            {
                Movement->SetMovementMode(MOVE_Walking);
            }
        }
    }
}
void UWGasBossPhaseUltimate::FinishAttack(bool bWasCancelled)
{
    ClearPhaseSequence();
    if (AWGasCharacterEnemy* Enemy =Cast<AWGasCharacterEnemy>(GetAvatarActorFromActorInfo()))
    {
        Enemy->OnPhaseUltimateEnded();
    }
    Super::FinishAttack(bWasCancelled);
}

void UWGasBossPhaseUltimate::EndAbility(const FGameplayAbilitySpecHandle Handle,const FGameplayAbilityActorInfo* ActorInfo,const FGameplayAbilityActivationInfo ActivationInfo,bool bReplicateEndAbility,bool bWasCancelled)
{
    ClearPhaseSequence();
    if (AWGasCharacterEnemy* Enemy =Cast<AWGasCharacterEnemy>(GetAvatarActorFromActorInfo()))
    {
        Enemy->OnPhaseUltimateEnded();
    }
    Super::EndAbility(Handle,ActorInfo,ActivationInfo,bReplicateEndAbility,bWasCancelled);
}
