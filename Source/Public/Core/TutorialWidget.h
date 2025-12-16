#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "TutorialWidget.generated.h"

UCLASS()
class DISTRICT_TEST_API UTutorialWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UPROPERTY(meta = (BindWidget))
    UTextBlock* TutorialText;

    UFUNCTION(BlueprintCallable, Category = "Tutorial")
    void ShowMessage(const FText& Message, float Duration);

    UFUNCTION(BlueprintCallable, Category = "Tutorial")
    void HideMessage();

protected:
    virtual void NativeConstruct() override;

private:
    FTimerHandle HideTimerHandle;
};