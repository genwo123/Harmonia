#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Blueprint/UserWidget.h"
#include "Engine/DataTable.h"
#include "TutorialManager.generated.h"

class UTutorialWidget;

USTRUCT(BlueprintType)
struct DISTRICT_TEST_API FTutorialMessageData : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
    TArray<FText> Messages;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
    TArray<float> DisplayDurations;

    FTutorialMessageData()
    {
        Messages.Add(FText::FromString(TEXT("Tutorial Message 1")));
        DisplayDurations.Add(3.0f);
    }
};

UCLASS()
class DISTRICT_TEST_API ATutorialManager : public AActor
{
    GENERATED_BODY()

public:
    ATutorialManager();

protected:
    virtual void BeginPlay() override;

public:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UBoxComponent* TriggerBox;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* VisualMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
    UDataTable* TutorialMessageDataTable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial", meta = (DisplayName = "MessageGroup"))
    int32 MessageGroupIndex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
    TSubclassOf<UUserWidget> TutorialWidgetClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
    bool bIsOneTimeUse;

    UFUNCTION(BlueprintCallable, Category = "Tutorial")
    void StartTutorialWithGroup(int32 GroupIndex);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
    bool bIsActive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    bool bShowDebugMesh;

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Tutorial")
    bool bHasTriggered;

    UPROPERTY(BlueprintReadOnly, Category = "Tutorial")
    int32 CurrentMessageIndex;

    UPROPERTY(BlueprintReadOnly, Category = "Tutorial")
    UUserWidget* TutorialWidget;

    FTutorialMessageData CurrentMessageData;
    FTimerHandle MessageTimerHandle;

public:
    UFUNCTION()
    void OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
        UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION(BlueprintCallable, Category = "Tutorial")
    void StartTutorial();

    UFUNCTION(BlueprintCallable, Category = "Tutorial")
    void ShowNextMessage();

    UFUNCTION(BlueprintCallable, Category = "Tutorial")
    void EndTutorial();

    UFUNCTION(BlueprintCallable, Category = "Tutorial")
    void ResetTrigger();

    UFUNCTION(BlueprintCallable, Category = "Tutorial")
    UUserWidget* GetOrCreateTutorialWidget();

private:
    bool LoadMessageGroupFromDataTable();
    void ShowCurrentMessage();
    void HideCurrentMessage();
};