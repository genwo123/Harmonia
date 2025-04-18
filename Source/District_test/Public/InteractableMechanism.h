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

	// ��ȣ�ۿ� �޽� ������Ʈ
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UStaticMeshComponent* MeshComponent;

	// ��ȣ�ۿ� �ؽ�Ʈ
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	FString InteractionText;

	// ��ȣ�ۿ� �Ҹ�
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	class USoundBase* InteractionSound;

	// ��ȣ�ۿ뿡 ���谡 �ʿ����� ����
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	bool bRequiresKey;

	// �ʿ��� ���� �̸�
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction", meta = (EditCondition = "bRequiresKey"))
	FString RequiredKeyName;

	// ��ȣ�ۿ� ���� ����
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	bool bCanInteract;

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

	// ��ȣ�ۿ� ȿ�� ó�� (�������Ʈ���� ����)
	UFUNCTION(BlueprintImplementableEvent, Category = "Interaction")
	void OnInteractionSuccess(AActor* Interactor);

	// ��ȣ�ۿ� ���� ó�� (Ű�� ���� ��� ��)
	UFUNCTION(BlueprintImplementableEvent, Category = "Interaction")
	void OnInteractionFailed(AActor* Interactor);
};