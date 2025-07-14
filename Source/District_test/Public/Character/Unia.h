// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Interaction/InteractableInterface.h"
#include "Components/SphereComponent.h"
#include "Engine/DataTable.h"
#include "Unia.generated.h"

// Forward Declarations
class UDialogueManagerComponent;

UCLASS()
class DISTRICT_TEST_API AUnia : public ACharacter, public IInteractableInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AUnia();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Components
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction")
	USphereComponent* InteractionSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dialogue")
	UDialogueManagerComponent* DialogueManager;

	// NPC Settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Settings")
	FString NPCName = TEXT("Unia");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Settings")
	float InteractionRange = 150.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Settings")
	bool bCanFollow = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Settings")
	bool bLookAtPlayer = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Settings")
	float LookAtSpeed = 2.0f;

	// Dialogue Tables
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	UDataTable* MainStoryDialogueTable; // 하모니아 데이터 테이블 (스크립트 순서)

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	UDataTable* UniaRandomDialogueTable; // 우니아 전용 테이블 (랜덤 선택)

	// Current Story Progress
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
	FString CurrentStoryDialogueID = TEXT(""); // 현재 진행 중인 스토리 대화 ID

	// Quest Settings (BP_DialogueTrigger 방식과 동일)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	FString RequiredQuestID = TEXT("");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	bool bRequireQuestCompletion = false;

	// Player Reference
	UPROPERTY(BlueprintReadOnly, Category = "NPC")
	APawn* PlayerPawn;

	// AI State
	UPROPERTY(BlueprintReadWrite, Category = "AI")
	bool bIsFollowingPlayer = false;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// IInteractableInterface Implementation
	virtual void Interact_Implementation(AActor* Interactor) override;
	virtual bool CanInteract_Implementation(AActor* Interactor) override;
	virtual FString GetInteractionText_Implementation() override;
	virtual EInteractionType GetInteractionType_Implementation() override;

	// NPC Core Functions
	UFUNCTION(BlueprintCallable, Category = "NPC")
	void StartDialogue(AActor* Interactor);

	UFUNCTION(BlueprintCallable, Category = "NPC")
	void SetFollowPlayer(bool bShouldFollow);

	UFUNCTION(BlueprintCallable, Category = "NPC")
	void LookAtPlayer();

	// Dialogue Logic
	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	bool ShouldShowMainStoryDialogue();

	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	FString GetCurrentStoryDialogueID();

	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	FString GetRandomDialogueID();

	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	void UpdateStoryProgress(const FString& NewStoryDialogueID);

	// Blueprint Events
	UFUNCTION(BlueprintImplementableEvent, Category = "NPC")
	void OnDialogueStarted();

	UFUNCTION(BlueprintImplementableEvent, Category = "NPC")
	void OnDialogueEnded();

	UFUNCTION(BlueprintImplementableEvent, Category = "AI")
	void StartFollowingPlayer();

	UFUNCTION(BlueprintImplementableEvent, Category = "AI")
	void StopFollowingPlayer();

	UFUNCTION(BlueprintImplementableEvent, Category = "Quest")
	bool CheckQuestRequirement(const FString& QuestID);

	// Getter Functions
	UFUNCTION(BlueprintPure, Category = "NPC")
	FString GetNPCName() const { return NPCName; }

	UFUNCTION(BlueprintPure, Category = "NPC")
	bool IsInDialogue() const;

	UFUNCTION(BlueprintCallable, Category = "NPC")
	void SetDialogueState(bool bInDialogue);

private:
	void UpdateLookAtPlayer(float DeltaTime);
	void FindPlayerPawn();
};