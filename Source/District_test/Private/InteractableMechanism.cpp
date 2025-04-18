// Fill out your copyright notice in the Description page of Project Settings.

#include "InteractableMechanism.h"
#include "InventoryComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"

AInteractableMechanism::AInteractableMechanism()
{
	PrimaryActorTick.bCanEverTick = true;

	// 메시 컴포넌트 생성
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	RootComponent = MeshComponent;

	// 기본 설정
	InteractionText = TEXT("Interact");
	bRequiresKey = false;
	bCanInteract = true;
}

void AInteractableMechanism::BeginPlay()
{
	Super::BeginPlay();
}

void AInteractableMechanism::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AInteractableMechanism::Interact_Implementation(AActor* Interactor)
{
	// 상호작용 가능 여부 확인
	if (!bCanInteract)
	{
		return;
	}

	// 열쇠가 필요하면 확인
	if (bRequiresKey)
	{
		UInventoryComponent* InventoryComponent = Interactor->FindComponentByClass<UInventoryComponent>();
		if (!InventoryComponent)
		{
			OnInteractionFailed(Interactor);
			return;
		}

		// 인벤토리에서 필요한 열쇠 찾기
		UItem* Key = InventoryComponent->FindItemByName(RequiredKeyName);
		if (!Key)
		{
			// 열쇠가 없는 경우
			OnInteractionFailed(Interactor);
			return;
		}
	}

	// 상호작용 소리 재생
	if (InteractionSound)
	{
		UGameplayStatics::PlaySound2D(this, InteractionSound);
	}

	// 상호작용 효과 처리 (블루프린트에서 구현)
	OnInteractionSuccess(Interactor);
}

bool AInteractableMechanism::CanInteract_Implementation(AActor* Interactor)
{
	return bCanInteract;
}

FString AInteractableMechanism::GetInteractionText_Implementation()
{
	return InteractionText;
}