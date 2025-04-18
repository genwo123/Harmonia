// Fill out your copyright notice in the Description page of Project Settings.

#include "PickupActor.h"
#include "InventoryComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"

APickupActor::APickupActor()
{
	PrimaryActorTick.bCanEverTick = true;

	// 메시 컴포넌트 생성
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	RootComponent = MeshComponent;

	// 물리 활성화
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
	// 아이템 습득 시도
	if (PickupItem(Interactor))
	{
		// 습득 성공 시 액터 제거
		Destroy();
	}
}

bool APickupActor::CanInteract_Implementation(AActor* Interactor)
{
	// 인벤토리가 있는지 확인
	UInventoryComponent* InventoryComponent = Interactor->FindComponentByClass<UInventoryComponent>();
	if (!InventoryComponent)
	{
		return false;
	}

	// 인벤토리에 공간이 있는지 확인
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

	// 아이템 인스턴스 생성
	UItem* NewItem = NewObject<UItem>(InventoryComponent, ItemClass);

	// 사용자 지정 이름과 설명이 있으면 설정
	if (!CustomItemName.IsEmpty())
	{
		NewItem->Name = CustomItemName;
	}

	if (!CustomItemDescription.IsEmpty())
	{
		NewItem->Description = CustomItemDescription;
	}

	// 인벤토리에 아이템 추가
	bool bPickedUp = InventoryComponent->AddItem(NewItem);

	// 습득 성공하면 효과음 재생
	if (bPickedUp && PickupSound)
	{
		UGameplayStatics::PlaySound2D(this, PickupSound);
	}

	return bPickedUp;
}