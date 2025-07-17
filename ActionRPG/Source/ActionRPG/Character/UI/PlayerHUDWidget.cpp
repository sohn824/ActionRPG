#include "PlayerHUDWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

void UPlayerHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void UPlayerHUDWidget::UpdateHPBar(float CurrentHp, float MaxHp)
{
	if (PB_HpBar)
	{
		const float HpPercent = CurrentHp / MaxHp;
		PB_HpBar->SetPercent(HpPercent);
	}
}
