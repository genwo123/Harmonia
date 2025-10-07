#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interaction/InteractableInterface.h"
#include "Engine/DataTable.h"
#include "Components/TextRenderComponent.h"  
#include "HintFrame.generated.h"

// 뒷면 텍스트 배치 정보
USTRUCT(BlueprintType)
struct FBackTextPlacement
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Text;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector RelativeLocation; // 액자 뒷면 기준 상대 위치

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FRotator RelativeRotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FontSize = 24.0f;
};

USTRUCT(BlueprintType)
struct FHintFrameData : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString HintID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UTexture2D* FrontImage; // 앞면 탑뷰 이미지

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FBackTextPlacement> BackTexts; // 뒷면 여러 위치에 텍스트

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSubclassOf<UUserWidget> ViewerWidgetClass;
};

UCLASS()
class DISTRICT_TEST_API AHintFrame : public AActor, public IInteractableInterface
{
    GENERATED_BODY()

public:
    AHintFrame();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* FrameMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* FrontImagePoint; // 앞면 이미지 위치

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* BackTextRoot; // 뒷면 텍스트들의 부모

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hint Settings")
    FString HintID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hint Settings")
    UDataTable* HintDataTable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction Settings")
    float InteractionDistance = 300.0f;

    // Interface
    virtual void Interact_Implementation(AActor* Interactor) override;
    virtual bool CanInteract_Implementation(AActor* Interactor) override;
    virtual FString GetInteractionText_Implementation() override;
    virtual EInteractionType GetInteractionType_Implementation() override;

    UFUNCTION(BlueprintCallable, Category = "Hint")
    FHintFrameData GetHintData();

    UFUNCTION(BlueprintImplementableEvent, Category = "Hint")
    void OnFrameInteracted(AActor* Interactor);

protected:
    virtual void BeginPlay() override;
    virtual void OnConstruction(const FTransform& Transform) override;

private:
    void UpdateBackTextsFromData();
    TArray<UTextRenderComponent*> BackTextComponents;
};