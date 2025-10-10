#include "Gameplay/HintWidget.h"
#include "Interaction/InteractableMechanism.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Engine/Texture2D.h"

void UHintWidget::NativeConstruct()
{
    Super::NativeConstruct();

    BlockButtons.Empty();
    BlockButtons.Add(Block0Button);
    BlockButtons.Add(Block1Button);
    BlockButtons.Add(Block2Button);
    BlockButtons.Add(Block3Button);
    BlockButtons.Add(Block4Button);
    BlockButtons.Add(Block5Button);

    InitializeBlocks();
    BindButtonEvents();
    UpdateBlockVisibility();
}

void UHintWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    if (bIsOnCooldown)
    {
        UpdateCooldown(InDeltaTime);
    }

    UpdateCooldownDisplay();
}

void UHintWidget::InitializeBlocks()
{
    RevealedBlocks.SetNum(6);
    for (int32 i = 0; i < 6; i++)
    {
        RevealedBlocks[i] = false;
    }

    bIsOnCooldown = false;
    CurrentCooldown = 0.0f;
}

void UHintWidget::LoadHintImage(UDataTable* HintTable, int32 LevelNumber)
{
    if (!HintTable || !HintImage)
    {
        return;
    }

    FString RowName = FString::FromInt(LevelNumber);
    FHintImageData* RowData = HintTable->FindRow<FHintImageData>(*RowName, TEXT(""));

    if (RowData && RowData->HintImage)
    {
        HintImage->SetBrushFromTexture(RowData->HintImage);
    }
}

void UHintWidget::BindButtonEvents()
{
    if (Block0Button)
        Block0Button->OnClicked.AddDynamic(this, &UHintWidget::OnBlock0Clicked);
    if (Block1Button)
        Block1Button->OnClicked.AddDynamic(this, &UHintWidget::OnBlock1Clicked);
    if (Block2Button)
        Block2Button->OnClicked.AddDynamic(this, &UHintWidget::OnBlock2Clicked);
    if (Block3Button)
        Block3Button->OnClicked.AddDynamic(this, &UHintWidget::OnBlock3Clicked);
    if (Block4Button)
        Block4Button->OnClicked.AddDynamic(this, &UHintWidget::OnBlock4Clicked);
    if (Block5Button)
        Block5Button->OnClicked.AddDynamic(this, &UHintWidget::OnBlock5Clicked);
}

void UHintWidget::OnBlock0Clicked()
{
    HandleBlockClick(0);
}

void UHintWidget::OnBlock1Clicked()
{
    HandleBlockClick(1);
}

void UHintWidget::OnBlock2Clicked()
{
    HandleBlockClick(2);
}

void UHintWidget::OnBlock3Clicked()
{
    HandleBlockClick(3);
}

void UHintWidget::OnBlock4Clicked()
{
    HandleBlockClick(4);
}

void UHintWidget::OnBlock5Clicked()
{
    HandleBlockClick(5);
}

void UHintWidget::HandleBlockClick(int32 BlockIndex)
{
    if (BlockIndex < 0 || BlockIndex >= RevealedBlocks.Num())
    {
        return;
    }

    if (RevealedBlocks[BlockIndex])
    {
        OnRevealFailed(BlockIndex, TEXT("Already revealed"));
        return;
    }

    if (bIsOnCooldown)
    {
        int32 Minutes = FMath::FloorToInt(CurrentCooldown / 60.0f);
        int32 Seconds = FMath::FloorToInt(CurrentCooldown) % 60;
        FString Reason = FString::Printf(TEXT("Cooldown: %02d:%02d"), Minutes, Seconds);
        OnRevealFailed(BlockIndex, Reason);
        return;
    }

    RevealedBlocks[BlockIndex] = true;
    StartCooldown();
    UpdateBlockVisibility();
    OnBlockRevealed(BlockIndex);
}

void UHintWidget::StartCooldown()
{
    bIsOnCooldown = true;
    CurrentCooldown = RevealCooldown;
}

void UHintWidget::UpdateCooldown(float DeltaTime)
{
    CurrentCooldown -= DeltaTime;

    if (CurrentCooldown <= 0.0f)
    {
        CurrentCooldown = 0.0f;
        bIsOnCooldown = false;
    }
}

void UHintWidget::UpdateBlockVisibility()
{
    for (int32 i = 0; i < BlockButtons.Num(); i++)
    {
        if (BlockButtons[i])
        {
            bool bIsRevealed = RevealedBlocks[i];
            BlockButtons[i]->SetVisibility(bIsRevealed ? ESlateVisibility::Hidden : ESlateVisibility::Visible);
        }
    }
}

void UHintWidget::UpdateCooldownDisplay()
{
    if (!CooldownText)
        return;

    int32 Minutes = FMath::FloorToInt(CurrentCooldown / 60.0f);
    int32 Seconds = FMath::FloorToInt(CurrentCooldown) % 60;
    FString CooldownString = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
    CooldownText->SetText(FText::FromString(CooldownString));
    CooldownText->SetColorAndOpacity(FSlateColor(FLinearColor::White));
}

void UHintWidget::ResetHintWidget()
{
    for (int32 i = 0; i < RevealedBlocks.Num(); i++)
    {
        RevealedBlocks[i] = false;
    }

    bIsOnCooldown = false;
    CurrentCooldown = 0.0f;
    UpdateBlockVisibility();
}