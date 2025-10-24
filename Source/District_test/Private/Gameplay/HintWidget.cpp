#include "Gameplay/HintWidget.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Engine/Texture2D.h"
#include "Save_Instance/Hamoina_GameInstance.h"
#include "Kismet/GameplayStatics.h"

void UHintWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // GameInstance 참조 가져오기 (한 번만)
    if (!GameInstance)
    {
        GameInstance = Cast<UHamoina_GameInstance>(UGameplayStatics::GetGameInstance(this));
    }

    // BlockButtons 배열 초기화 (한 번만)
    if (BlockButtons.Num() == 0)
    {
        BlockButtons.Empty();
        BlockButtons.Add(Block0Button);
        BlockButtons.Add(Block1Button);
        BlockButtons.Add(Block2Button);
        BlockButtons.Add(Block3Button);
        BlockButtons.Add(Block4Button);
        BlockButtons.Add(Block5Button);
    }

    // 버튼 이벤트 바인딩 (한 번만)
    if (!bButtonsBound)
    {
        BindButtonEvents();
        bButtonsBound = true;
    }

    // 최초 초기화가 아직 안 되었다면
    if (!bIsInitialized)
    {
        InitializeBlocks();
        bIsInitialized = true;
    }

    // 레벨 리셋 체크
    if (CheckIfLevelWasReset())
    {
        OnLevelReset();
    }
    else
    {
        // SaveGame에서 상태 복원
        LoadStateFromSaveGame();
    }

    // UI 업데이트
    UpdateBlockVisibility();
    UpdateCooldownDisplay();
}

void UHintWidget::NativeDestruct()
{
    // 위젯이 파괴될 때 버튼 바인딩 해제
    UnbindButtonEvents();
    bButtonsBound = false;

    Super::NativeDestruct();
}

void UHintWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    if (bIsOnCooldown)
    {
        UpdateCooldown(InDeltaTime);
        UpdateCooldownDisplay();
    }
}

void UHintWidget::InitializeHint(int32 InLevelNumber)
{
    CurrentLevelNumber = InLevelNumber;

    if (GameInstance && GameInstance->CurrentSaveData)
    {
        GameInstance->CurrentSaveData->InitializeHintForLevel(CurrentLevelNumber);
    }

    LoadStateFromSaveGame();
    UpdateBlockVisibility();
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
    if (Block0Button && !Block0Button->OnClicked.IsBound())
        Block0Button->OnClicked.AddDynamic(this, &UHintWidget::OnBlock0Clicked);
    if (Block1Button && !Block1Button->OnClicked.IsBound())
        Block1Button->OnClicked.AddDynamic(this, &UHintWidget::OnBlock1Clicked);
    if (Block2Button && !Block2Button->OnClicked.IsBound())
        Block2Button->OnClicked.AddDynamic(this, &UHintWidget::OnBlock2Clicked);
    if (Block3Button && !Block3Button->OnClicked.IsBound())
        Block3Button->OnClicked.AddDynamic(this, &UHintWidget::OnBlock3Clicked);
    if (Block4Button && !Block4Button->OnClicked.IsBound())
        Block4Button->OnClicked.AddDynamic(this, &UHintWidget::OnBlock4Clicked);
    if (Block5Button && !Block5Button->OnClicked.IsBound())
        Block5Button->OnClicked.AddDynamic(this, &UHintWidget::OnBlock5Clicked);
}

void UHintWidget::UnbindButtonEvents()
{
    if (Block0Button)
        Block0Button->OnClicked.RemoveDynamic(this, &UHintWidget::OnBlock0Clicked);
    if (Block1Button)
        Block1Button->OnClicked.RemoveDynamic(this, &UHintWidget::OnBlock1Clicked);
    if (Block2Button)
        Block2Button->OnClicked.RemoveDynamic(this, &UHintWidget::OnBlock2Clicked);
    if (Block3Button)
        Block3Button->OnClicked.RemoveDynamic(this, &UHintWidget::OnBlock3Clicked);
    if (Block4Button)
        Block4Button->OnClicked.RemoveDynamic(this, &UHintWidget::OnBlock4Clicked);
    if (Block5Button)
        Block5Button->OnClicked.RemoveDynamic(this, &UHintWidget::OnBlock5Clicked);
}

void UHintWidget::OnBlock0Clicked() { HandleBlockClick(0); }
void UHintWidget::OnBlock1Clicked() { HandleBlockClick(1); }
void UHintWidget::OnBlock2Clicked() { HandleBlockClick(2); }
void UHintWidget::OnBlock3Clicked() { HandleBlockClick(3); }
void UHintWidget::OnBlock4Clicked() { HandleBlockClick(4); }
void UHintWidget::OnBlock5Clicked() { HandleBlockClick(5); }

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

    // 블록 해금
    RevealedBlocks[BlockIndex] = true;
    StartCooldown();

    // SaveGame에 저장
    SaveStateToSaveGame();

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

        // 쿨다운 끝났으니 저장
        SaveStateToSaveGame();
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

    if (bIsOnCooldown)
    {
        int32 Minutes = FMath::FloorToInt(CurrentCooldown / 60.0f);
        int32 Seconds = FMath::FloorToInt(CurrentCooldown) % 60;
        FString CooldownString = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
        CooldownText->SetText(FText::FromString(CooldownString));
        CooldownText->SetColorAndOpacity(FSlateColor(FLinearColor::White));
    }
    else
    {
        CooldownText->SetText(FText::FromString(TEXT("--:--")));
        CooldownText->SetColorAndOpacity(FSlateColor(FLinearColor(0.5f, 0.5f, 0.5f)));
    }
}

void UHintWidget::LoadStateFromSaveGame()
{
    if (!GameInstance || !GameInstance->CurrentSaveData)
        return;

    // SaveGame에서 불러오기
    TArray<bool> SavedBlocks = GameInstance->CurrentSaveData->GetHintBlockStates(CurrentLevelNumber);

    if (SavedBlocks.Num() == 6)
    {
        RevealedBlocks = SavedBlocks;
    }

    float SavedCooldown = GameInstance->CurrentSaveData->GetHintCooldownTime(CurrentLevelNumber);
    if (SavedCooldown > 0.0f)
    {
        bIsOnCooldown = true;
        CurrentCooldown = SavedCooldown;
    }
    else
    {
        bIsOnCooldown = false;
        CurrentCooldown = 0.0f;
    }
}

void UHintWidget::SaveStateToSaveGame()
{
    if (!GameInstance || !GameInstance->CurrentSaveData)
        return;

    // SaveGame에 저장
    GameInstance->CurrentSaveData->SetHintBlockStates(CurrentLevelNumber, RevealedBlocks);
    GameInstance->CurrentSaveData->SetHintCooldownTime(CurrentLevelNumber, CurrentCooldown);
    GameInstance->SaveContinueGame();
}

void UHintWidget::ResetHintWidget()
{
    for (int32 i = 0; i < RevealedBlocks.Num(); i++)
    {
        RevealedBlocks[i] = false;
    }

    bIsOnCooldown = false;
    CurrentCooldown = 0.0f;

    // SaveGame에도 리셋 저장
    SaveStateToSaveGame();

    UpdateBlockVisibility();
    UpdateCooldownDisplay();
}

bool UHintWidget::CheckIfLevelWasReset()
{
    UWorld* World = GetWorld();
    if (!World)
        return false;

    float WorldTime = World->GetTimeSeconds();
    return (WorldTime < 5.0f);
}

void UHintWidget::OnLevelReset()
{
    for (int32 i = 0; i < RevealedBlocks.Num(); i++)
    {
        RevealedBlocks[i] = false;
    }

    bIsOnCooldown = false;
    CurrentCooldown = 0.0f;

    SaveStateToSaveGame();

    UpdateBlockVisibility();
    UpdateCooldownDisplay();
}