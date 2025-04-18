// Fill out your copyright notice in the Description page of Project Settings.
#pragma once
#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "InteractableInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI, Blueprintable)
class UInteractableInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 상호작용 가능한 객체들이 구현해야 하는 인터페이스
 */
class DISTRICT_TEST_API IInteractableInterface
{
	GENERATED_BODY()

public:
	// 상호작용 시 호출될 함수
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	void Interact(AActor* Interactor);

	// 상호작용 가능한지 확인하는 함수
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	bool CanInteract(AActor* Interactor);

	// 상호작용 텍스트 가져오기 (예: "열기", "줍기" 등)
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	FString GetInteractionText();
};