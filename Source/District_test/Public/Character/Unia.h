#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/SphereComponent.h"
#include "Interaction/InteractableInterface.h"
#include "Interaction/InteractionEnums.h"
#include "Engine/DataTable.h"
#include "AI/UniaAIController.h"
#include "Unia.generated.h"

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
	// ==================== AI Control ====================
	UFUNCTION(BlueprintCallable, Category = "Story Control")
	void EnableFollowing();

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

	UFUNCTION(BlueprintCallable, Category = "Level Setup")
	void InitializeLevelSettings();

	// ==================== Save/Load ====================
	UFUNCTION(BlueprintCallable, Category = "Game Instance")
	void SaveStateToGameInstance();

	UFUNCTION(BlueprintCallable, Category = "Game Instance")
	void LoadStateFromGameInstance();

	// ==================== Base Overrides ====================
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// ==================== IInteractableInterface ====================
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

	// ==================== Dialogue System ====================
	UPROPERTY(BlueprintAssignable, Category = "Dialogue Events")
	FOnUniaDialogueActivated OnUniaDialogueActivated;

	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	void StartDialogue(AActor* Interactor);

	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	void EndDialogue();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Dialogue")
	void OnDialogueStarted();

	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	void OnDialogueWidgetClosed();

	UFUNCTION(BlueprintPure, Category = "Dialogue State")
	bool IsInDialogue() const;

	UFUNCTION(BlueprintCallable, Category = "Dialogue State")
	void SetDialogueState(bool bInDialogue);

	// ==================== Player Interaction ====================
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Interaction")
	void OnPlayerEnterRange(APawn* Player);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Interaction")
	void OnPlayerExitRange(APawn* Player);

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void HandlePlayerInteraction();

	UFUNCTION(BlueprintPure, Category = "Interaction")
	bool IsPlayerInRange() const { return bPlayerInRange; }

	// ==================== Following System ====================
	UFUNCTION(BlueprintCallable, Category = "Following")
	void SetFollowPlayer(bool bShouldFollow);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Following")
	void StartFollowingPlayer();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Following")
	void StopFollowingPlayer();

	// ==================== Look At ====================
	UFUNCTION(BlueprintCallable, Category = "Look At")
	void LookAtPlayer();

protected:
	// ==================== Components ====================
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction")
	USphereComponent* InteractionSphere;

	// ==================== NPC Settings ====================
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

	// ==================== Dialogue Settings ====================
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	FString DialogueSceneID = TEXT("Level_Main_0_001");

	// ==================== Level Settings ====================
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level Settings")
	bool bStartWithFollowing = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level Settings")
	FString DefaultWaitSpotID = TEXT("");

	// ==================== Teleport Settings ====================
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Teleport Settings")
	float TeleportDistance = 1500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Teleport Settings")
	float TeleportCooldown = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Teleport Settings")
	bool bEnableTeleport = true;

	// ==================== Following Settings ====================
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Following")
	bool bCanFollow = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Following")
	bool bIsFollowingPlayer;

	// ==================== AI Events ====================
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Events")
	TArray<FString> FollowActivationDialogues;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Events")
	TMap<FString, FString> DialogueToSpotMap;

	// ==================== State ====================
	UPROPERTY(BlueprintReadOnly, Category = "Interaction")
	bool bPlayerInRange = false;

	UPROPERTY(BlueprintReadOnly, Category = "Spot Movement")
	FString CurrentTargetSpotID = TEXT("");

	APawn* PlayerPawn;

	// ==================== Overlap Events ====================
	UFUNCTION()
	void OnInteractionSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnInteractionSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex);

	// ==================== Helper Functions ====================
	void UpdateLookAtPlayer(float DeltaTime);
	void FindPlayerPawn();
	void CheckSpotArrival();

private:
	// ==================== Private Helpers ====================
	AUniaWaitSpot* FindWaitSpot(const FString& SpotID);

	// ==================== Timers ====================
	FTimerHandle DelayedActionTimer;
	FTimerHandle SpotCheckTimer;
	FTimerHandle TeleportCooldownTimer;

	// ==================== Settings ====================
	float SpotArrivalThreshold = 100.0f;
	bool bCanTeleport = true;
};