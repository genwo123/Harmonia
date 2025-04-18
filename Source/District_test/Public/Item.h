// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Item.generated.h"

/**
 * �⺻ ������ Ŭ���� - �κ��丮�� ���� ������ ��� �������� �⺻ Ŭ����
 */
UCLASS(Blueprintable)
class DISTRICT_TEST_API UItem : public UObject
{
	GENERATED_BODY()

public:
	UItem();

	// ������ �⺻ �Ӽ�
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	FString Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	FString Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	class UTexture2D* Icon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	bool bCanBeUsed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	int32 MaxStackSize;

	// ������ ��� �Լ� - �ڽ� Ŭ�������� �������̵�
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Item")
	void Use(AActor* User);
	virtual void Use_Implementation(AActor* User);
};