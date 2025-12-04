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

    UFUNCTION(BlueprintCallable, Category = "Outline")
    void ShowOutline();

    UFUNCTION(BlueprintCallable, Category = "Outline")
    void HideOutline();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Outline")
    UMaterialInterface* OutlineMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Outline")
    float OutlineThickness = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Outline")
    FLinearColor OutlineColor = FLinearColor::White;

    UPROPERTY()
    UStaticMeshComponent* TargetMesh;
};