#include "Interaction/UStrokeCell.h"
#include "Interaction/UStrokeGrid.h"
#include "Components/Border.h"
#include "Components/Image.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"

UStrokeCell::UStrokeCell(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    CellData.CellType = EStrokeCellType::Empty;
    CellData.bIsVisited = false;
    CellData.GridPosition = FIntPoint(0, 0);
    bHasPlayer = false;
}

void UStrokeCell::NativeConstruct()
{
    Super::NativeConstruct();

    if (CellButton)
    {
        CellButton->OnClicked.AddDynamic(this, &UStrokeCell::OnCellClicked);
    }

    UpdateVisuals();
}

void UStrokeCell::SetCellData(const FStrokeCellData& NewCellData)
{
    CellData = NewCellData;
    UpdateVisuals();
}

void UStrokeCell::UpdateVisuals()
{
    if (!CellBorder) return;

    FLinearColor CellColor = GetCellColor();
    CellBorder->SetBrushColor(CellColor);

    if (PlayerIcon)
    {
        PlayerIcon->SetVisibility(bHasPlayer ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
    }

    UpdateDebugDisplay(ParentGrid ? ParentGrid->bShowGridNumbers : false);
}

FLinearColor UStrokeCell::GetCellColor() const
{
    // 방문한 셀이면 회색 (시작점 제외)
    if (CellData.bIsVisited && CellData.CellType != EStrokeCellType::Start)
    {
        return FLinearColor(0.5f, 0.5f, 0.5f, 1.0f);
    }

    // 텔레포트 포털 색상
    if (IsTeleportPortal())
    {
        int32 PortalID = GetTeleportPortalID();
        switch (PortalID)
        {
        case 1: return FLinearColor(0.0f, 0.5f, 1.0f, 1.0f);  // 파란색 포털
        case 2: return FLinearColor(1.0f, 1.0f, 0.0f, 1.0f);  // 노란색 포털
        case 3: return FLinearColor(1.0f, 0.0f, 1.0f, 1.0f);  // 보라색 포털
        case 4: return FLinearColor(0.0f, 1.0f, 1.0f, 1.0f);  // 청록색 포털
        default: return FLinearColor(0.8f, 0.8f, 0.8f, 1.0f); // 기본 포털
        }
    }

    // 부모 그리드에서 커스텀 색상 가져오기
    return GetCustomCellColor();
}

void UStrokeCell::SetPlayerPresence(bool bPresent)
{
    bHasPlayer = bPresent;
    UpdateVisuals();
}

void UStrokeCell::SetVisited(bool bVisited)
{
    CellData.bIsVisited = bVisited;
    UpdateVisuals();
}

void UStrokeCell::UpdateDebugDisplay(bool bShowDebug)
{
    if (!DebugText) return;

    if (bShowDebug)
    {
        FString DebugString = FString::Printf(TEXT("%d,%d"),
            CellData.GridPosition.X, CellData.GridPosition.Y);

        // 텔레포트 포털이면 ID도 표시
        if (IsTeleportPortal())
        {
            int32 PortalID = GetTeleportPortalID();
            DebugString += FString::Printf(TEXT("\nTP:%d"), PortalID);
        }

        DebugText->SetText(FText::FromString(DebugString));
        DebugText->SetVisibility(ESlateVisibility::Visible);
    }
    else
    {
        DebugText->SetVisibility(ESlateVisibility::Hidden);
    }
}

bool UStrokeCell::IsTeleportPortal() const
{
    if (!ParentGrid) return false;

    for (const FTeleportPortal& Portal : ParentGrid->EditorTeleportPortals)
    {
        if (Portal.PortalA == CellData.GridPosition || Portal.PortalB == CellData.GridPosition)
        {
            return true;
        }
    }
    return false;
}

int32 UStrokeCell::GetTeleportPortalID() const
{
    if (!ParentGrid) return -1;

    for (const FTeleportPortal& Portal : ParentGrid->EditorTeleportPortals)
    {
        if (Portal.PortalA == CellData.GridPosition || Portal.PortalB == CellData.GridPosition)
        {
            return Portal.PortalID;
        }
    }
    return -1;
}

void UStrokeCell::OnCellClicked()
{
    if (ParentGrid && ParentGrid->bEditMode)
    {
        // 에디터 모드에서는 편집
        ParentGrid->OnCellEditClicked(CellData.GridPosition);
    }
    else
    {
        // 게임 모드에서는 디버그 정보 출력
        UE_LOG(LogTemp, Log, TEXT("Cell clicked: (%d, %d) - Type: %d"),
            CellData.GridPosition.X, CellData.GridPosition.Y, (int32)CellData.CellType);
    }
}


void UStrokeCell::UpdatePathConnections(const TArray<FIntPoint>& VisitedPath, FIntPoint CurrentPos)
{
    ConnectedDirections.Empty();
    FIntPoint MyPos = CellData.GridPosition;

    int32 MyIndex = VisitedPath.Find(MyPos);

    if (MyIndex != INDEX_NONE)
    {
        // 들어온 방향 (라인이 표시될 방향)
        if (MyIndex > 0)
        {
            FIntPoint PrevPos = VisitedPath[MyIndex - 1];
            FIntPoint IncomingLineDirection = PrevPos - MyPos;  // 방향 뒤집기!
            ConnectedDirections.Add(IncomingLineDirection);
        }

        // 나가는 방향 (이건 맞다고 하셨으니 그대로)
        if (MyIndex < VisitedPath.Num() - 1)
        {
            FIntPoint NextPos = VisitedPath[MyIndex + 1];
            FIntPoint OutgoingDirection = NextPos - MyPos;
            ConnectedDirections.Add(OutgoingDirection);
        }
        else if (MyIndex == VisitedPath.Num() - 1)
        {
            FIntPoint OutgoingDirection = CurrentPos - MyPos;
            ConnectedDirections.Add(OutgoingDirection);
        }
    }

    DrawPathLines();
}
FLinearColor UStrokeCell::GetCustomCellColor() const
{
    if (!ParentGrid)
    {
        // ParentGrid가 없으면 기본 색상 사용
        switch (CellData.CellType)
        {
        case EStrokeCellType::Empty:
            return FLinearColor::White;
        case EStrokeCellType::Wall:
            return FLinearColor::Black;
        case EStrokeCellType::Start:
            return FLinearColor::Green;
        case EStrokeCellType::Goal:
            return FLinearColor::Red;
        case EStrokeCellType::RedPoint:
            return FLinearColor(1.0f, 0.2f, 0.2f, 1.0f);
        case EStrokeCellType::GreenPoint:
            return FLinearColor(0.2f, 1.0f, 0.2f, 1.0f);
        case EStrokeCellType::BluePoint:
            return FLinearColor(0.2f, 0.2f, 1.0f, 1.0f);
        default:
            return FLinearColor::White;
        }
    }

    // ParentGrid의 커스텀 색상 사용
    switch (CellData.CellType)
    {
    case EStrokeCellType::Start:
        return ParentGrid->StartPointColor;
    case EStrokeCellType::Goal:
        return ParentGrid->GoalPointColor;
    case EStrokeCellType::RedPoint:
        return ParentGrid->RedPointColor;
    case EStrokeCellType::GreenPoint:
        return ParentGrid->GreenPointColor;
    case EStrokeCellType::BluePoint:
        return ParentGrid->BluePointColor;
    case EStrokeCellType::Wall:
        return FLinearColor::Black;
    case EStrokeCellType::Empty:
    default:
        return FLinearColor::White;
    }
}