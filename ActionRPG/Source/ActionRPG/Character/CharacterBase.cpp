// Fill out your copyright notice in the Description page of Project Settings.

#include "CharacterBase.h"

ACharacterBase::ACharacterBase()
{
	PrimaryActorTick.bCanEverTick = true;
	MaxHp = 100.0f;
}

void ACharacterBase::BeginPlay()
{
	Super::BeginPlay();

	CurrentHp = MaxHp;
}

void ACharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ACharacterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

float ACharacterBase::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	CurrentHp = FMath::Clamp(CurrentHp - DamageAmount, 0.0f, MaxHp);

	OnDamaged(DamageAmount);

	return DamageAmount;
}

void ACharacterBase::Die()
{
	Destroy();
}

void ACharacterBase::OnDamaged(float InDamage)
{

}
