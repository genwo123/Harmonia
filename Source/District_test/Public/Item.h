// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Item.generated.h"

/**
 * 기본 아이템 클래스 - 인벤토리에 저장 가능한 모든 아이템의 기본 클래스
 */
UCLASS(Blueprintable)
class DISTRICT_TEST_API UItem : public UObject
{
	GENERATED_BODY()

public:
	UItem();

	// 아이템 기본 속성
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

	// 아이템 사용 함수 - 자식 클래스에서 오버라이드
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Item")
	void Use(AActor* User);
	virtual void Use_Implementation(AActor* User);
};