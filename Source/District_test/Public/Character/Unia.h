#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/SphereComponent.h"
#include "Interaction/InteractableInterface.h"
#include "Interaction/InteractionEnums.h"
#include "Engine/DataTable.h"
#include "Unia.generated.h"

class UDialogueManagerComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnUniaDialogueActivated, FString, DialogueID, UDataTable*, DataTable);

USTRUCT(BlueprintType)
struct FUniaQuestMacro
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Macro")
	FString QuestID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Macro")
	FString ProgressDialogueID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Macro")
	FString CompletionDialogueID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Macro")
	TArray<FString> ProgressDialoguesByStep;

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
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction")
	USphereComponent* InteractionSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dialogue")
	UDialogueManagerComponent* DialogueManager;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Settings")
	FString NPCName = TEXT("Unia");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Settings")
	float InteractionRange = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Settings")
	bool bLookAtPlayer = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Settings")
	float LookAtSpeed = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	UDataTable* MainStoryDialogueTable;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	UDataTable* UniaRandomDialogueTable;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	FString DialogueSceneID = TEXT("Unia_Default_001");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Progress")
	FString RequiredQuestID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Progress")
	FString CurrentStoryDialogueID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Macro")
	TArray<FUniaQuestMacro> QuestMacroList;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Macro")
	TArray<FString> CompletedMacroDialogues;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Following")
	bool bCanFollow = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Following")
	bool bIsFollowingPlayer;

	UPROPERTY(BlueprintReadOnly, Category = "Interaction")
	bool bPlayerInRange = false;

	APawn* PlayerPawn;

	UFUNCTION()
	void OnInteractionSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnInteractionSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex);

public:
	UPROPERTY(BlueprintAssignable, Category = "Dialogue Events")
	FOnUniaDialogueActivated OnUniaDialogueActivated;

	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	void StartDialogue(AActor* Interactor);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Dialogue")
	void OnDialogueStarted();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Interaction")
	void OnPlayerEnterRange(APawn* Player);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Interaction")
	void OnPlayerExitRange(APawn* Player);

	UFUNCTION(BlueprintCallable, Category = "Following")
	void SetFollowPlayer(bool bShouldFollow);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Following")
	void StartFollowingPlayer();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Following")
	void StopFollowingPlayer();

	UFUNCTION(BlueprintCallable, Category = "Look At")
	void LookAtPlayer();

	UFUNCTION(BlueprintCallable, Category = "Story Progress")
	void UpdateStoryProgress(const FString& NewStoryDialogueID);

	UFUNCTION(BlueprintPure, Category = "Dialogue State")
	bool IsInDialogue() const;

	UFUNCTION(BlueprintCallable, Category = "Dialogue State")
	void SetDialogueState(bool bInDialogue);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Quest")
	bool IsQuestActive(const FString& QuestID);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Quest")
	bool IsQuestCompleted(const FString& QuestID);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Quest")
	bool CheckQuestRequirement(const FString& QuestID);

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void HandlePlayerInteraction();

	UFUNCTION(BlueprintPure, Category = "Interaction")
	bool IsPlayerInRange() const { return bPlayerInRange; }

protected:
	bool ShouldShowMainStoryDialogue();
	FString GetCurrentStoryDialogueID();
	FString GetRandomDialogueID();
	FString GetQuestMacroDialogue();
	void UpdateLookAtPlayer(float DeltaTime);
	void FindPlayerPawn();
};