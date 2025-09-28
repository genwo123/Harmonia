#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Blueprint/UserWidget.h"
#include "TutorialTrigger.generated.h"

USTRUCT(BlueprintType)
struct DISTRICT_TEST_API FTutorialMessage
{
    GENERATED_BODY()

    // 표시할 텍스트
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
    FText MessageText;

    // 표시 시간 (초)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
    float DisplayDuration;

    FTutorialMessage()
    {
        MessageText = FText::FromString(TEXT("튜토리얼 텍스트"));
        DisplayDuration = 3.0f;
    }
};

UCLASS()
class DISTRICT_TEST_API ATutorialTrigger : public AActor
{
    GENERATED_BODY()

public:
    ATutorialTrigger();

protected:
    virtual void BeginPlay() override;

public:
    // 컴포넌트들
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UBoxComponent* TriggerBox;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* VisualMesh;

    // 튜토리얼 메시지들 (BP에서 수정 가능)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
    TArray<FTutorialMessage> TutorialMessages;

    // 튜토리얼 위젯 클래스 (BP에서 설정)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
    TSubclassOf<UUserWidget> TutorialWidgetClass;

    // 일회용 트리거 여부
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
    bool bIsOneTimeUse;

    // 트리거 활성화 여부
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
    bool bIsActive;

    // 디버그용 시각적 표시
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    bool bShowDebugMesh;

protected:
    // 이미 트리거됨 여부
    UPROPERTY(BlueprintReadOnly, Category = "Tutorial")
    bool bHasTriggered;

    // 현재 메시지 인덱스
    UPROPERTY(BlueprintReadOnly, Category = "Tutorial")
    int32 CurrentMessageIndex;

    // 튜토리얼 위젯 참조
    UPROPERTY(BlueprintReadOnly, Category = "Tutorial")
    UUserWidget* TutorialWidget;

    // 타이머 핸들
    FTimerHandle MessageTimerHandle;

public:
    // 트리거 이벤트
    UFUNCTION()
    void OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
        UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    // 튜토리얼 시작
    UFUNCTION(BlueprintCallable, Category = "Tutorial")
    void StartTutorial();

    // 다음 메시지 표시
    UFUNCTION(BlueprintCallable, Category = "Tutorial")
    void ShowNextMessage();

    // 튜토리얼 종료
    UFUNCTION(BlueprintCallable, Category = "Tutorial")
    void EndTutorial();

    // 트리거 리셋 (테스트용)
    UFUNCTION(BlueprintCallable, Category = "Tutorial")
    void ResetTrigger();

    // 튜토리얼 위젯 찾기/생성
    UFUNCTION(BlueprintCallable, Category = "Tutorial")
    UUserWidget* GetOrCreateTutorialWidget();

    // 메시지 표시 (Blueprint에서 오버라이드 가능)
    UFUNCTION(BlueprintImplementableEvent, Category = "Tutorial")
    void OnShowMessage(const FText& MessageText, float Duration);

    // 튜토리얼 완료 이벤트 (Blueprint에서 오버라이드 가능)
    UFUNCTION(BlueprintImplementableEvent, Category = "Tutorial")
    void OnTutorialCompleted();

private:
    void ShowCurrentMessage();
    void HideCurrentMessage();
};