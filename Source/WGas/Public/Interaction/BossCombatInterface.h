#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "BossCombatInterface.generated.h"

class UBossAttackInfo;

UINTERFACE(MinimalAPI, BlueprintType)
class UBossCombatInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * Boss 战斗边界：AI、伤害结算与 GA 通过该接口协作，避免依赖具体 Boss 角色和组件。
 */
class WGAS_API IBossCombatInterface
{
	GENERATED_BODY()

public:
	virtual const UBossAttackInfo* GetBossAttackSet() const = 0;
	virtual bool IsBossPhase2() const = 0;
	virtual float GetBossIncomingDamageMultiplier() const = 0;

	virtual void NotifyBossGuardedHit() = 0;
	virtual void NotifyBossNormalAttackFinished() = 0;
	virtual bool IsBossGuardReleaseReady() const = 0;
	virtual void ConsumeBossGuardRelease() = 0;
};
