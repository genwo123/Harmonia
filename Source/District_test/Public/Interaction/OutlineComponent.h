#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "OutlineComponent.generated.h"

UCLASS(ClassGroup = (Interaction), meta = (BlueprintSpawnableComponent))
class DISTRICT_TEST_API UOutlineComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UOutlineComponent();

    UFUNCTION(BlueprintCallable, Category = "Outline")
    void ShowOutline();

    UFUNCTION(BlueprintCallable, Category = "Outline")
    void HideOutline();

    // BP에서 M_OutlineOverlay 할당
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Outline")
    UMaterialInterface* OutlineMaterial;

    // 나중에 색상 커스터마이징 가능하게
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Outline")
    FLinearColor OutlineColor = FLinearColor::Yellow;

protected:
    virtual void BeginPlay() override;

private:
    UPROPERTY()
    TArray<UPrimitiveComponent*> CachedPrimitiveComponents;
};