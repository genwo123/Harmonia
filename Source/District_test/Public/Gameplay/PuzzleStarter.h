// PuzzleStarter.h - 사운드만 간단히 추가
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Interaction/InteractableInterface.h"
#include "Sound/SoundBase.h"  // 추가
#include "PuzzleStarter.generated.h"

class AGridMazeManager;

UCLASS(Blueprintable, BlueprintType)
class DISTRICT_TEST_API APuzzleStarter : public AActor, public IInteractableInterface
{
    GENERATED_BODY()

public:
    APuzzleStarter();

protected:
    // 컴포넌트
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* MeshComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USphereComponent* InteractionSphere;

public:
    // 연결할 매니저
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Connection")
    AGridMazeManager* ConnectedMazeManager;

    // 한 번만 사용할지 여부
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    bool bSingleUse = true;

    // 사용됐는지 여부
    UPROPERTY(BlueprintReadOnly, Category = "State")
    bool bUsed = false;

    // ====== 사운드 설정 (추가된 부분) ======
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
    USoundBase* ButtonSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
    float SoundVolume = 1.0f;

    // 상호작용 함수
    UFUNCTION(BlueprintCallable, Category = "Puzzle")
    void StartConnectedPuzzle();

    // InteractableInterface 구현
    virtual void Interact_Implementation(AActor* Interactor) override;
    virtual bool CanInteract_Implementation(AActor* Interactor) override;
    virtual FString GetInteractionText_Implementation() override;
    virtual EInteractionType GetInteractionType_Implementation() override;
};