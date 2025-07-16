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
	// 몽타주 재생 시작
	AnimInstance->Montage_Play(AttackMontage);
	// 몽타주가 끝날 때 호출할 콜백 함수 지정
	FOnMontageEnded EndDelegate;
	EndDelegate.BindUObject(this, &ACharacterBase::ComboActionEndCallback);
	AnimInstance->Montage_SetEndDelegate(EndDelegate, AttackMontage);
	// 콤보 타이머 시작
	ComboTimerHandle.Invalidate();
	// 콤보 입력을 넣을 수 있는 시간
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

	// 다시 방향키 입력 받을 수 있도록 MovementMode 복구
	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);
}

void ACharacterBase::ComboTimerCallback()
{
	ComboTimerHandle.Invalidate();

	// 콤보 입력에 성공했다면 애니메이션 몽타주를 다음 섹션으로 넘기기
	if (bNextComboReserved)
	{
		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

		CurrentComboIndex = FMath::Clamp(CurrentComboIndex + 1, 1, MaxComboCount);

		const FName& NextSectionName = GetComboSectionName(CurrentComboIndex);

		AnimInstance->Montage_JumpToSection(NextSectionName, AttackMontage);

		// 콤보 타이머 재시작
		const float ComboEffectiveTime = 0.3f;
		GetWorld()->GetTimerManager().SetTimer(ComboTimerHandle, this,
			&ACharacterBase::ComboTimerCallback, ComboEffectiveTime, false);

		bNextComboReserved = false;
	}
}

// 공격 판정을 하는 Animation Notify에서 호출될 콜백 함수
void ACharacterBase::Notify_AttackHitCheck()
{
	if (StatComponent == nullptr)
	{
		return;
	}

	// 충돌 지점과 해당 지점에서의 여러 정보를 포함하는 구조체
	// SweepSingleByChannel()과 같은 함수에 전달하면 충돌 판정을 하고 관련 정보를 반환함
	FHitResult OutHitResult;

	// 충돌 함수에 전달되는 매개변수 구조체
	FCollisionQueryParams CollisionParams(SCENE_QUERY_STAT(Attack), false, this);

	const float AttackRange = 100.0f;
	const float& AttackPower = StatComponent->GetAttackPower();
	const FVector StartPos = GetActorLocation()
		+ GetActorForwardVector() * GetCapsuleComponent()->GetScaledCapsuleRadius();
	const FVector EndPos = StartPos + GetActorForwardVector() * AttackRange;

	// 특정 채널을 사용하여 충돌 검사를 수행
	// StartPos에서 EndPos까지 지정된 충돌체를 이동시키면서 충돌을 감지하고
	// FHitResult 구조체에 충돌 관련 정보들을 담아 반환함
	const float AttackRadius = 50.0f;
	bool bHitSucceed = GetWorld()->SweepSingleByChannel(OutHitResult, StartPos, EndPos,
		FQuat::Identity, ECC_GameTraceChannel1, FCollisionShape::MakeSphere(AttackRadius), CollisionParams);
	if (bHitSucceed)
	{
		FDamageEvent DamageEvent;
		// 맞은 액터의 TakeDamage 함수 호출
		OutHitResult.GetActor()->TakeDamage(AttackPower, DamageEvent, GetController(), this);
	}

#ifdef ENABLE_DRAW_DEBUG
	// 충돌 디버깅용
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

