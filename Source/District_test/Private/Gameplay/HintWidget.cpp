// ============================================
// HintWidget.cpp
// ============================================
#include "Gameplay/HintWidget.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"

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

    BlockImages.Empty();
    BlockImages.Add(Block0Image);
    BlockImages.Add(Block1Image);
    BlockImages.Add(Block2Image);
    BlockImages.Add(Block3Image);
    BlockImages.Add(Block4Image);
    BlockImages.Add(Block5Image);

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
        FString Reason = FString::Printf(TEXT("Cooldown: %.0f seconds"), CurrentCooldown);
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
    for (int32 i = 0; i < BlockImages.Num(); i++)
    {
        if (BlockImages[i])
        {
            bool bIsRevealed = RevealedBlocks[i];
            BlockImages[i]->SetVisibility(bIsRevealed ? ESlateVisibility::Hidden : ESlateVisibility::Visible);
        }
    }
}

void UHintWidget::UpdateCooldownDisplay()
{
    if (!CooldownText)
        return;

    if (!bIsOnCooldown)
    {
        CooldownText->SetText(FText::FromString(TEXT("Ready")));
        CooldownText->SetColorAndOpacity(FSlateColor(FLinearColor::Green));
    }
    else
    {
        FString CooldownString = FString::Printf(TEXT("Wait: %.0fs"), CurrentCooldown);
        CooldownText->SetText(FText::FromString(CooldownString));
        CooldownText->SetColorAndOpacity(FSlateColor(FLinearColor::Red));
    }
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