#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Animation/AnimInstance.h"
#include "Engine/DamageEvents.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Component/CharacterStatComponent.h"
#include "Components/CapsuleComponent.h"
#include "CharacterBase.generated.h"

class UAnimInstance;
class UCharacterStatComponent;
struct FInputActionValue;

UCLASS()
class ACTIONRPG_API ACharacterBase : public ACharacter
{
	GENERATED_BODY()

public:
	ACharacterBase();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

public:
	// UFUNCTION 매크로 - UE에 함수 등록을 위해 필요 (블루프린트 인식 및 리플렉션 시스템)
	UFUNCTION(BlueprintCallable)
	virtual void Die();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UAnimInstance> AnimBlueprintClass;

	/* Combo System */
protected:
	void StartComboSection();
	FName GetComboSectionName(const int32& ComboIndex) const;
	void ComboActionEndCallback(class UAnimMontage* TargetMontage, bool bIsProperlyEnded);
	void ComboTimerCallback();

public:
	// 공격 판정을 하는 Animation Notify에서 호출될 콜백 함수
	void Notify_AttackHitCheck();

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Combat|Montage")
	UAnimMontage*	AttackMontage = nullptr;

	int32           CurrentComboIndex = 0;
	int32			MaxComboCount = 4;
	FTimerHandle	ComboTimerHandle;
	bool			bIsAttacking = false;
	bool            bNextComboReserved = false;

	/* Damage System */
public:
	virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
		AController* EventInstigator, AActor* DamageCauser) override;
public:
	UCharacterStatComponent* GetStatComponent() const;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stat", meta = (AllowPrivateAccess = "true"))
	UCharacterStatComponent* StatComponent;
};
