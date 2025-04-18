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

	// 습득할 아이템
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup")
	TSubclassOf<UItem> ItemClass;

	// 사용자 지정 아이템 이름 (지정하지 않으면 기본 이름 사용)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup")
	FString CustomItemName;

	// 사용자 지정 아이템 설명 (지정하지 않으면 기본 설명 사용)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup")
	FString CustomItemDescription;

	// 메시 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UStaticMeshComponent* MeshComponent;

	// 인터페이스 구현
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	void Interact(AActor* Interactor);
	virtual void Interact_Implementation(AActor* Interactor) override;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	bool CanInteract(AActor* Interactor);
	virtual bool CanInteract_Implementation(AActor* Interactor) override;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	FString GetInteractionText();
	virtual FString GetInteractionText_Implementation() override;

	// 습득 성공 여부
	UFUNCTION(BlueprintCallable, Category = "Pickup")
	bool PickupItem(AActor* Interactor);

	// 습득 효과음
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup")
	class USoundBase* PickupSound;
};