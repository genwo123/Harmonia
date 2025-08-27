#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/SphereComponent.h"
#include "Interaction/InteractableInterface.h"
#include "Engine/DataTable.h"
#include "Unia_Demo.generated.h"

class UDialogueManagerComponent;

// 각 다이얼로그 단계별 퀘스트 요구사항을 담는 구조체
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

	// === BP에서 설정 가능한 프로퍼티들 ===

	// NPC 기본 설정
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Settings")
	FString NPCName = TEXT("Unia");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Settings")
	float InteractionRange = 150.0f;

	// 플레이어 바라보기 설정
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Look At Player")
	bool bLookAtPlayer = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Look At Player")
	float LookAtRange = 200.0f;  // 바라보기 시작 범위

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Look At Player")
	float LookAtSpeed = 2.0f;

	// 다이얼로그 테이블 설정
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue Tables")
	UDataTable* MainDialogueTable;  // 메인 다이얼로그 테이블

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue Tables")
	UDataTable* RepeatDialogueTable;  // 반복 대사 테이블 (퀘스트 대기 중)

	// 다이얼로그 시퀀스 설정 (BP에서 배열로 설정)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue Sequence")
	TArray<FString> DialogueSequence;  // 순서대로 진행할 다이얼로그 ID들

	// 각 다이얼로그 단계별 필요한 퀘스트 ID들
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Requirements")
	TArray<FDialogueStepQuests> RequiredQuestsByStep;

	// 현재 진행 상태 (저장되는 데이터)
	UPROPERTY(SaveGame, BlueprintReadWrite, Category = "Progress")
	int32 CurrentDialogueStep = 0;  // 현재 다이얼로그 단계

	// Player Reference
	UPROPERTY(BlueprintReadOnly, Category = "References")
	APawn* PlayerPawn;

public:
	// 다이얼로그 관련 함수들
	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	void StartDialogue(AActor* Interactor);

	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	bool CanProgressToNextStep();  // 다음 단계로 진행 가능한지 체크

	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	FString GetCurrentDialogueID();  // 현재 실행할 다이얼로그 ID

	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	FString GetRandomRepeatDialogue();  // 랜덤 반복 대사

	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	void OnDialogueCompleted();  // 다이얼로그 완료 시 호출

	// 퀘스트 체크 (BP에서 구현)
	UFUNCTION(BlueprintImplementableEvent, Category = "Quest")
	bool CheckQuestCompletion(const FString& QuestID) const;

	UFUNCTION(BlueprintImplementableEvent, Category = "Quest")
	bool CheckAllQuestsForStep(int32 StepIndex) const;

	// BP 이벤트들
	UFUNCTION(BlueprintImplementableEvent, Category = "Events")
	void OnDialogueStarted();

	UFUNCTION(BlueprintImplementableEvent, Category = "Events")
	void OnDialogueEnded();

	// 상태 체크 함수들
	UFUNCTION(BlueprintPure, Category = "State")
	bool IsInDialogue() const;

	UFUNCTION(BlueprintPure, Category = "State")
	bool IsWaitingForQuest() const;  // 퀘스트 대기 중인지

	UFUNCTION(BlueprintCallable, Category = "State")
	void SetDialogueState(bool bInDialogue);

private:
	void UpdateLookAtPlayer(float DeltaTime);
	void FindPlayerPawn();
	bool ShouldShowRepeatDialogue() const;
};