// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "InteractableInterface.h"
#include "InteractableMechanism.generated.h"

UCLASS()
class DISTRICT_TEST_API AInteractableMechanism : public AActor, public IInteractableInterface
{
	GENERATED_BODY()

public:
	AInteractableMechanism();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	// 상호작용 메시 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UStaticMeshComponent* MeshComponent;

	// 상호작용 텍스트
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	FString InteractionText;

	// 상호작용 소리
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	class USoundBase* InteractionSound;

	// 상호작용에 열쇠가 필요한지 여부
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	bool bRequiresKey;

	// 필요한 열쇠 이름
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction", meta = (EditCondition = "bRequiresKey"))
	FString RequiredKeyName;

	// 상호작용 가능 여부
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	bool bCanInteract;

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

	// 상호작용 효과 처리 (블루프린트에서 구현)
	UFUNCTION(BlueprintImplementableEvent, Category = "Interaction")
	void OnInteractionSuccess(AActor* Interactor);

	// 상호작용 실패 처리 (키가 없는 경우 등)
	UFUNCTION(BlueprintImplementableEvent, Category = "Interaction")
	void OnInteractionFailed(AActor* Interactor);
};