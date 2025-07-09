#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Character/HamoniaCharacter.h"
#include "DialogueTrigger.generated.h"

UCLASS()
class DISTRICT_TEST_API ADialogueTrigger : public AActor
{
	GENERATED_BODY()

public:
	ADialogueTrigger();

protected:
	virtual void BeginPlay() override;

	// Components
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UBoxComponent* TriggerBox;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* DebugMesh;

	// Ŭ���� ���� ���� ��������Ʈ �߰�
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDialogueTriggerActivated, FString, DialogueID, UDataTable*, DataTable);

	// Dialogue Settings (���� �����ϸ鼭 �߰�)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	FString DialogueSceneID = "Scene01_001"; // �⺻�� �߰�

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	bool bOneTimeOnly = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	bool bAutoGetFromCharacter = true;

	// ���� �߰�
	UPROPERTY(BlueprintAssignable, Category = "Dialogue Events")
	FOnDialogueTriggerActivated OnDialogueTriggerActivated;

	// State
	UPROPERTY(BlueprintReadOnly, Category = "State")
	bool bHasTriggered = false;

	// Quest Requirements (for future use)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	FString RequiredQuestID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	bool bRequireQuestComplete = false;

	// Events
	UFUNCTION()
	void OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

public:
	// Functions
	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	void TriggerDialogue(AHamoniaCharacter* Character);

	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	void GetDefaultDialogueSettings();

	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	bool CanTrigger(AHamoniaCharacter* Character);

	UFUNCTION(BlueprintCallable, Category = "Debug")
	void SetDebugMeshVisible(bool bVisible);
};