#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/SphereComponent.h"
#include "Interaction/InteractableInterface.h"
#include "Interaction/InteractionEnums.h"
#include "Engine/DataTable.h"
#include "Unia.generated.h"

class UDialogueManagerComponent;

// 우니아 대화 타입 열거형
UENUM(BlueprintType)
enum class EUniaDialogueType : uint8
{
	MainStory   UMETA(DisplayName = "Main Story"),
	QuestMacro  UMETA(DisplayName = "Quest Macro"),
	Random      UMETA(DisplayName = "Random")
};

// 퀘스트 매크로 구조체
USTRUCT(BlueprintType)
struct FUniaQuestMacro
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Macro")
	FString QuestID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Macro")
	FString ProgressDialogueID;    // "~~를 깨고 와"

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Macro")
	FString CompletionDialogueID;  // "잘했어!"

	// 기본 생성자
	FUniaQuestMacro()
	{
		QuestID = TEXT("");
		ProgressDialogueID = TEXT("");
		CompletionDialogueID = TEXT("");
	}
};

UCLASS()
class DISTRICT_TEST_API AUnia : public ACharacter, public IInteractableInterface
{
	GENERATED_BODY()

public:
	AUnia();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// IInteractableInterface 구현
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	void Interact(AActor* Interactor);
	virtual void Interact_Implementation(AActor* Interactor) override;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	bool CanInteract(AActor* Interactor);
	virtual bool CanInteract_Implementation(AActor* Interactor) override;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	FString GetInteractionText();
	virtual FString GetInteractionText_Implementation() override;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	EInteractionType GetInteractionType();
	virtual EInteractionType GetInteractionType_Implementation() override;

protected:
	// Components
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction")
	USphereComponent* InteractionSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dialogue")
	UDialogueManagerComponent* DialogueManager;

	// NPC Settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Settings")
	FString NPCName = TEXT("Unia");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Settings")
	float InteractionRange = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Settings")
	bool bLookAtPlayer = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Settings")
	float LookAtSpeed = 2.0f;

	// Dialogue Tables
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	UDataTable* MainStoryDialogueTable;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	UDataTable* UniaRandomDialogueTable;

	// Story Progress
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Progress")
	FString RequiredQuestID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Progress")
	FString CurrentStoryDialogueID;

	// Quest Macro System - 새로 추가된 부분
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Macro")
	TArray<FUniaQuestMacro> QuestMacroList;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Macro")
	TArray<FString> CompletedMacroDialogues;

	// Following System
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Following")
	bool bCanFollow = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Following")
	bool bIsFollowingPlayer;

	// Private
	APawn* PlayerPawn;

public:
	// Dialogue Functions
	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	void StartDialogue(AActor* Interactor);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Dialogue")
	void OnDialogueStarted();

	// Following Functions
	UFUNCTION(BlueprintCallable, Category = "Following")
	void SetFollowPlayer(bool bShouldFollow);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Following")
	void StartFollowingPlayer();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Following")
	void StopFollowingPlayer();

	// Look At Functions
	UFUNCTION(BlueprintCallable, Category = "Look At")
	void LookAtPlayer();

	// Story Progress Functions
	UFUNCTION(BlueprintCallable, Category = "Story Progress")
	void UpdateStoryProgress(const FString& NewStoryDialogueID);

	UFUNCTION(BlueprintPure, Category = "Dialogue State")
	bool IsInDialogue() const;

	UFUNCTION(BlueprintCallable, Category = "Dialogue State")
	void SetDialogueState(bool bInDialogue);

	// Quest Functions - 새로 추가된 부분
	UFUNCTION(BlueprintImplementableEvent, Category = "Quest")
	bool IsQuestActive(const FString& QuestID);

	UFUNCTION(BlueprintImplementableEvent, Category = "Quest")
	bool IsQuestCompleted(const FString& QuestID);

	UFUNCTION(BlueprintImplementableEvent, Category = "Quest")
	bool CheckQuestRequirement(const FString& QuestID);

protected:
	// Story Logic
	bool ShouldShowMainStoryDialogue();
	FString GetCurrentStoryDialogueID();
	FString GetRandomDialogueID();

	// Quest Macro Logic - 새로 추가된 부분
	FString GetQuestMacroDialogue();

	// Helper Functions
	void UpdateLookAtPlayer(float DeltaTime);
	void FindPlayerPawn();
};