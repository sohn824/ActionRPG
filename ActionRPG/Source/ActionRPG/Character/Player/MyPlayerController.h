#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MyPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class ACTIONRPG_API AMyPlayerController : public APlayerController
{
	GENERATED_BODY()
	
protected:
	virtual void BeginPlay() override;

public:
	// EditDefaultsOnly - 블루프린트에서 클래스 지정 가능
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<class UPlayerHUDWidget> HUDWidgetClass;
};
