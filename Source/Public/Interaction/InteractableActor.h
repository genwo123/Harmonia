// InteractableActor.h
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interaction/InteractableInterface.h"
#include "InteractableActor.generated.h"

UCLASS(Blueprintable)
class DISTRICT_TEST_API AInteractableActor : public AActor, public IInteractableInterface
{
    GENERATED_BODY()
public:
    AInteractableActor();


    virtual void Interact_Implementation(AActor* Interactor) override;
    virtual bool CanInteract_Implementation(AActor* Interactor) override;
    virtual FString GetInteractionText_Implementation() override;
    virtual EInteractionType GetInteractionType_Implementation() override;

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* MeshComponent;


    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    FString InteractionText = "Interaction!";


    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    EInteractionType InteractionType = EInteractionType::Default;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    bool bCanBeInteracted = true;
};