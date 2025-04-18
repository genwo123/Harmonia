// Fill out your copyright notice in the Description page of Project Settings.

#include "PickupActor.h"
#include "InventoryComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"

APickupActor::APickupActor()
{
	PrimaryActorTick.bCanEverTick = true;

	// �޽� ������Ʈ ����
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	RootComponent = MeshComponent;

	// ���� Ȱ��ȭ
	MeshComponent->SetSimulatePhysics(true);
	MeshComponent->SetCollisionProfileName(TEXT("PhysicsActor"));
}

void APickupActor::BeginPlay()
{
	Super::BeginPlay();
}

void APickupActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void APickupActor::Interact_Implementation(AActor* Interactor)
{
	// ������ ���� �õ�
	if (PickupItem(Interactor))
	{
		// ���� ���� �� ���� ����
		Destroy();
	}
}

bool APickupActor::CanInteract_Implementation(AActor* Interactor)
{
	// �κ��丮�� �ִ��� Ȯ��
	UInventoryComponent* InventoryComponent = Interactor->FindComponentByClass<UInventoryComponent>();
	if (!InventoryComponent)
	{
		return false;
	}

	// �κ��丮�� ������ �ִ��� Ȯ��
	return InventoryComponent->HasRoomForItem();
}

FString APickupActor::GetInteractionText_Implementation()
{
	return FString::Printf(TEXT("pick up : %s"), *(!CustomItemName.IsEmpty() ? CustomItemName : TEXT("Item")));
}

bool APickupActor::PickupItem(AActor* Interactor)
{
	if (!ItemClass)
	{
		return false;
	}

	UInventoryComponent* InventoryComponent = Interactor->FindComponentByClass<UInventoryComponent>();
	if (!InventoryComponent)
	{
		return false;
	}

	// ������ �ν��Ͻ� ����
	UItem* NewItem = NewObject<UItem>(InventoryComponent, ItemClass);

	// ����� ���� �̸��� ������ ������ ����
	if (!CustomItemName.IsEmpty())
	{
		NewItem->Name = CustomItemName;
	}

	if (!CustomItemDescription.IsEmpty())
	{
		NewItem->Description = CustomItemDescription;
	}

	// �κ��丮�� ������ �߰�
	bool bPickedUp = InventoryComponent->AddItem(NewItem);

	// ���� �����ϸ� ȿ���� ���
	if (bPickedUp && PickupSound)
	{
		UGameplayStatics::PlaySound2D(this, PickupSound);
	}

	return bPickedUp;
}