#include "MyPlayerController.h"
#include "PlayerCharacter.h"
#include "../UI/PlayerHUDWidget.h"
#include "../Component/CharacterStatComponent.h"

void AMyPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// 위젯 블루프린트 클래스가 연결되어 있을 때만 실행
	if (HUDWidgetClass)
	{
		UPlayerHUDWidget* HUD = CreateWidget<UPlayerHUDWidget>(this, HUDWidgetClass);
		if (HUD)
		{
			HUD->AddToViewport();
		}

		APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(GetPawn());
		if (PlayerCharacter && PlayerCharacter->GetStatComponent())
		{
			// StatComponent의 체력 변경 이벤트에 UI 함수 바인딩
			PlayerCharacter->GetStatComponent()->OnHPChanged.AddDynamic(HUD, &UPlayerHUDWidget::UpdateHPBar);
		}
	}
}
