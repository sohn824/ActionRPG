#include "CharacterStatComponent.h"

UCharacterStatComponent::UCharacterStatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UCharacterStatComponent::BeginPlay()
{
	Super::BeginPlay();

	SetCurrentHp(MaxHp);
}

void UCharacterStatComponent::TakeDamage(float DamageAmount)
{
	if (IsDead())
	{
		return;
	}

	SetCurrentHp(CurrentHp - DamageAmount);

	if (IsDead())
	{
		OnDieParent.Broadcast();
	}
}

void UCharacterStatComponent::Heal(float HealAmount)
{
	if (IsDead())
	{
		return;
	}

	SetCurrentHp(CurrentHp + HealAmount);
}

bool UCharacterStatComponent::IsDead() const
{
	return CurrentHp <= 0.0f;
}

float UCharacterStatComponent::GetMaxHp() const
{
	return MaxHp;
}

void UCharacterStatComponent::SetMaxHp(float NewMaxHp)
{
	MaxHp = NewMaxHp;
}

float UCharacterStatComponent::GetCurrentHp() const
{
	return CurrentHp;
}

void UCharacterStatComponent::SetCurrentHp(float NewHp)
{
	CurrentHp = FMath::Clamp(NewHp, 0, MaxHp);

	OnHPChanged.Broadcast(CurrentHp, MaxHp);
}

const float UCharacterStatComponent::GetAttackPower() const
{
	return AttackPower;
}

void UCharacterStatComponent::SetAttackPower(float NewAttackPower)
{
	AttackPower = FMath::Max(0, NewAttackPower);
}
