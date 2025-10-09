#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HintWidget.generated.h"

class UButton;
class UTextBlock;
class UImage;

UCLASS(Abstract, Blueprintable)
class DISTRICT_TEST_API UHintWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Hint")
    void ResetHintWidget();

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
    UImage* Block0Image;

    UPROPERTY(meta = (BindWidget), BlueprintReadWrite)
    UImage* Block1Image;

    UPROPERTY(meta = (BindWidget), BlueprintReadWrite)
    UImage* Block2Image;

    UPROPERTY(meta = (BindWidget), BlueprintReadWrite)
    UImage* Block3Image;

    UPROPERTY(meta = (BindWidget), BlueprintReadWrite)
    UImage* Block4Image;

    UPROPERTY(meta = (BindWidget), BlueprintReadWrite)
    UImage* Block5Image;

    UPROPERTY(meta = (BindWidget), BlueprintReadWrite)
    UImage* HintBackgroundImage;

    UPROPERTY(meta = (BindWidget), BlueprintReadWrite)
    UTextBlock* CooldownText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hint Settings")
    float RevealCooldown = 90.0f;

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
    void HandleBlockClick(int32 BlockIndex);
    void UpdateBlockVisibility();
    void UpdateCooldownDisplay();
    void InitializeBlocks();
    void StartCooldown();
    void UpdateCooldown(float DeltaTime);

    UFUNCTION(BlueprintImplementableEvent, Category = "Hint")
    void OnBlockRevealed(int32 BlockIndex);

    UFUNCTION(BlueprintImplementableEvent, Category = "Hint")
    void OnRevealFailed(int32 BlockIndex, const FString& Reason);

private:
    TArray<UButton*> BlockButtons;
    TArray<UImage*> BlockImages;
};


