#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/SphereComponent.h"
#include "Interaction/InteractableInterface.h"
#include "Engine/DataTable.h"
#include "Unia_Demo.generated.h"

class UDialogueManagerComponent;

// �� ���̾�α� �ܰ躰 ����Ʈ �䱸������ ��� ����ü
USTRUCT(BlueprintType)
struct FDialogueStepQuests
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	TArray<FString> RequiredQuestIDs;

	FDialogueStepQuests()
	{
		RequiredQuestIDs = TArray<FString>();
	}
};

UCLASS()
class DISTRICT_TEST_API AUnia_Demo : public ACharacter, public IInteractableInterface
{
	GENERATED_BODY()

public:
	AUnia_Demo();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

public:
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// IInteractableInterface implementation
	virtual void Interact_Implementation(AActor* Interactor) override;
	virtual bool CanInteract_Implementation(AActor* Interactor) override;
	virtual FString GetInteractionText_Implementation() override;
	virtual EInteractionType GetInteractionType_Implementation() override;

protected:
	// Components
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction")
	USphereComponent* InteractionSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dialogue")
	UDialogueManagerComponent* DialogueManager;

	// === BP���� ���� ������ ������Ƽ�� ===

	// NPC �⺻ ����
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Settings")
	FString NPCName = TEXT("Unia");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Settings")
	float InteractionRange = 150.0f;

	// �÷��̾� �ٶ󺸱� ����
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Look At Player")
	bool bLookAtPlayer = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Look At Player")
	float LookAtRange = 200.0f;  // �ٶ󺸱� ���� ����

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Look At Player")
	float LookAtSpeed = 2.0f;

	// ���̾�α� ���̺� ����
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue Tables")
	UDataTable* MainDialogueTable;  // ���� ���̾�α� ���̺�

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue Tables")
	UDataTable* RepeatDialogueTable;  // �ݺ� ��� ���̺� (����Ʈ ��� ��)

	// ���̾�α� ������ ���� (BP���� �迭�� ����)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue Sequence")
	TArray<FString> DialogueSequence;  // ������� ������ ���̾�α� ID��

	// �� ���̾�α� �ܰ躰 �ʿ��� ����Ʈ ID��
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Requirements")
	TArray<FDialogueStepQuests> RequiredQuestsByStep;

	// ���� ���� ���� (����Ǵ� ������)
	UPROPERTY(SaveGame, BlueprintReadWrite, Category = "Progress")
	int32 CurrentDialogueStep = 0;  // ���� ���̾�α� �ܰ�

	// Player Reference
	UPROPERTY(BlueprintReadOnly, Category = "References")
	APawn* PlayerPawn;

public:
	// ���̾�α� ���� �Լ���
	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	void StartDialogue(AActor* Interactor);

	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	bool CanProgressToNextStep();  // ���� �ܰ�� ���� �������� üũ

	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	FString GetCurrentDialogueID();  // ���� ������ ���̾�α� ID

	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	FString GetRandomRepeatDialogue();  // ���� �ݺ� ���

	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	void OnDialogueCompleted();  // ���̾�α� �Ϸ� �� ȣ��

	// ����Ʈ üũ (BP���� ����)
	UFUNCTION(BlueprintImplementableEvent, Category = "Quest")
	bool CheckQuestCompletion(const FString& QuestID) const;

	UFUNCTION(BlueprintImplementableEvent, Category = "Quest")
	bool CheckAllQuestsForStep(int32 StepIndex) const;

	// BP �̺�Ʈ��
	UFUNCTION(BlueprintImplementableEvent, Category = "Events")
	void OnDialogueStarted();

	UFUNCTION(BlueprintImplementableEvent, Category = "Events")
	void OnDialogueEnded();

	// ���� üũ �Լ���
	UFUNCTION(BlueprintPure, Category = "State")
	bool IsInDialogue() const;

	UFUNCTION(BlueprintPure, Category = "State")
	bool IsWaitingForQuest() const;  // ����Ʈ ��� ������

	UFUNCTION(BlueprintCallable, Category = "State")
	void SetDialogueState(bool bInDialogue);

private:
	void UpdateLookAtPlayer(float DeltaTime);
	void FindPlayerPawn();
	bool ShouldShowRepeatDialogue() const;
};