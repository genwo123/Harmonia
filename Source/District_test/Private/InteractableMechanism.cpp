// Fill out your copyright notice in the Description page of Project Settings.

#include "InteractableMechanism.h"
#include "InventoryComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"

AInteractableMechanism::AInteractableMechanism()
{
	PrimaryActorTick.bCanEverTick = true;

	// �޽� ������Ʈ ����
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	RootComponent = MeshComponent;

	// �⺻ ����
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
	// ��ȣ�ۿ� ���� ���� Ȯ��
	if (!bCanInteract)
	{
		return;
	}

	// ���谡 �ʿ��ϸ� Ȯ��
	if (bRequiresKey)
	{
		UInventoryComponent* InventoryComponent = Interactor->FindComponentByClass<UInventoryComponent>();
		if (!InventoryComponent)
		{
			OnInteractionFailed(Interactor);
			return;
		}

		// �κ��丮���� �ʿ��� ���� ã��
		UItem* Key = InventoryComponent->FindItemByName(RequiredKeyName);
		if (!Key)
		{
			// ���谡 ���� ���
			OnInteractionFailed(Interactor);
			return;
		}
	}

	// ��ȣ�ۿ� �Ҹ� ���
	if (InteractionSound)
	{
		UGameplayStatics::PlaySound2D(this, InteractionSound);
	}

	// ��ȣ�ۿ� ȿ�� ó�� (�������Ʈ���� ����)
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