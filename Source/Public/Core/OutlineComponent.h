#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "OutlineComponent.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class DISTRICT_TEST_API UOutlineComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UOutlineComponent();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Outline")
    FLinearColor OutlineColor = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Outline")
    float OutlineThickness = 1.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Outline")
    float BloomIntensity = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Outline")
    UMaterialInterface* OutlineMaterial;

    UFUNCTION(BlueprintCallable, Category = "Outline")
    void ShowOutline();

    UFUNCTION(BlueprintCallable, Category = "Outline")
    void HideOutline();

protected:
    virtual void BeginPlay() override;

private:
    UPROPERTY()
    TArray<UStaticMeshComponent*> OutlineMeshes;

    void CreateOutlineMeshes();
    void DestroyOutlineMeshes();
};