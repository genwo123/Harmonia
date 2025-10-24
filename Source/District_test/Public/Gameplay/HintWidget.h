#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Engine/DataTable.h"
#include "Interaction/InteractableMechanism.h"
#include "HintWidget.generated.h"

class UButton;
class UTextBlock;
class UImage;
class UDataTable;
class UHamoina_GameInstance;

UCLASS(Abstract, Blueprintable)
class DISTRICT_TEST_API UHintWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Hint")
    void InitializeHint(int32 InLevelNumber);

    UFUNCTION(BlueprintCallable, Category = "Hint")
    void ResetHintWidget();

    UFUNCTION(BlueprintCallable, Category = "Hint")
    void LoadHintImage(UDataTable* HintTable, int32 LevelNumber);

protected:
    UPROPERTY(meta = (BindWidget), BlueprintReadWrite)
    UButton* Block0Button;

    UPROPERTY(meta = (BindWidget), BlueprintReadWrite)
    UButton* Block1Button;

    UPROPERTY(meta = (BindWidget), BlueprintReadWrite)
    UButton* Block2Button;

    UPROPERTY(meta = (BindWidget), BlueprintReadWrite)
    UButton* Block3Button;

    UPROPERTY(meta = (BindWidget), BlueprintReadWrite)
    UButton* Block4Button;

    UPROPERTY(meta = (BindWidget), BlueprintReadWrite)
    UButton* Block5Button;

    UPROPERTY(meta = (BindWidget), BlueprintReadWrite)
    UImage* HintImage;

    UPROPERTY(meta = (BindWidget), BlueprintReadWrite)
    UTextBlock* CooldownText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hint Settings")
    float RevealCooldown = 60.0f;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    int32 CurrentLevelNumber = 0;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    TArray<bool> RevealedBlocks;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    float CurrentCooldown = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    bool bIsOnCooldown = false;

    UFUNCTION()
    void OnBlock0Clicked();
    UFUNCTION()
    void OnBlock1Clicked();
    UFUNCTION()
    void OnBlock2Clicked();
    UFUNCTION()
    void OnBlock3Clicked();
    UFUNCTION()
    void OnBlock4Clicked();
    UFUNCTION()
    void OnBlock5Clicked();

    void BindButtonEvents();
    void UnbindButtonEvents();
    void HandleBlockClick(int32 BlockIndex);
    void UpdateBlockVisibility();
    void UpdateCooldownDisplay();
    void InitializeBlocks();
    void StartCooldown();
    void UpdateCooldown(float DeltaTime);
    void LoadStateFromSaveGame();
    void SaveStateToSaveGame();

    UFUNCTION(BlueprintImplementableEvent, Category = "Hint")
    void OnBlockRevealed(int32 BlockIndex);

    UFUNCTION(BlueprintImplementableEvent, Category = "Hint")
    void OnRevealFailed(int32 BlockIndex, const FString& Reason);

    UFUNCTION(BlueprintCallable, Category = "Hint")
    void OnLevelReset();

    bool CheckIfLevelWasReset();

private:
    TArray<UButton*> BlockButtons;
    UHamoina_GameInstance* GameInstance;
    bool bIsInitialized = false;
    bool bButtonsBound = false;
};