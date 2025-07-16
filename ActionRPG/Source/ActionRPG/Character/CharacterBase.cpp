#include "CharacterBase.h"

ACharacterBase::ACharacterBase()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ACharacterBase::BeginPlay()
{
	Super::BeginPlay();
}

void ACharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ACharacterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void ACharacterBase::Die()
{
	Destroy();
}

void ACharacterBase::StartComboSection()
{
	if (AttackMontage == nullptr)
	{
		return;
	}

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance == nullptr)
	{
		return;
	}

	CurrentComboIndex = 0;
	const FName& ComboSectionName = GetComboSectionName(CurrentComboIndex);
	// ��Ÿ�� ��� ����
	AnimInstance->Montage_Play(AttackMontage);
	// ��Ÿ�ְ� ���� �� ȣ���� �ݹ� �Լ� ����
	FOnMontageEnded EndDelegate;
	EndDelegate.BindUObject(this, &ACharacterBase::ComboActionEndCallback);
	AnimInstance->Montage_SetEndDelegate(EndDelegate, AttackMontage);
	// �޺� Ÿ�̸� ����
	ComboTimerHandle.Invalidate();
	// �޺� �Է��� ���� �� �ִ� �ð�
	const float ComboEffectiveTime = 0.3f;
	GetWorld()->GetTimerManager().SetTimer(ComboTimerHandle, this,
		&ACharacterBase::ComboTimerCallback, ComboEffectiveTime, false);

	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_None);
}

FName ACharacterBase::GetComboSectionName(const int32& ComboIndex) const
{
	return FName(*FString::Printf(TEXT("Attack%d"), ComboIndex + 1));
}

void ACharacterBase::ComboActionEndCallback(class UAnimMontage* TargetMontage, bool bIsProperlyEnded)
{
	CurrentComboIndex = 0;
	bIsAttacking = false;
	bNextComboReserved = false;

	// �ٽ� ����Ű �Է� ���� �� �ֵ��� MovementMode ����
	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);
}

void ACharacterBase::ComboTimerCallback()
{
	ComboTimerHandle.Invalidate();

	// �޺� �Է¿� �����ߴٸ� �ִϸ��̼� ��Ÿ�ָ� ���� �������� �ѱ��
	if (bNextComboReserved)
	{
		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

		CurrentComboIndex = FMath::Clamp(CurrentComboIndex + 1, 1, MaxComboCount);

		const FName& NextSectionName = GetComboSectionName(CurrentComboIndex);

		AnimInstance->Montage_JumpToSection(NextSectionName, AttackMontage);

		// �޺� Ÿ�̸� �����
		const float ComboEffectiveTime = 0.3f;
		GetWorld()->GetTimerManager().SetTimer(ComboTimerHandle, this,
			&ACharacterBase::ComboTimerCallback, ComboEffectiveTime, false);

		bNextComboReserved = false;
	}
}

// ���� ������ �ϴ� Animation Notify���� ȣ��� �ݹ� �Լ�
void ACharacterBase::Notify_AttackHitCheck()
{
	if (StatComponent == nullptr)
	{
		return;
	}

	// �浹 ������ �ش� ���������� ���� ������ �����ϴ� ����ü
	// SweepSingleByChannel()�� ���� �Լ��� �����ϸ� �浹 ������ �ϰ� ���� ������ ��ȯ��
	FHitResult OutHitResult;

	// �浹 �Լ��� ���޵Ǵ� �Ű����� ����ü
	FCollisionQueryParams CollisionParams(SCENE_QUERY_STAT(Attack), false, this);

	const float AttackRange = 100.0f;
	const float& AttackPower = StatComponent->GetAttackPower();
	const FVector StartPos = GetActorLocation()
		+ GetActorForwardVector() * GetCapsuleComponent()->GetScaledCapsuleRadius();
	const FVector EndPos = StartPos + GetActorForwardVector() * AttackRange;

	// Ư�� ä���� ����Ͽ� �浹 �˻縦 ����
	// StartPos���� EndPos���� ������ �浹ü�� �̵���Ű�鼭 �浹�� �����ϰ�
	// FHitResult ����ü�� �浹 ���� �������� ��� ��ȯ��
	const float AttackRadius = 50.0f;
	bool bHitSucceed = GetWorld()->SweepSingleByChannel(OutHitResult, StartPos, EndPos,
		FQuat::Identity, ECC_GameTraceChannel1, FCollisionShape::MakeSphere(AttackRadius), CollisionParams);
	if (bHitSucceed)
	{
		FDamageEvent DamageEvent;
		// ���� ������ TakeDamage �Լ� ȣ��
		OutHitResult.GetActor()->TakeDamage(AttackPower, DamageEvent, GetController(), this);
	}

#ifdef ENABLE_DRAW_DEBUG
	// �浹 ������
	FVector CapsuleOrigin = StartPos + (EndPos - StartPos) * 0.5f;
	const float CapsuleHalfHeight = AttackRange * 0.5f;
	FColor DrawColor = bHitSucceed ? FColor::Green : FColor::Red;

	DrawDebugCapsule(GetWorld(), CapsuleOrigin, CapsuleHalfHeight, AttackRadius,
		FRotationMatrix::MakeFromZ(GetActorForwardVector()).ToQuat(), DrawColor, false, 3.f);
#endif //ENABLE_DRAW_DEBUG
}

float ACharacterBase::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
	AController* EventInstigator, AActor* DamageCauser)
{
	float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	if (StatComponent)
	{
		StatComponent->TakeDamage(ActualDamage);
	}

	return ActualDamage;
}

UCharacterStatComponent* ACharacterBase::GetStatComponent() const
{
	return StatComponent;
}

