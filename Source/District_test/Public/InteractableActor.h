// InteractableActor.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "InteractableInterface.h"
#include "InteractableActor.generated.h"

UCLASS(Blueprintable)
class DISTRICT_TEST_API AInteractableActor : public AActor, public IInteractableInterface
{
    GENERATED_BODY()

public:
    AInteractableActor();

    // InteractableInterface 구현
    virtual void Interact_Implementation(AActor* Interactor) override;
    virtual bool CanInteract_Implementation(AActor* Interactor) override;
    virtual FString GetInteractionText_Implementation() override;
    virtual EInteractionType GetInteractionType_Implementation() override;

protected:
    virtual void BeginPlay() override;

    // 메시 컴포넌트
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* MeshComponent;

    // 상호작용 텍스트
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    FString InteractionText = "Interaction!";

    // 상호작용 타입
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    EInteractionType InteractionType = EInteractionType::Default;

    // 상호작용 가능 여부
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    bool bCanBeInteracted = true;
};