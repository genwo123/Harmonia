#include "PickupActor.h"
#include "InventoryComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

APickupActor::APickupActor()
{
	PrimaryActorTick.bCanEverTick = true;

	// 메시 컴포넌트 생성
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	RootComponent = MeshComponent;

	// 충돌 설정 변경 - 물리 시뮬레이션 비활성화
	MeshComponent->SetSimulatePhysics(false);
	MeshComponent->SetCollisionProfileName(TEXT("OverlapOnlyPawn"));

	// 상호작용 영역 추가
	InteractionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionSphere"));
	InteractionSphere->SetupAttachment(RootComponent);
	InteractionSphere->SetSphereRadius(100.0f);
	InteractionSphere->SetCollisionProfileName(TEXT("OverlapAll"));

	// 기본값 설정
	bDrawDebug = true;
}

void APickupActor::BeginPlay()
{
	Super::BeginPlay();

	// 아이템 클래스가 설정되어 있는지 확인
	if (!ItemClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("PickupActor %s has no ItemClass set!"), *GetName());
	}

	// 디버그 로그
	UE_LOG(LogTemp, Display, TEXT("PickupActor %s BeginPlay at location %s"),
		*GetName(), *GetActorLocation().ToString());
}

void APickupActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 디버그 시각화
	if (bDrawDebug)
	{
		// 상호작용 영역 표시
		DrawDebugSphere(
			GetWorld(),
			GetActorLocation(),
			InteractionSphere->GetScaledSphereRadius(),
			12,
			FColor::Purple,
			false,
			-1.0f,
			0,
			1.0f
		);
	}
}

void APickupActor::Interact_Implementation(AActor* Interactor)
{
	// 디버그 로그
	UE_LOG(LogTemp, Warning, TEXT("PickupActor::Interact called on %s by %s"),
		*GetName(), *Interactor->GetName());

	// 아이템 습득 시도
	if (PickupItem(Interactor))
	{
		// 컴포넌트 비활성화 (즉시)
		if (MeshComponent)
		{
			MeshComponent->SetVisibility(false);
			MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}

		if (InteractionSphere)
		{
			InteractionSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}

		// 습득 성공 이벤트 호출
		OnPickupSuccess(Interactor);

		// 액터 제거 (약간의 지연 사용)
		UE_LOG(LogTemp, Warning, TEXT("Destroying PickupActor %s after successful pickup"), *GetName());
		SetLifeSpan(0.1f);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to pick up item - inventory issue or no item class"));
	}
}

bool APickupActor::CanInteract_Implementation(AActor* Interactor)
{
	// 인벤토리가 있는지 확인
	UInventoryComponent* InventoryComponent = Interactor->FindComponentByClass<UInventoryComponent>();
	if (!InventoryComponent)
	{
		UE_LOG(LogTemp, Verbose, TEXT("Cannot interact - No inventory found on %s"), *Interactor->GetName());
		return false;
	}

	// 아이템 클래스가 설정되어 있는지 확인
	if (!ItemClass)
	{
		UE_LOG(LogTemp, Verbose, TEXT("Cannot interact - No ItemClass set on %s"), *GetName());
		return false;
	}

	// 인벤토리에 공간이 있는지 확인
	bool bHasRoom = InventoryComponent->HasRoomForItem();
	UE_LOG(LogTemp, Verbose, TEXT("Inventory has room: %s"), bHasRoom ? TEXT("Yes") : TEXT("No"));
	return bHasRoom;
}

FString APickupActor::GetInteractionText_Implementation()
{
	return FString::Printf(TEXT("Pick up: %s"), *(!CustomItemName.IsEmpty() ? CustomItemName : TEXT("Item")));
}

bool APickupActor::PickupItem(AActor* Interactor)
{
	if (!ItemClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("No ItemClass set for PickupActor %s"), *GetName());
		return false;
	}

	UInventoryComponent* InventoryComponent = Interactor->FindComponentByClass<UInventoryComponent>();
	if (!InventoryComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("No InventoryComponent found on interactor %s"), *Interactor->GetName());
		return false;
	}

	// 아이템 인스턴스 생성
	UItem* NewItem = NewObject<UItem>(InventoryComponent, ItemClass);
	if (!NewItem)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to create item instance"));
		return false;
	}

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

	if (bPickedUp)
	{
		UE_LOG(LogTemp, Display, TEXT("Item %s added to inventory of %s"),
			*NewItem->Name, *Interactor->GetName());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to add item to inventory"));
	}

	return bPickedUp;
}

void APickupActor::OnPickupSuccess(AActor* Interactor)
{
	// 블루프린트 이벤트 호출
	OnPickedUp(Interactor);

	// 습득 성공하면 효과음 재생
	if (PickupSound)
	{
		UGameplayStatics::PlaySound2D(this, PickupSound);
	}

	UE_LOG(LogTemp, Display, TEXT("PickupActor %s components disabled after successful pickup"), *GetName());
}

EInteractionType APickupActor::GetInteractionType_Implementation()
{
	return EInteractionType::Pickup;
}