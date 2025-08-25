// PuzzleStarter.h - ���常 ������ �߰�
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Interaction/InteractableInterface.h"
#include "Sound/SoundBase.h"  // �߰�
#include "PuzzleStarter.generated.h"

class AGridMazeManager;

UCLASS(Blueprintable, BlueprintType)
class DISTRICT_TEST_API APuzzleStarter : public AActor, public IInteractableInterface
{
    GENERATED_BODY()

public:
    APuzzleStarter();

protected:
    // ������Ʈ
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* MeshComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USphereComponent* InteractionSphere;

public:
    // ������ �Ŵ���
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Connection")
    AGridMazeManager* ConnectedMazeManager;

    // �� ���� ������� ����
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    bool bSingleUse = true;

    // ���ƴ��� ����
    UPROPERTY(BlueprintReadOnly, Category = "State")
    bool bUsed = false;

    // ====== ���� ���� (�߰��� �κ�) ======
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
    USoundBase* ButtonSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
    float SoundVolume = 1.0f;

    // ��ȣ�ۿ� �Լ�
    UFUNCTION(BlueprintCallable, Category = "Puzzle")
    void StartConnectedPuzzle();

    // InteractableInterface ����
    virtual void Interact_Implementation(AActor* Interactor) override;
    virtual bool CanInteract_Implementation(AActor* Interactor) override;
    virtual FString GetInteractionText_Implementation() override;
    virtual EInteractionType GetInteractionType_Implementation() override;
};