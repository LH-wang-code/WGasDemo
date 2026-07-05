#include "UI/Widgets/StaminaBarComponent.h"

UStaminaBarComponent::UStaminaBarComponent()
{
	// Screen：挂在世界坐标，但始终朝向相机（UE5 无 Billboard GeometryMode）
	SetWidgetSpace(EWidgetSpace::Screen);
	SetDrawSize(FVector2D(200.f, 16.f));
	SetPivot(FVector2D(0.5f, 0.5f));
	SetCollisionEnabled(ECollisionEnabled::NoCollision);
}
