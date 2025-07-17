#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PlayerHUDWidget.generated.h"

/**
 * 
 */
UCLASS()
class ACTIONRPG_API UPlayerHUDWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;

public:
	void UpdateHPBar(float CurrentHp, float MaxHp);

protected:
	// BindWidget - 에디터에서 이름이 정확히 일치하는 위젯을 자동으로 bind함 (없을 경우 nullptr)
	UPROPERTY(meta = (BindWidget))
	class UProgressBar* PB_HpBar;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* TB_HpText;
};
