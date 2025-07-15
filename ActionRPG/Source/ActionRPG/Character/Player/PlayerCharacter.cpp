#include "PlayerCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "InputAction.h"
#include "InputActionValue.h"
#include "Camera/CameraComponent.h"
#include "Engine/DamageEvents.h"

APlayerCharacter::APlayerCharacter()
{
	GetMesh()->SetRelativeLocation(FVector(0.0f, 0.0f, -98.0f));

	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// 캐릭터가 컨트롤러 rotate에 영향받지 않도록
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 500.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Input Mapping Context
	static ConstructorHelpers::FObjectFinder<UInputMappingContext> IMC_Ref(
		TEXT("/Game/ActionRPG/Input/IMC_Player"));
	if (IMC_Ref.Succeeded())
	{
		InputMappingContext = IMC_Ref.Object;
	}

	// Input Action
	static ConstructorHelpers::FObjectFinder<UInputAction> JumpAction_Ref(
		TEXT("/Game/ActionRPG/Input/Actions/IA_PlayerJump"));
	if (JumpAction_Ref.Succeeded())
	{
		JumpAction = JumpAction_Ref.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputAction> MoveAction_Ref(
		TEXT("/Game/ActionRPG/Input/Actions/IA_PlayerMove"));
	if (MoveAction_Ref.Succeeded())
	{
		MoveAction = MoveAction_Ref.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputAction> LookAction_Ref(
		TEXT("/Game/ActionRPG/Input/Actions/IA_PlayerLook"));
	if (LookAction_Ref.Succeeded())
	{
		LookAction = LookAction_Ref.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputAction> AttackAction_Ref(
		TEXT("/Game/ActionRPG/Input/Actions/IA_PlayerAttack"));
	if (AttackAction_Ref.Succeeded())
	{
		AttackAction = AttackAction_Ref.Object;
	}

	// Animation Blueprint
	static ConstructorHelpers::FClassFinder<UAnimInstance> AnimBPClass(TEXT("/Game/ActionRPG/Character/ABP_PlayerCharacter"));
	if (AnimBPClass.Succeeded())
	{
		AnimBlueprintClass = AnimBPClass.Class;
		GetMesh()->SetAnimInstanceClass(AnimBlueprintClass);
	}

	// Attack Animation Montage
	static ConstructorHelpers::FObjectFinder<UAnimMontage> AttackMontage_Ref(
		TEXT("/Game/ActionRPG/Character/Montage_Attack"));
	if (AttackMontage_Ref.Succeeded())
	{
		AttackMontage = AttackMontage_Ref.Object;
	}
}

void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		UEnhancedInputLocalPlayerSubsystem* Subsystem =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer());

		if (Subsystem)
		{
			Subsystem->AddMappingContext(InputMappingContext, 0);
		}
	}

	// Set up action bindings
	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (EnhancedInputComponent)
	{
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Move);

		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Look);

		EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Attack);
	}
}

void APlayerCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void APlayerCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void APlayerCharacter::Attack(const FInputActionValue& Value)
{
	// 공중 공격 불가
	if (GetCharacterMovement()->IsFalling())
	{
		return;
	}

	// 아직 공격 시작 안했으면 콤보 시작
	if (bIsAttacking == false)
	{
		bIsAttacking = true;
		StartComboSection();
		return;
	}

	// 콤보 타이머가 돌고 있을 때 입력이 들어왔다면 콤보 예약 성공 처리
	if (ComboTimerHandle.IsValid())
	{
		bNextComboReserved = true;
	}
	else
	{
		bNextComboReserved = false;
	}
}

void APlayerCharacter::StartComboSection()
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
	EndDelegate.BindUObject(this, &APlayerCharacter::ComboActionEndCallback);
	AnimInstance->Montage_SetEndDelegate(EndDelegate, AttackMontage);
	// 콤보 타이머 시작
	ComboTimerHandle.Invalidate();
	// 콤보 입력을 넣을 수 있는 시간
	const float ComboEffectiveTime = 0.3f;
	GetWorld()->GetTimerManager().SetTimer(ComboTimerHandle, this,
		&APlayerCharacter::ComboTimerCallback, ComboEffectiveTime, false);

	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_None);
}

FName APlayerCharacter::GetComboSectionName(const int32& ComboIndex) const
{
	return FName(*FString::Printf(TEXT("Attack%d"), ComboIndex + 1));
}

void APlayerCharacter::ComboActionEndCallback(class UAnimMontage* TargetMontage, bool bIsProperlyEnded)
{
	CurrentComboIndex = 0;
	bIsAttacking = false;
	bNextComboReserved = false;

	// 다시 방향키 입력 받을 수 있도록 MovementMode 복구
	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);
}

void APlayerCharacter::ComboTimerCallback()
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
			&APlayerCharacter::ComboTimerCallback, ComboEffectiveTime, false);

		bNextComboReserved = false;
	}
}

// 공격 판정을 하는 Animation Notify에서 호출될 콜백 함수
void APlayerCharacter::Notify_AttackHitCheck()
{
	// 충돌 지점과 해당 지점에서의 여러 정보를 포함하는 구조체
	// SweepSingleByChannel()과 같은 함수에 전달하면 충돌 판정을 하고 관련 정보를 반환함
	FHitResult OutHitResult;

	// 충돌 함수에 전달되는 매개변수 구조체
	FCollisionQueryParams CollisionParams(SCENE_QUERY_STAT(Attack), false, this);

	const float AttackRange = 100.0f;
	const float AttackDamage = 100.0f;
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
		OutHitResult.GetActor()->TakeDamage(AttackDamage, DamageEvent, GetController(), this);
	}

#ifdef ENABLE_DRAW_DEBUG
	// 충돌 디버깅용
	FVector CapsuleOrigin = StartPos + (EndPos - StartPos) * 0.5f;
	const float CapsuleHalfHeight = AttackRange * 0.5f;
	FColor DrawColor = bHitSucceed ? FColor::Green : FColor::Red;

	DrawDebugCapsule(GetWorld(), CapsuleOrigin, CapsuleHalfHeight, AttackRadius,
		FRotationMatrix::MakeFromZ(GetActorForwardVector()).ToQuat(), DrawColor, false, 5.f);
#endif //ENABLE_DRAW_DEBUG
}
