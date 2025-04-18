// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "InteractableInterface.h"
#include "Item.h"
#include "PickupActor.generated.h"

UCLASS()
class DISTRICT_TEST_API APickupActor : public AActor, public IInteractableInterface
{
	GENERATED_BODY()

public:
	APickupActor();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	// ������ ������
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup")
	TSubclassOf<UItem> ItemClass;

	// ����� ���� ������ �̸� (�������� ������ �⺻ �̸� ���)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup")
	FString CustomItemName;

	// ����� ���� ������ ���� (�������� ������ �⺻ ���� ���)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup")
	FString CustomItemDescription;

	// �޽� ������Ʈ
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UStaticMeshComponent* MeshComponent;

	// �������̽� ����
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	void Interact(AActor* Interactor);
	virtual void Interact_Implementation(AActor* Interactor) override;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	bool CanInteract(AActor* Interactor);
	virtual bool CanInteract_Implementation(AActor* Interactor) override;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	FString GetInteractionText();
	virtual FString GetInteractionText_Implementation() override;

	// ���� ���� ����
	UFUNCTION(BlueprintCallable, Category = "Pickup")
	bool PickupItem(AActor* Interactor);

	// ���� ȿ����
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup")
	class USoundBase* PickupSound;
};