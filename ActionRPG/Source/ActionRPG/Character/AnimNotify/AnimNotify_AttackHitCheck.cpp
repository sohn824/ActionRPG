#include "AnimNotify_AttackHitCheck.h"
#include "../Player/PlayerCharacter.h"

void UAnimNotify_AttackHitCheck::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	Super::Notify(MeshComp, Animation);

	APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(MeshComp->GetOwner());

	if (PlayerCharacter)
	{
		PlayerCharacter->Notify_AttackHitCheck();
	}
}
