// GridMazeManager.cpp
#include "Gameplay/GridMazeManager.h"
#include "Gameplay/GridTile.h"
#include "Gameplay/MazeDisplay.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

#if WITH_EDITOR
#include "Editor.h"
#endif

AGridMazeManager::AGridMazeManager()
{
    PrimaryActorTick.bCanEverTick = true;
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    StartingFloorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StartingFloorMesh"));
    StartingFloorMesh->SetupAttachment(RootComponent);

    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMeshAsset(TEXT("/Engine/BasicShapes/Cube"));
    if (CubeMeshAsset.Succeeded())
    {
        StartingFloorMesh->SetStaticMesh(CubeMeshAsset.Object);
    }

    StartingFloorMesh->SetMobility(EComponentMobility::Movable);
    StartingFloorMesh->SetSimulatePhysics(false);
    StartingFloorMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    StartingFloorMesh->SetCollisionResponseToAllChannels(ECR_Block);

    FailResetDelay = 3.0f;
    bContinueTimeOnFail = true;
    bKeepProgressOnFail = true;
    SoundVolume = 1.0f;
}

void AGridMazeManager::BeginPlay()
{
    Super::BeginPlay();

    CreateTilesInternal();

    if (bUseStartingFloor)
    {
        CreateStartingFloor();
    }

    if (bAutoFindDisplay)
    {
        ConnectToDisplay();
    }

    ValidateCorrectPath();

    TimeRemaining = PuzzleTimeLimit;
    SetPuzzleState(EPuzzleState::Ready);

    SetAllTilesInactive();
}

void AGridMazeManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if ((CurrentState == EPuzzleState::Playing ||
        (CurrentState == EPuzzleState::Failed && bContinueTimeOnFail)) &&
        !bGamePaused)
    {
        UpdateTimer(DeltaTime);
    }
}

void AGridMazeManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    DestroyAllTiles();
    Super::EndPlay(EndPlayReason);
}

void AGridMazeManager::Destroyed()
{
    DestroyAllTiles();
    Super::Destroyed();
}

#if WITH_EDITOR
void AGridMazeManager::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);

    UWorld* World = GetWorld();
    if (!World || World->WorldType != EWorldType::Editor) return;

    if (bShowPreviewInEditor && TileClass && GridRows > 0 && GridColumns > 0)
    {
        ClearGridTiles();
        CreateTilesInternal();

        if (bUseStartingFloor)
        {
            CreateStartingFloor();
        }
    }
    else
    {
        ClearGridTiles();
    }
}

void AGridMazeManager::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);

    if (!PropertyChangedEvent.Property) return;

    FString PropertyName = PropertyChangedEvent.Property->GetName();

    if (PropertyName.Contains(TEXT("Location")) ||
        PropertyName.Contains(TEXT("Rotation")) ||
        PropertyName.Contains(TEXT("Scale")))
    {
        UpdateTilePositions();
        if (bUseStartingFloor)
        {
            CreateStartingFloor();
        }
        return;
    }

    if (PropertyName == TEXT("TileThickness"))
    {
        UpdateTileThickness();
        return;
    }

    if (PropertyName == TEXT("GridRows") ||
        PropertyName == TEXT("GridColumns") ||
        PropertyName == TEXT("TileClass") ||
        PropertyName == TEXT("TileSize") ||
        PropertyName == TEXT("TileSpacing") ||
        PropertyName == TEXT("bShowPreviewInEditor"))
    {
        if (bShowPreviewInEditor)
        {
            ClearGridTiles();
            CreateTilesInternal();
            if (bUseStartingFloor)
            {
                CreateStartingFloor();
            }
        }
        else
        {
            ClearGridTiles();
        }
    }
}
#endif

// ============ 퍼즐 제어 함수 ============

void AGridMazeManager::StartPuzzle()
{
    if (CurrentState != EPuzzleState::Ready)
    {
        return;
    }

    if (!ValidateCorrectPath())
    {
        UE_LOG(LogTemp, Error, TEXT("Invalid path configuration"));
        return;
    }

    SetPuzzleState(EPuzzleState::Playing);
    TimeRemaining = PuzzleTimeLimit;
    CurrentPathIndex = 0;
    bGamePaused = false;
    bIsShowingPreview = true;

    SetAllTilesInactive();

    if (bEnablePreview)
    {
        ShowPathPreviewSequence();
    }
    else
    {
        SetAllTilesReady();
        if (CorrectPath.Num() > 0)
        {
            FIntPoint FirstStep = CorrectPath[0];
            AGridTile* FirstTile = GetTileAt(FirstStep.X, FirstStep.Y);
            if (FirstTile)
            {
                FirstTile->SetTileState(ETileState::FirstStep);
            }
        }
    }

    OnPuzzleStarted_Event.Broadcast();
    OnPuzzleStarted();
}

void AGridMazeManager::StartPuzzleWithCountdown()
{
    if (CurrentState == EPuzzleState::Ready && ConnectedDisplay)
    {
        ConnectedDisplay->SetDisplayState(EDisplayState::Countdown);
    }
    else
    {
        StartPuzzle();
    }
}

void AGridMazeManager::ResetPuzzle()
{
    SetPuzzleState(EPuzzleState::Ready);
    TimeRemaining = PuzzleTimeLimit;
    CurrentPathIndex = 0;
    bGamePaused = false;

    if (bKeepProgressOnFail)
    {
        RestoreProgressColors();
    }
    else
    {
        SetAllTilesInactive();
        ClearProgressHistory();
    }

    CustomResetLogic();
}

void AGridMazeManager::CompleteReset()
{
    SetPuzzleState(EPuzzleState::Ready);
    TimeRemaining = PuzzleTimeLimit;
    CurrentPathIndex = 0;
    bGamePaused = false;

    ClearProgressHistory();
    SetAllTilesInactive();

    CustomResetLogic();
}

void AGridMazeManager::CompletePuzzle()
{
    SetPuzzleState(EPuzzleState::Success);
    PlaySound(SuccessSound);

    if (ConnectedDisplay)
    {
        ConnectedDisplay->ShowMessage(TEXT("SUCCESS!"), ConnectedDisplay->SuccessColor); 
    }

    OnPuzzleCompleted_Event.Broadcast();
    OnPuzzleCompleted();
}

void AGridMazeManager::FailPuzzle()
{
    SetPuzzleState(EPuzzleState::Failed);
    PlaySound(FailSound);

    if (ConnectedDisplay)
    {
        ConnectedDisplay->ShowMessage(TEXT("TIME OUT!"), ConnectedDisplay->FailedColor);
    }

    OnPuzzleFailed_Event.Broadcast();
    OnPuzzleFailed();

    GetWorld()->GetTimerManager().SetTimer(ResetTimer, [this]()
        {
            CompleteReset();

            APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
            if (PlayerPawn)
            {
                RespawnPlayerToStart(PlayerPawn);
            }

            if (ConnectedDisplay)
            {
                ConnectedDisplay->ShowMessage(TEXT("READY"), ReadyColor);
            }

        }, 15.0f, false);
}

void AGridMazeManager::PausePuzzle()
{
    if (CurrentState == EPuzzleState::Playing)
    {
        bGamePaused = true;
    }
}

void AGridMazeManager::ResumePuzzle()
{
    if (CurrentState == EPuzzleState::Playing)
    {
        bGamePaused = false;
    }
}

// ============ 타일 관리 ============

void AGridMazeManager::OnTileStep(AGridTile* SteppedTile, AActor* Player)
{
    if (!SteppedTile || bIsShowingPreview) return;

    FVector2D TilePos = SteppedTile->GetGridPosition();
    FIntPoint IntTilePos = FIntPoint(TilePos.X, TilePos.Y);

    UE_LOG(LogTemp, Warning, TEXT("=== Tile Stepped ==="));
    UE_LOG(LogTemp, Warning, TEXT("Stepped: (%d, %d)"), IntTilePos.X, IntTilePos.Y);
    UE_LOG(LogTemp, Warning, TEXT("CurrentPathIndex: %d"), CurrentPathIndex);

    if (CorrectPath.IsValidIndex(CurrentPathIndex))
    {
        FIntPoint Expected = CorrectPath[CurrentPathIndex];
        UE_LOG(LogTemp, Warning, TEXT("Expected: (%d, %d)"), Expected.X, Expected.Y);
    }

    if (CurrentState == EPuzzleState::Ready)
    {
        if (ShouldStartPuzzle(SteppedTile))
        {
            StartPuzzle();
        }
        return;
    }

    if (CurrentState != EPuzzleState::Playing || bGamePaused)
    {
        return;
    }

    if (CorrectPath.Num() == 0)
    {
        return;
    }

    bool bIsCorrect = IsCorrectNextStep(IntTilePos.X, IntTilePos.Y);
    UE_LOG(LogTemp, Warning, TEXT("Result: %s"), bIsCorrect ? TEXT("CORRECT") : TEXT("WRONG"));

    if (bIsCorrect)
    {
        SteppedTile->SetTileState(ETileState::Correct);
        PlaySound(CorrectStepSound);

        MarkStepAsCompleted(IntTilePos);
        CurrentPathIndex++;
        OnCorrectStep(SteppedTile);

        if (ConnectedDisplay)
        {
            ConnectedDisplay->UpdateProgress(CurrentPathIndex, CorrectPath.Num());
        }

        OnProgressUpdate.Broadcast(CurrentPathIndex);

        if (CurrentPathIndex < CorrectPath.Num())
        {
            FIntPoint NextStep = CorrectPath[CurrentPathIndex];
            AGridTile* NextTile = GetTileAt(NextStep.X, NextStep.Y);
            if (NextTile)
            {
                NextTile->SetTileState(ETileState::FirstStep);
            }
        }

        if (CurrentPathIndex >= CorrectPath.Num())
        {
            CompletePuzzle();
        }
    }
    else
    {
        SteppedTile->SetTileState(ETileState::Wrong);
        PlaySound(WrongStepSound);

        if (ConnectedDisplay)
        {
            ConnectedDisplay->ShowMessage(TEXT("WRONG!"), FLinearColor::Red);
        }

        GetWorld()->GetTimerManager().SetTimer(WrongTileTimer, [this, SteppedTile]()
            {
                if (SteppedTile && IsValid(SteppedTile))
                {
                    SteppedTile->SetTileState(ETileState::Ready);
                }

                CurrentPathIndex = 0;

                SetAllTilesReady();

                if (CorrectPath.Num() > 0)
                {
                    FIntPoint FirstStep = CorrectPath[0];
                    AGridTile* FirstTile = GetTileAt(FirstStep.X, FirstStep.Y);
                    if (FirstTile)
                    {
                        FirstTile->SetTileState(ETileState::FirstStep);
                    }
                }

                if (ConnectedDisplay)
                {
                    ConnectedDisplay->ShowMessage(TEXT("TRY AGAIN"), ReadyColor);
                }

            }, 0.5f, false);

        OnWrongStep(SteppedTile);
    }

    OnCustomTileStep(SteppedTile, bIsCorrect);
}

AGridTile* AGridMazeManager::GetTileAt(int32 X, int32 Y)
{
    if (!IsValidPosition(X, Y)) return nullptr;
    int32 Index = Y * GridColumns + X;
    return GridTiles.IsValidIndex(Index) ? GridTiles[Index] : nullptr;
}

void AGridMazeManager::CreateGrid()
{
    CreateTilesInternal();
}

void AGridMazeManager::ClearGrid()
{
    ClearGridTiles();
}

void AGridMazeManager::SetAllTilesInactive()
{
    for (AGridTile* Tile : GridTiles)
    {
        if (Tile && IsValid(Tile))
        {
            Tile->SetTileState(ETileState::Inactive);
        }
    }
}

void AGridMazeManager::SetAllTilesReady()
{
    for (AGridTile* Tile : GridTiles)
    {
        if (Tile && IsValid(Tile))
        {
            Tile->SetTileState(ETileState::Ready);
        }
    }
}

// ============ 미리보기 제어 ============

void AGridMazeManager::ShowPathPreviewSequence()
{
    if (CorrectPath.Num() == 0)
    {
        return;
    }

    OnPreviewStarted();
    ShowNextPreviewTile(0);
}

void AGridMazeManager::ShowNextPreviewTile(int32 Index)
{
    UE_LOG(LogTemp, Warning, TEXT("=== ShowNextPreviewTile: Index=%d ==="), Index);

    if (Index >= CorrectPath.Num())
    {
        UE_LOG(LogTemp, Warning, TEXT("Preview Complete! Total shown: %d"), Index);

        GetWorld()->GetTimerManager().SetTimer(PreviewTimerHandle, [this]()
            {
                bIsShowingPreview = false;
                SetAllTilesReady();

                if (CorrectPath.Num() > 0)
                {
                    FIntPoint FirstStep = CorrectPath[0];
                    AGridTile* FirstTile = GetTileAt(FirstStep.X, FirstStep.Y);
                    if (FirstTile)
                    {
                        FirstTile->SetTileState(ETileState::FirstStep);
                        UE_LOG(LogTemp, Warning, TEXT("First tile set at (%d,%d)"), FirstStep.X, FirstStep.Y);
                    }
                }

                if (ConnectedDisplay)
                {
                    ConnectedDisplay->ShowMessage(TEXT("START!"), ReadyColor);
                }

                OnPreviewFinished();

            }, TileLightDelay, false);

        return;
    }

    FIntPoint CurrentPoint = CorrectPath[Index];
    UE_LOG(LogTemp, Warning, TEXT("Correct Path[%d] = (%d, %d)"), Index, CurrentPoint.X, CurrentPoint.Y);

    AGridTile* CurrentTile = GetTileAt(CurrentPoint.X, CurrentPoint.Y);

    if (CurrentTile)
    {
        FVector2D TileGridPos = CurrentTile->GetGridPosition();
        UE_LOG(LogTemp, Warning, TEXT("Tile found! Grid Position: (%.0f, %.0f)"), TileGridPos.X, TileGridPos.Y);

        CurrentTile->SetTileState(ETileState::Preview);

        if (Index == 0)
        {
            CurrentTile->SetLightIntensity(CurrentTile->PreviewLightIntensity * 1.5f);
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("ERROR: Tile NOT FOUND at (%d, %d)!"), CurrentPoint.X, CurrentPoint.Y);
    }

    GetWorld()->GetTimerManager().SetTimer(PreviewTimerHandle, [this, Index]()
        {
            ShowNextPreviewTile(Index + 1);
        }, TileLightDelay, false);
}

void AGridMazeManager::StopPreview()
{
    bIsShowingPreview = false;
    GetWorld()->GetTimerManager().ClearTimer(PreviewTimerHandle);
}

// ============ 시작 위치 제어 ============

void AGridMazeManager::CreateStartingFloor()
{
    if (!StartingFloorMesh || !bUseStartingFloor) return;


    StartingFloorMesh->SetRelativeLocation(StartingFloorPosition);

    FVector Scale = FVector(
        StartingFloorSize.X / 100.0f,
        StartingFloorSize.Y / 100.0f,
        StartingFloorSize.Z / 100.0f
    );
    StartingFloorMesh->SetRelativeScale3D(Scale);

    SetStartingFloorColor(StartingFloorColor);

#if WITH_EDITOR
    if (GetWorld() && GetWorld()->WorldType == EWorldType::Editor)
    {
        StartingFloorMesh->SetVisibility(true);
    }
    else
#endif
    {
        StartingFloorMesh->SetVisibility(false);  // 게임 중엔 숨김
    }
}

void AGridMazeManager::SetStartingFloorColor(FLinearColor NewColor)
{
    StartingFloorColor = NewColor;

    if (StartingFloorMesh)
    {
        UMaterialInstanceDynamic* DynamicMaterial = StartingFloorMesh->CreateAndSetMaterialInstanceDynamic(0);
        if (DynamicMaterial)
        {
            DynamicMaterial->SetVectorParameterValue(TEXT("Color"), NewColor);
        }
    }
}

FVector AGridMazeManager::GetStartingFloorLocation() const
{
    return GetActorLocation() + StartingFloorPosition;
}

void AGridMazeManager::RespawnPlayerToStart(AActor* Player)
{
    if (!Player || !bUseStartingFloor) return;

    FVector SpawnLocation = GetStartingFloorLocation();
    SpawnLocation.Z += 100.0f;

    Player->SetActorLocation(SpawnLocation);
    OnPlayerRespawned(Player);
}

// ============ 진행도 추적 ============

void AGridMazeManager::MarkStepAsCompleted(const FIntPoint& Position)
{
    if (!CompletedSteps.Contains(Position))
    {
        CompletedSteps.Add(Position);
    }
}

void AGridMazeManager::MarkStepAsFailed(const FIntPoint& Position)
{
    if (!FailedSteps.Contains(Position))
    {
        FailedSteps.Add(Position);
    }
}

bool AGridMazeManager::IsStepCompleted(const FIntPoint& Position) const
{
    return CompletedSteps.Contains(Position);
}

bool AGridMazeManager::IsStepFailed(const FIntPoint& Position) const
{
    return FailedSteps.Contains(Position);
}

void AGridMazeManager::ClearProgressHistory()
{
    CompletedSteps.Empty();
    FailedSteps.Empty();
}

void AGridMazeManager::RestoreProgressColors()
{
    SetAllTilesReady();

    for (const FIntPoint& CompletedPos : CompletedSteps)
    {
        AGridTile* Tile = GetTileAt(CompletedPos.X, CompletedPos.Y);
        if (Tile)
        {
            Tile->SetTileState(ETileState::Correct);
        }
    }

    for (const FIntPoint& FailedPos : FailedSteps)
    {
        AGridTile* Tile = GetTileAt(FailedPos.X, FailedPos.Y);
        if (Tile)
        {
            Tile->SetTileState(ETileState::Wrong);
        }
    }
}

// ============ 경로 검증 ============

bool AGridMazeManager::ValidateCorrectPath()
{
    if (CorrectPath.Num() < 2)
    {
        return false;
    }

    for (int32 i = 0; i < CorrectPath.Num(); i++)
    {
        FIntPoint Point = CorrectPath[i];
        if (!IsValidPosition(Point.X, Point.Y))
        {
            return false;
        }
    }

    for (int32 i = 1; i < CorrectPath.Num(); i++)
    {
        FIntPoint Current = CorrectPath[i];
        FIntPoint Previous = CorrectPath[i - 1];

        int32 DistanceX = FMath::Abs(Current.X - Previous.X);
        int32 DistanceY = FMath::Abs(Current.Y - Previous.Y);

        if ((DistanceX > 1) || (DistanceY > 1) || (DistanceX + DistanceY != 1))
        {
            return false;
        }
    }

    StartPosition = CorrectPath[0];
    GoalPosition = CorrectPath.Last();

    return true;
}

// ============ 설정 변경 ============

void AGridMazeManager::SetGridSize(int32 NewRows, int32 NewColumns)
{
    if (NewRows > 0 && NewRows <= 10 && NewColumns > 0 && NewColumns <= 10)
    {
        GridRows = NewRows;
        GridColumns = NewColumns;

#if WITH_EDITOR
        if (GetWorld() && GetWorld()->WorldType == EWorldType::Editor)
        {
            ClearGridTiles();
            CreateTilesInternal();
        }
#endif
    }
}

void AGridMazeManager::SetTimeLimit(float NewTimeLimit)
{
    if (NewTimeLimit > 0.0f)
    {
        PuzzleTimeLimit = NewTimeLimit;
        if (CurrentState == EPuzzleState::Ready || CurrentState == EPuzzleState::Playing)
        {
            TimeRemaining = NewTimeLimit;
        }
    }
}

void AGridMazeManager::SetCorrectPath(const TArray<FIntPoint>& NewPath)
{
    CorrectPath = NewPath;

    if (CorrectPath.Num() >= 2)
    {
        StartPosition = CorrectPath[0];
        GoalPosition = CorrectPath.Last();
    }

    ValidateCorrectPath();
}

void AGridMazeManager::SetFailResetDelay(float NewDelay)
{
    if (NewDelay >= 0.0f)
    {
        FailResetDelay = NewDelay;
    }
}

void AGridMazeManager::SetTileThickness(float NewThickness)
{
    if (NewThickness > 0.0f)
    {
        TileThickness = NewThickness;
        UpdateTileThickness();
    }
}

void AGridMazeManager::SetPuzzleColors(FLinearColor Inactive, FLinearColor Ready, FLinearColor Correct, FLinearColor Wrong)
{
    InactiveColor = Inactive;
    ReadyColor = Ready;
    CorrectColor = Correct;
    WrongColor = Wrong;

    ApplyTileColors();
}

// ============ 정보 조회 ============

float AGridMazeManager::GetProgress() const
{
    if (CorrectPath.Num() == 0)
    {
        return 0.0f;
    }

    return static_cast<float>(CurrentPathIndex) / static_cast<float>(CorrectPath.Num());
}

FString AGridMazeManager::GetProgressText() const
{
    return FString::Printf(TEXT("(%d/%d)"), CurrentPathIndex, CorrectPath.Num());
}

FIntPoint AGridMazeManager::GetPathStepAt(int32 Index) const
{
    if (CorrectPath.IsValidIndex(Index))
    {
        return CorrectPath[Index];
    }
    return FIntPoint(-1, -1);
}

// ============ 에디터 도구 ============

void AGridMazeManager::EditorCreateGrid()
{
    ClearGridTiles();
    CreateTilesInternal();
    if (bUseStartingFloor)
    {
        CreateStartingFloor();
    }
}

void AGridMazeManager::EditorClearGrid()
{
    ClearGridTiles();
}

// ============ 내부 함수들 ============

void AGridMazeManager::UpdateTimer(float DeltaTime)
{
    if (TimeRemaining > 0.0f)
    {
        TimeRemaining -= DeltaTime;

        OnTimerUpdate.Broadcast(TimeRemaining);

        if (TimeRemaining <= 10.0f && TimeRemaining > 9.0f)
        {
            OnTimeWarning(TimeRemaining);
        }

        if (TimeRemaining <= 0.0f)
        {
            TimeRemaining = 0.0f;
            OnTimerUpdate.Broadcast(0.0f);

            if (ConnectedDisplay)
            {
                ConnectedDisplay->ShowMessage(TEXT("TIME OUT!"), FLinearColor::Red);
            }

            FailPuzzle();
        }
    }
}

void AGridMazeManager::SetPuzzleState(EPuzzleState NewState)
{
    if (CurrentState != NewState)
    {
        CurrentState = NewState;
        OnPuzzleStateChanged.Broadcast(NewState);
    }
}

void AGridMazeManager::ConnectToDisplay()
{
    if (ConnectedDisplay)
    {
        return;
    }

    TArray<AActor*> FoundDisplays;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AMazeDisplay::StaticClass(), FoundDisplays);

    if (FoundDisplays.Num() > 0)
    {
        ConnectedDisplay = Cast<AMazeDisplay>(FoundDisplays[0]);
        if (ConnectedDisplay)
        {
            ConnectedDisplay->ConnectToManager(this);
        }
    }
}

void AGridMazeManager::CreateTilesInternal()
{
    if (!TileClass || GridRows <= 0 || GridColumns <= 0)
    {
        return;
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    static bool bIsCreatingTiles = false;
    if (bIsCreatingTiles)
    {
        return;
    }

    bIsCreatingTiles = true;

    DestroyAllTiles();
    GridTiles.SetNum(GridRows * GridColumns);

    //  루프 순서 변경: Y가 바깥, X가 안쪽
    for (int32 Y = 0; Y < GridColumns; Y++)
    {
        for (int32 X = 0; X < GridRows; X++)
        {
            FVector SpawnLocation = CalculateTilePosition(X, Y);

            FActorSpawnParameters SpawnParams;
            SpawnParams.Owner = this;
            SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

            AGridTile* NewTile = World->SpawnActor<AGridTile>(
                TileClass, SpawnLocation, FRotator::ZeroRotator, SpawnParams);

            if (NewTile && IsValid(NewTile))
            {
                NewTile->SetOwner(this);
                NewTile->AttachToActor(this, FAttachmentTransformRules::KeepWorldTransform);
                NewTile->SetOwnerManager(this);
                NewTile->SetGridPosition(X, Y);
                NewTile->SetTileThickness(TileThickness);

                int32 Index = Y * GridColumns + X;  // 이제 올바름
                if (GridTiles.IsValidIndex(Index))
                {
                    GridTiles[Index] = NewTile;
                }

                NewTile->SetTileState(ETileState::Inactive);
            }
        }
    }

    bIsCreatingTiles = false;
}

void AGridMazeManager::ClearGridTiles()
{
    DestroyAllTiles();
}

void AGridMazeManager::UpdateTilePositions()
{
    for (int32 i = 0; i < GridTiles.Num(); i++)
    {
        AGridTile* Tile = GridTiles[i];
        if (Tile && IsValid(Tile))
        {
            int32 X = Tile->GetGridPosition().X;
            int32 Y = Tile->GetGridPosition().Y;
            FVector NewPosition = CalculateTilePosition(X, Y);
            Tile->SetActorLocation(NewPosition);
        }
    }
}

void AGridMazeManager::UpdateTileThickness()
{
    for (AGridTile* Tile : GridTiles)
    {
        if (Tile && IsValid(Tile))
        {
            Tile->SetTileThickness(TileThickness);
        }
    }
}

FVector AGridMazeManager::CalculateTilePosition(int32 X, int32 Y)
{
    float TotalSize = TileSize + TileSpacing;
    float OffsetX = (GridRows - 1) * TotalSize * 0.5f;
    float OffsetY = (GridColumns - 1) * TotalSize * 0.5f;

    FVector LocalPosition = FVector(
        X * TotalSize - OffsetX,
        Y * TotalSize - OffsetY,
        0.0f
    );

    return GetActorLocation() + GetActorRotation().RotateVector(LocalPosition);
}

bool AGridMazeManager::IsValidPosition(int32 X, int32 Y)
{
    return X >= 0 && X < GridRows && Y >= 0 && Y < GridColumns;
}

bool AGridMazeManager::IsCorrectNextStep(int32 X, int32 Y)
{
    if (!CorrectPath.IsValidIndex(CurrentPathIndex))
    {
        return false;
    }

    FIntPoint ExpectedStep = CorrectPath[CurrentPathIndex];
    return (X == ExpectedStep.X && Y == ExpectedStep.Y);
}

void AGridMazeManager::PlaySound(USoundBase* Sound)
{
    if (Sound && GetWorld())
    {
        UGameplayStatics::PlaySoundAtLocation(GetWorld(), Sound, GetActorLocation(), SoundVolume);
    }
}

void AGridMazeManager::ApplyTileColors()
{
    for (AGridTile* Tile : GridTiles)
    {
        if (Tile && IsValid(Tile))
        {
            ETileState CurrentTileState = Tile->GetTileState();
            Tile->SetTileState(CurrentTileState);
        }
    }
}

void AGridMazeManager::DestroyAllTiles()
{
    for (int32 i = GridTiles.Num() - 1; i >= 0; i--)
    {
        AGridTile* Tile = GridTiles[i];
        if (Tile && IsValid(Tile))
        {
            if (Tile->OnTileStepped.IsBound())
            {
                Tile->OnTileStepped.RemoveAll(this);
            }
            Tile->Destroy();
        }
    }
    GridTiles.Empty();

    if (GetWorld())
    {
        TArray<AActor*> AllActors;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), AGridTile::StaticClass(), AllActors);

        for (AActor* Actor : AllActors)
        {
            if (Actor && Actor->GetOwner() == this)
            {
                Actor->Destroy();
            }
        }
    }

    TArray<AActor*> AttachedActors;
    GetAttachedActors(AttachedActors);

    for (AActor* AttachedActor : AttachedActors)
    {
        if (AGridTile* GridTile = Cast<AGridTile>(AttachedActor))
        {
            AttachedActor->Destroy();
        }
    }
}

void AGridMazeManager::ResetToStartPosition()
{
    CurrentPathIndex = 0;
    SetPuzzleState(EPuzzleState::Playing);

    if (bKeepProgressOnFail)
    {
        RestoreProgressColors();
    }
    else
    {
        SetAllTilesReady();
    }
}