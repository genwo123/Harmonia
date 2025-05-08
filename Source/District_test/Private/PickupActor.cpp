#include "PickupActor.h"
#include "InventoryComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

APickupActor::APickupActor()
{
	PrimaryActorTick.bCanEverTick = true;

	// �޽� ������Ʈ ����
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	RootComponent = MeshComponent;

	// �浹 ���� ���� - ���� �ùķ��̼� ��Ȱ��ȭ
	MeshComponent->SetSimulatePhysics(false);
	MeshComponent->SetCollisionProfileName(TEXT("OverlapOnlyPawn"));

	// ��ȣ�ۿ� ���� �߰�
	InteractionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionSphere"));
	InteractionSphere->SetupAttachment(RootComponent);
	InteractionSphere->SetSphereRadius(100.0f);
	InteractionSphere->SetCollisionProfileName(TEXT("OverlapAll"));

	// �⺻�� ����
	bDrawDebug = true;
}

void APickupActor::BeginPlay()
{
	Super::BeginPlay();

	// ������ Ŭ������ �����Ǿ� �ִ��� Ȯ��
	if (!ItemClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("PickupActor %s has no ItemClass set!"), *GetName());
	}

	// ����� �α�
	UE_LOG(LogTemp, Display, TEXT("PickupActor %s BeginPlay at location %s"),
		*GetName(), *GetActorLocation().ToString());
}

void APickupActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// ����� �ð�ȭ
	if (bDrawDebug)
	{
		// ��ȣ�ۿ� ���� ǥ��
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
	// ����� �α�
	UE_LOG(LogTemp, Warning, TEXT("PickupActor::Interact called on %s by %s"),
		*GetName(), *Interactor->GetName());

	// ������ ���� �õ�
	if (PickupItem(Interactor))
	{
		// ������Ʈ ��Ȱ��ȭ (���)
		if (MeshComponent)
		{
			MeshComponent->SetVisibility(false);
			MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}

		if (InteractionSphere)
		{
			InteractionSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}

		// ���� ���� �̺�Ʈ ȣ��
		OnPickupSuccess(Interactor);

		// ���� ���� (�ణ�� ���� ���)
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
	// �κ��丮�� �ִ��� Ȯ��
	UInventoryComponent* InventoryComponent = Interactor->FindComponentByClass<UInventoryComponent>();
	if (!InventoryComponent)
	{
		UE_LOG(LogTemp, Verbose, TEXT("Cannot interact - No inventory found on %s"), *Interactor->GetName());
		return false;
	}

	// ������ Ŭ������ �����Ǿ� �ִ��� Ȯ��
	if (!ItemClass)
	{
		UE_LOG(LogTemp, Verbose, TEXT("Cannot interact - No ItemClass set on %s"), *GetName());
		return false;
	}

	// �κ��丮�� ������ �ִ��� Ȯ��
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

	// ������ �ν��Ͻ� ����
	UItem* NewItem = NewObject<UItem>(InventoryComponent, ItemClass);
	if (!NewItem)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to create item instance"));
		return false;
	}

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
	// �������Ʈ �̺�Ʈ ȣ��
	OnPickedUp(Interactor);

	// ���� �����ϸ� ȿ���� ���
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