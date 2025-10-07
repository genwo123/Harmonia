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
        CellButton->SetIsEnabled(false);
    }

    SetVisibility(ESlateVisibility::SelfHitTestInvisible);
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
    UTexture2D* CellTexture = GetCustomCellImage();

    if (CellTexture && CellImage)
    {
        CellBorder->SetBrushColor(FLinearColor::White);
        CellImage->SetBrushFromTexture(CellTexture);
        CellImage->SetVisibility(ESlateVisibility::Visible);
    }
    else
    {
        CellBorder->SetBrushColor(CellColor);
        if (CellImage)
        {
            CellImage->SetVisibility(ESlateVisibility::Hidden);
        }
    }

    if (PlayerIcon)
    {
        PlayerIcon->SetVisibility(bHasPlayer ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
    }

    UpdateDebugDisplay(ParentGrid ? ParentGrid->bShowGridNumbers : false);
}

FLinearColor UStrokeCell::GetCellColor() const
{
    if (CellData.bIsVisited && CellData.CellType != EStrokeCellType::Start)
    {
        if (ParentGrid)
        {
            return ParentGrid->CurrentPathLineColor;
        }
        return FLinearColor(0.5f, 0.5f, 0.5f, 1.0f);
    }

    if (IsTeleportPortal())
    {
        int32 PortalID = GetTeleportPortalID();
        switch (PortalID)
        {
        case 1: return FLinearColor(0.0f, 0.5f, 1.0f, 1.0f);
        case 2: return FLinearColor(1.0f, 1.0f, 0.0f, 1.0f);
        case 3: return FLinearColor(1.0f, 0.0f, 1.0f, 1.0f);
        case 4: return FLinearColor(0.0f, 1.0f, 1.0f, 1.0f);
        default: return FLinearColor(0.8f, 0.8f, 0.8f, 1.0f);
        }
    }

    return GetCustomCellColor();
}

UTexture2D* UStrokeCell::GetCustomCellImage() const
{
    if (!ParentGrid)
    {
        return nullptr;
    }

    if (IsTeleportPortal())
    {
        int32 PortalID = GetTeleportPortalID();
        switch (PortalID)
        {
        case 1: return ParentGrid->TeleportPortal1Image;
        case 2: return ParentGrid->TeleportPortal2Image;
        case 3: return ParentGrid->TeleportPortal3Image;
        case 4: return ParentGrid->TeleportPortal4Image;
        default: return nullptr;
        }
    }

    if (CellData.CellType == EStrokeCellType::Start)
    {
        return ParentGrid->StartPointImage;
    }

    if (CellData.CellType == EStrokeCellType::Goal)
    {
        return ParentGrid->GoalPointImage;
    }

    if (CellData.CellType == EStrokeCellType::Wall)
    {
        return ParentGrid->WallImage;
    }

    if (CellData.CellType == EStrokeCellType::RedPoint)
    {
        return CellData.bIsVisited ? ParentGrid->RedPointVisitedImage : ParentGrid->RedPointImage;
    }

    if (CellData.CellType == EStrokeCellType::GreenPoint)
    {
        return CellData.bIsVisited ? ParentGrid->GreenPointVisitedImage : ParentGrid->GreenPointImage;
    }

    if (CellData.CellType == EStrokeCellType::BluePoint)
    {
        return CellData.bIsVisited ? ParentGrid->BluePointVisitedImage : ParentGrid->BluePointImage;
    }

    if (CellData.CellType == EStrokeCellType::Empty)
    {
        if (CellData.bIsVisited)
        {
            FLinearColor PathColor = ParentGrid->CurrentPathLineColor;

            if (PathColor == ParentGrid->RedPointColor)
            {
                return ParentGrid->VisitedGroundRedImage;
            }
            else if (PathColor == ParentGrid->GreenPointColor)
            {
                return ParentGrid->VisitedGroundGreenImage;
            }
            else if (PathColor == ParentGrid->BluePointColor)
            {
                return ParentGrid->VisitedGroundBlueImage;
            }
            else
            {
                return ParentGrid->VisitedGroundImage;
            }
        }
        else
        {
            return ParentGrid->GroundImage;
        }
    }

    return nullptr;
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

void UStrokeCell::UpdateInteractionState(bool bIsEditMode)
{
    if (CellButton)
    {
        CellButton->SetIsEnabled(bIsEditMode);
    }

    if (bIsEditMode)
    {
        SetVisibility(ESlateVisibility::Visible);
    }
    else
    {
        SetVisibility(ESlateVisibility::SelfHitTestInvisible);
    }
}

void UStrokeCell::OnCellClicked()
{
    if (ParentGrid && ParentGrid->bEditMode)
    {
        ParentGrid->OnCellEditClicked(CellData.GridPosition);
    }
}

void UStrokeCell::UpdatePathConnections(const TArray<FIntPoint>& VisitedPath, FIntPoint CurrentPos)
{
    ConnectedDirections.Empty();
    FIntPoint MyPos = CellData.GridPosition;

    int32 MyIndex = VisitedPath.Find(MyPos);

    if (MyIndex != INDEX_NONE)
    {
        if (MyIndex > 0)
        {
            FIntPoint PrevPos = VisitedPath[MyIndex - 1];
            FIntPoint IncomingLineDirection = PrevPos - MyPos;
            ConnectedDirections.Add(IncomingLineDirection);
        }

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
