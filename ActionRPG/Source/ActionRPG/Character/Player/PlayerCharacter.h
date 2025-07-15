// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "../CharacterBase.h"
#include "PlayerCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
class UAnimInstance;
struct FInputActionValue;

/**
 * 
 */
UCLASS()
class ACTIONRPG_API APlayerCharacter : public ACharacterBase
{
	GENERATED_BODY()
	
public:
	APlayerCharacter();

protected:
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void Attack(const FInputActionValue& Value);

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom = nullptr;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* InputMappingContext = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* JumpAction = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAction = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* AttackAction = nullptr;

	/* Combo System */
protected:
	void StartComboSection();
	FName GetComboSectionName(const int32& ComboIndex) const;
	void ComboActionEndCallback(class UAnimMontage* TargetMontage, bool bIsProperlyEnded);
	void ComboTimerCallback();

public:
	// 공격 판정을 하는 Animation Notify에서 호출될 콜백 함수
	void Notify_AttackHitCheck();

private:
	UPROPERTY(EditDefaultsOnly, Category = "Combat|Montage")
	UAnimMontage*	AttackMontage = nullptr;

	int32           CurrentComboIndex = 0;
	int32			MaxComboCount = 4;
	FTimerHandle	ComboTimerHandle;
	bool			bIsAttacking = false;
	bool            bNextComboReserved = false;
};
