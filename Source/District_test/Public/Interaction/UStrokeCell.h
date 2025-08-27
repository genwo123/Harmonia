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
    // 위젯 컴포넌트들
    UPROPERTY(meta = (BindWidget))
    class UBorder* CellBorder;

    UPROPERTY(meta = (BindWidget))
    class UImage* PlayerIcon;

    UPROPERTY(meta = (BindWidget))
    class UButton* CellButton;

    UPROPERTY(meta = (BindWidget))
    class UTextBlock* DebugText;

    // 셀 데이터
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stroke Cell")
    FStrokeCellData CellData;

    // 부모 그리드 참조
    UPROPERTY(BlueprintReadWrite, Category = "Stroke Cell")
    UStrokeGrid* ParentGrid;

    // 플레이어가 현재 이 셀에 있는지
    UPROPERTY(BlueprintReadWrite, Category = "Stroke Cell")
    bool bHasPlayer = false;


    UPROPERTY(BlueprintReadWrite, Category = "Path")
    TArray<FIntPoint> ConnectedDirections;

    // 경로 연결 업데이트 함수
    UFUNCTION(BlueprintCallable, Category = "Path")
    void UpdatePathConnections(const TArray<FIntPoint>& VisitedPath, FIntPoint CurrentPos);

    // Blueprint에서 선 그리기 구현할 이벤트
    UFUNCTION(BlueprintImplementableEvent, Category = "Path")
    void DrawPathLines();

    // 커스텀 색상 가져오기 함수
    UFUNCTION(BlueprintPure, Category = "Colors")
    FLinearColor GetCustomCellColor() const;



public:
    // 함수들
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

protected:
    // 버튼 클릭 이벤트
    UFUNCTION()
    void OnCellClicked();
};