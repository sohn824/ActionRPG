// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CharacterStatComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHPChanged, float, CurrentHp, float, MaxHp);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDieParent);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ACTIONRPG_API UCharacterStatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UCharacterStatComponent();

protected:
	virtual void BeginPlay() override;

public:
	UFUNCTION(BlueprintCallable)
	void TakeDamage(float DamageAmount);

	UFUNCTION(BlueprintCallable)
	void Heal(float HealAmount);

	UFUNCTION(BlueprintCallable)
	bool IsDead() const;

	float GetMaxHp() const;
	void SetMaxHp(float NewMaxHp);
	float GetCurrentHp() const;
	void SetCurrentHp(float NewHp);
	const float GetAttackPower() const;
	void SetAttackPower(float NewAttackPower);

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat")
	float MaxHp = 100.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stat")
	float CurrentHp = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat")
	float AttackPower = 10.0f;

	// UI 바인딩용 Delegate
	UPROPERTY(BlueprintAssignable)
	FOnHPChanged OnHPChanged;

	UPROPERTY(BlueprintAssignable)
	FOnDieParent OnDieParent;
};
