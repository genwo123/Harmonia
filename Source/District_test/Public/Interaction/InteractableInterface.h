// Fill out your copyright notice in the Description page of Project Settings.
#pragma once
#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Interaction/InteractionEnums.h"
#include "InteractableInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI, Blueprintable)
class UInteractableInterface : public UInterface
{
    GENERATED_BODY()
};

/**
 * Interface for interactable objects
 */
class DISTRICT_TEST_API IInteractableInterface
{
    GENERATED_BODY()
public:
    // Function called when interacted with
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
    void Interact(AActor* Interactor);

    // Function to check if interaction is possible
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
    bool CanInteract(AActor* Interactor);

    // Get interaction text (e.g. "Open", "Pick up")
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
    FString GetInteractionText();

    // Get interaction type 
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
    EInteractionType GetInteractionType();

    // Quest interaction event - called after main interaction
    UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Quest")
    void OnQuestInteract(AActor* Interactor);
};