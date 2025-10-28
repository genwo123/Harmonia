#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/SphereComponent.h"
#include "Interaction/InteractableInterface.h"
#include "Interaction/InteractionEnums.h"
#include "Engine/DataTable.h"
#include "AI/UniaAIController.h"
#include "Unia.generated.h"

class UDialogueManagerComponent;
class AUniaWaitSpot;
class UHamoina_GameInstance;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnUniaDialogueActivated, FString, DialogueID, UDataTable*, DataTable);

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

	UPROPERTY(BlueprintAssignable, Category = "Dialogue Events")
	FOnUniaDialogueActivated OnUniaDialogueActivated;

	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	void StartDialogue(AActor* Interactor);

	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	void EndDialogue();

	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	void OnDialogueWidgetClosed();

	UFUNCTION(BlueprintPure, Category = "Dialogue State")
	bool IsInDialogue() const;

	UFUNCTION(BlueprintCallable, Category = "Dialogue State")
	void SetDialogueState(bool bInDialogue);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue State")
	bool bLevelDialogueEnded = false;

	UFUNCTION(BlueprintCallable, Category = "Dialogue State")
	void SetLevelDialogueEnded(bool bEnded);

	UFUNCTION(BlueprintPure, Category = "Dialogue State")
	bool IsLevelDialogueEnded() const { return bLevelDialogueEnded; }

	UFUNCTION(BlueprintCallable, Category = "Dialogue State")
	void ResetLevelDialogueState();

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void HandlePlayerInteraction();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Dialogue")
	void OnDialogueStarted();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Interaction")
	void OnPlayerEnterRange(APawn* Player);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Interaction")
	void OnPlayerExitRange(APawn* Player);

	UFUNCTION(BlueprintCallable, Category = "Story Control")
	void EnableFollowing();

	UFUNCTION(BlueprintCallable, Category = "Following")
	void SetFollowPlayer(bool bShouldFollow);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Following")
	void StartFollowingPlayer();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Following")
	void StopFollowingPlayer();

	UFUNCTION(BlueprintCallable, Category = "AI Control")
	class AUniaAIController* GetUniaAIController() const;

	UFUNCTION(BlueprintCallable, Category = "AI Control")
	void SetAIFollowing(bool bShouldFollow);

	UFUNCTION(BlueprintCallable, Category = "AI Control")
	void MoveAIToLocation(const FVector& Location);

	UFUNCTION(BlueprintCallable, Category = "AI Control")
	bool MoveToWaitSpot(const FString& SpotID);

	UFUNCTION(BlueprintCallable, Category = "AI Control")
	void SetDialogueSpotMapping(const FString& DialogueID, const FString& SpotID);

	UFUNCTION(BlueprintCallable, Category = "AI Control")
	void CheckDialogueForAIAction(const FString& DialogueID);

	UFUNCTION(BlueprintCallable, Category = "Game Instance")
	void SaveStateToGameInstance();

	UFUNCTION(BlueprintCallable, Category = "Game Instance")
	void LoadStateFromGameInstance();

	UFUNCTION(BlueprintCallable, Category = "Look At")
	void LookAtPlayer();

	UFUNCTION(BlueprintPure, Category = "Interaction")
	bool IsPlayerInRange() const { return bPlayerInRange; }

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction")
	USphereComponent* InteractionSphere;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Settings")
	FString NPCName = TEXT("Unia");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Settings")
	float InteractionRange = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Settings")
	bool bLookAtPlayer = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Settings")
	float LookAtSpeed = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Settings")
	float LookAtRange = 600.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	UDataTable* UniaRandomDialogueTable;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	FString DialogueSceneID = TEXT("Level_Main_0_001");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	FString UniaRandomDialogueID = TEXT("Unia_Random_001");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Following")
	bool bCanFollow = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Following")
	bool bIsFollowingPlayer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Events")
	TArray<FString> FollowActivationDialogues;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Events")
	TMap<FString, FString> DialogueToSpotMap;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Settings")
	bool bStartWithFollowing = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Settings")
	FString DefaultWaitSpotID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Settings")
	float SpotArrivalThreshold = 100.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Interaction")
	bool bPlayerInRange = false;

	APawn* PlayerPawn;

	UFUNCTION()
	void OnInteractionSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnInteractionSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex);

	void UpdateLookAtPlayer(float DeltaTime);
	void FindPlayerPawn();
	void InitializeLevelSettings();
	void CheckSpotArrival();

private:
	AUniaWaitSpot* FindWaitSpot(const FString& SpotID);

	FString CurrentTargetSpotID;
	FTimerHandle SpotCheckTimer;
	FString CurrentLevelName;
};