#include "UI/Widgets/StaminaBarComponent.h"

UStaminaBarComponent::UStaminaBarComponent()
{
	// Screen：挂在世界坐标，但始终朝向相机（UE5 无 Billboard GeometryMode）
	SetWidgetSpace(EWidgetSpace::Screen);
	// 竖条：宽小高大（原先是 200x16 横条）
	SetDrawSize(FVector2D(16.f, 200.f));
	SetPivot(FVector2D(0.5f, 0.5f));
	SetCollisionEnabled(ECollisionEnabled::NoCollision);
}
