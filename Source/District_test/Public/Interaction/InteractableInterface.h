#pragma once
#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Interaction/InteractionEnums.h"
#include "InteractableInterface.generated.h"

UINTERFACE(MinimalAPI, Blueprintable)
class UInteractableInterface : public UInterface
{
    GENERATED_BODY()
};

class DISTRICT_TEST_API IInteractableInterface
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
    void Interact(AActor* Interactor);

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
    bool CanInteract(AActor* Interactor);

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
    FString GetInteractionText();

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
    EInteractionType GetInteractionType();

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
    void ShowInteractionWidget();

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
    void HideInteractionWidget();

    UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Quest")
    void OnQuestInteract(AActor* Interactor);
};