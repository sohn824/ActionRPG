#include "PlayerCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "InputAction.h"
#include "InputActionValue.h"
#include "Camera/CameraComponent.h"

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

	// Stat Component
	StatComponent = CreateDefaultSubobject<UCharacterStatComponent>(TEXT("StatComponent"));
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
