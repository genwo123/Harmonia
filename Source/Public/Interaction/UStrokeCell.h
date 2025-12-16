#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Border.h"
#include "Components/Image.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Core/StrokeGameTypes.h"
#include "UStrokeCell.generated.h"

class UStrokeGrid;

UCLASS()
class DISTRICT_TEST_API UStrokeCell : public UUserWidget
{
    GENERATED_BODY()

public:
    UStrokeCell(const FObjectInitializer& ObjectInitializer);

protected:
    virtual void NativeConstruct() override;

public:
    UPROPERTY(meta = (BindWidget))
    class UBorder* CellBorder;

    UPROPERTY(meta = (BindWidget))
    class UImage* PlayerIcon;

    UPROPERTY(meta = (BindWidget))
    class UButton* CellButton;

    UPROPERTY(meta = (BindWidget))
    class UTextBlock* DebugText;

    UPROPERTY(meta = (BindWidget))
    class UImage* CellImage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stroke Cell")
    FStrokeCellData CellData;

    UPROPERTY(BlueprintReadWrite, Category = "Stroke Cell")
    UStrokeGrid* ParentGrid;

    UPROPERTY(BlueprintReadWrite, Category = "Stroke Cell")
    bool bHasPlayer = false;

    UPROPERTY(BlueprintReadWrite, Category = "Path")
    TArray<FIntPoint> ConnectedDirections;

    UFUNCTION(BlueprintCallable, Category = "Path")
    void UpdatePathConnections(const TArray<FIntPoint>& VisitedPath, FIntPoint CurrentPos);

    UFUNCTION(BlueprintImplementableEvent, Category = "Path")
    void DrawPathLines();

    UFUNCTION(BlueprintPure, Category = "Colors")
    FLinearColor GetCustomCellColor() const;

    UFUNCTION(BlueprintPure, Category = "Images")
    UTexture2D* GetCustomCellImage() const;

    UFUNCTION(BlueprintCallable, Category = "Stroke Cell")
    void SetCellData(const FStrokeCellData& NewCellData);

    UFUNCTION(BlueprintCallable, Category = "Stroke Cell")
    void UpdateVisuals();

    UFUNCTION(BlueprintCallable, Category = "Stroke Cell")
    FLinearColor GetCellColor() const;

    UFUNCTION(BlueprintCallable, Category = "Stroke Cell")
    void SetPlayerPresence(bool bPresent);

    UFUNCTION(BlueprintCallable, Category = "Stroke Cell")
    void SetVisited(bool bVisited);

    UFUNCTION(BlueprintCallable, Category = "Stroke Cell")
    void UpdateDebugDisplay(bool bShowDebug);

    UFUNCTION(BlueprintCallable, Category = "Stroke Cell")
    bool IsTeleportPortal() const;

    UFUNCTION(BlueprintCallable, Category = "Stroke Cell")
    int32 GetTeleportPortalID() const;

    UFUNCTION(BlueprintCallable, Category = "Stroke Cell")
    void UpdateInteractionState(bool bIsEditMode);

protected:
    UFUNCTION()
    void OnCellClicked();
};