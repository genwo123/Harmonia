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
 * ��ȣ�ۿ� ������ ��ü���� �����ؾ� �ϴ� �������̽�
 */
class DISTRICT_TEST_API IInteractableInterface
{
	GENERATED_BODY()

public:
	// ��ȣ�ۿ� �� ȣ��� �Լ�
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	void Interact(AActor* Interactor);

	// ��ȣ�ۿ� �������� Ȯ���ϴ� �Լ�
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	bool CanInteract(AActor* Interactor);

	// ��ȣ�ۿ� �ؽ�Ʈ �������� (��: "����", "�ݱ�" ��)
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	FString GetInteractionText();
};