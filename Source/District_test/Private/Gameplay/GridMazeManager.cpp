
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

    // Create Starting Floor Mesh Component
    StartingFloorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StartingFloorMesh"));
    StartingFloorMesh->SetupAttachment(RootComponent);

    // Load default cube mesh for floor
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMeshAsset(TEXT("/Engine/BasicShapes/Cube"));
    if (CubeMeshAsset.Succeeded())
    {
        StartingFloorMesh->SetStaticMesh(CubeMeshAsset.Object);
    }

    StartingFloorMesh->SetMobility(EComponentMobility::Movable);
    StartingFloorMesh->SetSimulatePhysics(false);
    StartingFloorMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    StartingFloorMesh->SetCollisionResponseToAllChannels(ECR_Block);

    // �⺻ ����
    FailResetDelay = 3.0f;
    bContinueTimeOnFail = true;
    bKeepProgressOnFail = true;
}

void AGridMazeManager::BeginPlay()
{
    Super::BeginPlay();

    // ���� ���� �� Ÿ�� ����
    CreateTilesInternal();

    // ���� �ٴ� ����
    if (bUseStartingFloor)
    {
        CreateStartingFloor();
    }

    if (bAutoFindDisplay)
    {
        ConnectToDisplay();
    }

    // ��� ��ȿ�� �˻�
    ValidateCorrectPath();

    TimeRemaining = PuzzleTimeLimit;
    SetPuzzleState(EPuzzleState::Ready);

    // ��� Ÿ���� ��� ����(ȸ��)�� ����
    SetAllTilesWaiting();

    UE_LOG(LogTemp, Warning, TEXT("GridMazeManager BeginPlay - Path length: %d"), CorrectPath.Num());
}

void AGridMazeManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // ���� ���¿����� �ð��� ��� �귯�� ��! (�߿� ����)
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

    if (bShowPreviewInEditor && TileClass && GridSize > 0)
    {
        ClearGridTiles();
        CreateTilesInternal();

        if (bUseStartingFloor)
        {
            CreateStartingFloor();
        }

        if (bShowPathInPreview && CorrectPath.Num() > 0)
        {
            ValidateCorrectPath();
            ShowPathPreview();
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

    // Transform ���� �� ��ġ�� ������Ʈ
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

    // CorrectPath �迭 ���� �� �ڵ� ����
    if (PropertyName == TEXT("CorrectPath"))
    {
        ValidateCorrectPath();
        if (bShowPathInPreview) ShowPathPreview();
        return;
    }

    // Starting Floor ���� ���� ����
    if (PropertyName == TEXT("StartingFloorPosition") ||
        PropertyName == TEXT("StartingFloorSize") ||
        PropertyName == TEXT("StartingFloorColor"))
    {
        if (bUseStartingFloor)
        {
            CreateStartingFloor();
        }
        return;
    }

    // bUseStartingFloor ���� ��
    if (PropertyName == TEXT("bUseStartingFloor"))
    {
        if (bUseStartingFloor)
        {
            CreateStartingFloor();
        }
        else
        {
            if (StartingFloorMesh)
            {
                StartingFloorMesh->SetVisibility(false);
            }
        }
        return;
    }

    // �׸��� ���� ���� �� �����
    if (PropertyName == TEXT("GridSize") ||
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
            if (bShowPathInPreview && CorrectPath.Num() > 0)
            {
                ValidateCorrectPath();
                ShowPathPreview();
            }
        }
        else
        {
            ClearGridTiles();
        }
    }
    else if (PropertyName == TEXT("bShowPathInPreview"))
    {
        if (bShowPathInPreview && CorrectPath.Num() > 0)
        {
            ShowPathPreview();
        }
    }
}
#endif

// ====== ���� ���� (������ ����) ======
void AGridMazeManager::StartPuzzle()
{
    if (CurrentState == EPuzzleState::Ready)
    {
        // ��� ��ȿ�� ��˻�
        if (!ValidateCorrectPath())
        {
            UE_LOG(LogTemp, Error, TEXT("Cannot start puzzle - invalid path!"));
            return;
        }

        SetPuzzleState(EPuzzleState::Playing);
        TimeRemaining = PuzzleTimeLimit;
        CurrentPathIndex = 0;  // �߿�: 0���� ����!
        bGamePaused = false;

        // ���� ����� �����Ǿ�� �ϸ� ����
        if (bKeepProgressOnFail)
        {
            RestoreProgressColors();
        }
        else
        {
            // ��� Ÿ���� ���� �غ� ����(�ʷϻ�)�� ����
            SetAllTilesReady();
        }

        OnPuzzleStarted();

        UE_LOG(LogTemp, Warning, TEXT("Puzzle started! Expected first step: (%d, %d)"),
            CorrectPath[0].X, CorrectPath[0].Y);
    }
}

void AGridMazeManager::StartPuzzleWithCountdown()
{
    if (CurrentState == EPuzzleState::Ready && ConnectedDisplay)
    {
        // Display���� ī��Ʈ�ٿ� ���� ��ȣ
        ConnectedDisplay->SetDisplayState(EDisplayState::Countdown);
        UE_LOG(LogTemp, Warning, TEXT("Puzzle countdown started"));
    }
    else
    {
        // Display�� ������ �ٷ� ����
        StartPuzzle();
    }
}

void AGridMazeManager::ResetPuzzle()
{
    SetPuzzleState(EPuzzleState::Ready);
    TimeRemaining = PuzzleTimeLimit;
    CurrentPathIndex = 0;
    bGamePaused = false;

    // ���� ��� ���� ���ο� ���� �ٸ��� ó��
    if (bKeepProgressOnFail)
    {
        RestoreProgressColors();  // ����/���� ��� ����
    }
    else
    {
        SetAllTilesWaiting();     // ��� Ÿ�� ȸ������
        ClearProgressHistory();   // ��� ����
    }

    CustomResetLogic();

    UE_LOG(LogTemp, Warning, TEXT("Puzzle reset - Keep progress: %s"),
        bKeepProgressOnFail ? TEXT("Yes") : TEXT("No"));
}

void AGridMazeManager::CompletePuzzle()
{
    SetPuzzleState(EPuzzleState::Success);
    PlaySound(SuccessSound);

    if (ConnectedDisplay)
    {
        ConnectedDisplay->ShowMessage(TEXT("SUCCESS!"));
    }

    OnPuzzleCompleted();

    UE_LOG(LogTemp, Warning, TEXT("Puzzle completed successfully!"));
}

void AGridMazeManager::FailPuzzle()
{
    SetPuzzleState(EPuzzleState::Failed);
    PlaySound(FailSound);

    if (ConnectedDisplay)
    {
        ConnectedDisplay->ShowMessage(TEXT("FAILED!"));
    }

    OnPuzzleFailed();

    // ������ �ð� �� �÷��̾ ���� ��ġ�� �̵� (�ð� ���� ����!)
    GetWorld()->GetTimerManager().SetTimer(ResetTimer, [this]()
        {
            // �÷��̾� ã�� �� ������
            APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
            if (PlayerPawn)
            {
                RespawnPlayerToStart(PlayerPawn);
            }
            ResetToStartPosition();  // ResetPuzzle ��� �ε巯�� ����
        }, FailResetDelay, false);  // �����Ϳ��� ���� ����!

    UE_LOG(LogTemp, Warning, TEXT("Puzzle failed - Reset in %.1f seconds"), FailResetDelay);
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

// ====== Ÿ�� ��ȣ�ۿ� (������ ����) ======
void AGridMazeManager::OnTileStep(AGridTile* SteppedTile, AActor* Player)
{
    if (!SteppedTile) return;

    FVector2D TilePos = SteppedTile->GetGridPosition();
    FIntPoint IntTilePos = FIntPoint(TilePos.X, TilePos.Y);  // FVector2D�� FIntPoint�� ��ȯ

    UE_LOG(LogTemp, Warning, TEXT("Tile stepped: (%d, %d), Expected: (%d, %d), Index: %d"),
        IntTilePos.X, IntTilePos.Y,
        CorrectPath.IsValidIndex(CurrentPathIndex) ? CorrectPath[CurrentPathIndex].X : -1,
        CorrectPath.IsValidIndex(CurrentPathIndex) ? CorrectPath[CurrentPathIndex].Y : -1,
        CurrentPathIndex);

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

    // ��ΰ� �����Ǿ� �ִ��� Ȯ��
    if (CorrectPath.Num() == 0)
    {
        UE_LOG(LogTemp, Error, TEXT("No correct path set!"));
        return;
    }

    bool bIsCorrect = IsCorrectNextStep(IntTilePos.X, IntTilePos.Y);

    if (bIsCorrect)
    {
        // ����! - �Ķ������� ����
        SteppedTile->SetTileState(ETileState::Correct);
        MarkStepAsCompleted(IntTilePos);  // ���� ��� ����
        CurrentPathIndex++;
        PlaySound(CorrectStepSound);
        OnCorrectStep(SteppedTile);

        // ���൵ ������Ʈ
        if (ConnectedDisplay)
        {
            ConnectedDisplay->UpdateProgress(CurrentPathIndex, CorrectPath.Num());
        }

        UE_LOG(LogTemp, Warning, TEXT("Correct step! Progress: %d/%d"), CurrentPathIndex, CorrectPath.Num());

        // ���� �Ϸ� üũ
        if (CurrentPathIndex >= CorrectPath.Num())
        {
            CompletePuzzle();
        }
    }
    else
    {
        // Ʋ����! - ���������� ����
        SteppedTile->SetTileState(ETileState::Wrong);
        MarkStepAsFailed(IntTilePos);  // ���� ��� ����
        PlaySound(WrongStepSound);
        OnWrongStep(SteppedTile);

        UE_LOG(LogTemp, Warning, TEXT("Wrong step! Expected (%d, %d), got (%d, %d)"),
            CorrectPath[CurrentPathIndex].X, CorrectPath[CurrentPathIndex].Y,
            IntTilePos.X, IntTilePos.Y);

        FailPuzzle();
    }

    OnCustomTileStep(SteppedTile, bIsCorrect);
}

// ====== ���� ��� ���� (���ο� ���) ======
void AGridMazeManager::MarkStepAsCompleted(const FIntPoint& Position)
{
    if (!CompletedSteps.Contains(Position))
    {
        CompletedSteps.Add(Position);
        UE_LOG(LogTemp, Warning, TEXT("Step (%d, %d) marked as completed"), Position.X, Position.Y);
    }
}

void AGridMazeManager::MarkStepAsFailed(const FIntPoint& Position)
{
    if (!FailedSteps.Contains(Position))
    {
        FailedSteps.Add(Position);
        UE_LOG(LogTemp, Warning, TEXT("Step (%d, %d) marked as failed"), Position.X, Position.Y);
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
    UE_LOG(LogTemp, Warning, TEXT("Progress history cleared"));
}

void AGridMazeManager::RestoreProgressColors()
{
    // ���� ��� Ÿ���� �ʷϻ�����
    SetAllTilesReady();

    // �� ���� ����� ��Ͽ� ���� ���� ����
    for (const FIntPoint& CompletedPos : CompletedSteps)
    {
        AGridTile* Tile = GetTileAt(CompletedPos.X, CompletedPos.Y);
        if (Tile)
        {
            Tile->SetTileState(ETileState::Correct);  // �Ķ��� ����
        }
    }

    for (const FIntPoint& FailedPos : FailedSteps)
    {
        AGridTile* Tile = GetTileAt(FailedPos.X, FailedPos.Y);
        if (Tile)
        {
            Tile->SetTileState(ETileState::Wrong);    // ������ ����
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("Progress colors restored - Completed: %d, Failed: %d"),
        CompletedSteps.Num(), FailedSteps.Num());
}

// ====== Starting Floor Functions ======
void AGridMazeManager::CreateStartingFloor()
{
    if (!StartingFloorMesh || !bUseStartingFloor) return;

    // ��ġ ���� (Manager ���� ��� ��ġ)
    StartingFloorMesh->SetRelativeLocation(StartingFloorPosition);

    // ũ�� ���� (������ �ٴ� ����)
    FVector Scale = FVector(
        StartingFloorSize.X / 100.0f,  // �⺻ Cube�� 100x100x100
        StartingFloorSize.Y / 100.0f,
        StartingFloorSize.Z / 100.0f
    );
    StartingFloorMesh->SetRelativeScale3D(Scale);

    // ���� ����
    SetStartingFloorColor(StartingFloorColor);

    // ���̱�
    StartingFloorMesh->SetVisibility(true);

    UE_LOG(LogTemp, Warning, TEXT("Starting Floor created at: %s with size: %s"),
        *StartingFloorPosition.ToString(), *StartingFloorSize.ToString());
}

void AGridMazeManager::SetStartingFloorColor(FLinearColor NewColor)
{
    StartingFloorColor = NewColor;

    if (StartingFloorMesh)
    {
        // ���� ��Ƽ���� �ν��Ͻ� ���� �� ���� ����
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
    SpawnLocation.Z += 100.0f; // �ٴ� ���� ��¦ �ø���

    Player->SetActorLocation(SpawnLocation);

    UE_LOG(LogTemp, Warning, TEXT("Player respawned to starting floor at: %s"), *SpawnLocation.ToString());
}

// GridMazeManager.cpp - Complete Implementation with Access Fixes
// Add these missing functions to the end of your existing GridMazeManager.cpp file

// ====== Settings Functions ======
void AGridMazeManager::SetCorrectPath(const TArray<FIntPoint>& NewPath)
{
    CorrectPath = NewPath;

    if (CorrectPath.Num() >= 2)
    {
        StartPosition = CorrectPath[0];
        GoalPosition = CorrectPath.Last();
    }

    // Auto validate path
    ValidateCorrectPath();

    UE_LOG(LogTemp, Warning, TEXT("Correct path updated - %d steps from (%d, %d) to (%d, %d)"),
        CorrectPath.Num(),
        StartPosition.X, StartPosition.Y,
        GoalPosition.X, GoalPosition.Y);
}

void AGridMazeManager::SetFailResetDelay(float NewDelay)
{
    if (NewDelay >= 0.0f)
    {
        FailResetDelay = NewDelay;
        UE_LOG(LogTemp, Warning, TEXT("Fail reset delay set to %.1f seconds"), FailResetDelay);
    }
}

void AGridMazeManager::SetGridSize(int32 NewSize)
{
    if (NewSize > 0 && NewSize <= 10)
    {
        GridSize = NewSize;

        // Recreate grid when size changes
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

// ====== Information Query Functions ======
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

// ====== Tile Management Functions ======
AGridTile* AGridMazeManager::GetTileAt(int32 X, int32 Y)
{
    if (!IsValidPosition(X, Y)) return nullptr;
    int32 Index = Y * GridSize + X;
    return GridTiles.IsValidIndex(Index) ? GridTiles[Index] : nullptr;
}

void AGridMazeManager::CreateGrid()
{
    CreateTilesInternal();
    UE_LOG(LogTemp, Warning, TEXT("Grid created via Blueprint call"));
}

void AGridMazeManager::ClearGrid()
{
    ClearGridTiles();
    UE_LOG(LogTemp, Warning, TEXT("Grid cleared via Blueprint call"));
}

void AGridMazeManager::EditorCreateGrid()
{
    ClearGridTiles();
    CreateTilesInternal();
    if (bUseStartingFloor)
    {
        CreateStartingFloor();
    }
    if (bShowPathInPreview && CorrectPath.Num() > 0)
    {
        ValidateCorrectPath();
        ShowPathPreview();
    }
    UE_LOG(LogTemp, Warning, TEXT("Grid created via Editor"));
}

void AGridMazeManager::EditorClearGrid()
{
    ClearGridTiles();
    UE_LOG(LogTemp, Warning, TEXT("Grid cleared via Editor"));
}

// ====== Path Validation ======
bool AGridMazeManager::ValidateCorrectPath()
{
    if (CorrectPath.Num() < 2)
    {
        UE_LOG(LogTemp, Warning, TEXT("Path validation: Need at least 2 points"));
        return false;
    }

    // Check if all coordinates are within grid bounds
    for (int32 i = 0; i < CorrectPath.Num(); i++)
    {
        FIntPoint Point = CorrectPath[i];
        if (!IsValidPosition(Point.X, Point.Y))
        {
            UE_LOG(LogTemp, Error, TEXT("Point %d (%d, %d) is outside grid (0,0) to (%d,%d)"),
                i, Point.X, Point.Y, GridSize - 1, GridSize - 1);
            return false;
        }
    }

    // Check if adjacent coordinates are connected
    for (int32 i = 1; i < CorrectPath.Num(); i++)
    {
        FIntPoint Current = CorrectPath[i];
        FIntPoint Previous = CorrectPath[i - 1];

        int32 DistanceX = FMath::Abs(Current.X - Previous.X);
        int32 DistanceY = FMath::Abs(Current.Y - Previous.Y);

        // Only allow single step moves (no diagonal)
        if ((DistanceX > 1) || (DistanceY > 1) || (DistanceX + DistanceY != 1))
        {
            UE_LOG(LogTemp, Error, TEXT("Invalid move from (%d, %d) to (%d, %d) - only adjacent moves allowed"),
                Previous.X, Previous.Y, Current.X, Current.Y);
            return false;
        }
    }

    // Auto update Start and Goal Position
    StartPosition = CorrectPath[0];
    GoalPosition = CorrectPath.Last();

    UE_LOG(LogTemp, Warning, TEXT("Path validated: %d steps from (%d, %d) to (%d, %d)"),
        CorrectPath.Num(),
        StartPosition.X, StartPosition.Y,
        GoalPosition.X, GoalPosition.Y);

    return true;
}

// ====== Private Functions Implementation ======
void AGridMazeManager::UpdateTimer(float DeltaTime)
{
    if (TimeRemaining > 0.0f)
    {
        TimeRemaining -= DeltaTime;

        // Broadcast timer update event
        OnTimerUpdate.Broadcast(TimeRemaining);

        // Check time warnings
        if (TimeRemaining <= 10.0f && TimeRemaining > 9.0f)
        {
            OnTimeWarning(TimeRemaining);
        }

        // Check timeout
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

        UE_LOG(LogTemp, Warning, TEXT("Puzzle state changed to: %d"), (int32)NewState);
    }
}

void AGridMazeManager::ConnectToDisplay()
{
    if (ConnectedDisplay)
    {
        return;  // Already connected
    }

    // Find MazeDisplay in world
    TArray<AActor*> FoundDisplays;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AMazeDisplay::StaticClass(), FoundDisplays);

    if (FoundDisplays.Num() > 0)
    {
        ConnectedDisplay = Cast<AMazeDisplay>(FoundDisplays[0]);
        if (ConnectedDisplay)
        {
            ConnectedDisplay->ConnectToManager(this);
            UE_LOG(LogTemp, Warning, TEXT("Auto-connected to MazeDisplay"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("No MazeDisplay found in world"));
    }
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

void AGridMazeManager::ShowPathPreview()
{
    if (!bShowPathInPreview || CorrectPath.Num() == 0)
    {
        return;
    }

    // Set all tiles to default state first
    SetAllTilesWaiting();

    // Highlight path tiles with different colors (Editor preview)
    for (int32 i = 0; i < CorrectPath.Num(); i++)
    {
        FIntPoint PathPoint = CorrectPath[i];
        AGridTile* Tile = GetTileAt(PathPoint.X, PathPoint.Y);

        if (Tile)
        {
            if (i == 0)
            {
                // Start point - bright green
                Tile->SetTileState(ETileState::StartPoint);
            }
            else if (i == CorrectPath.Num() - 1)
            {
                // Goal point - yellow
                Tile->SetTileState(ETileState::Goal);
            }
            else
            {
                // Path - hint color (purple)
                Tile->SetTileState(ETileState::Hint);
            }
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("Path preview shown for %d steps"), CorrectPath.Num());
}

FVector AGridMazeManager::CalculateTilePosition(int32 X, int32 Y)
{
    float TotalSize = TileSize + TileSpacing;
    float OffsetX = (GridSize - 1) * TotalSize * 0.5f;
    float OffsetY = (GridSize - 1) * TotalSize * 0.5f;

    FVector LocalPosition = FVector(
        X * TotalSize - OffsetX,
        Y * TotalSize - OffsetY,
        0.0f
    );

    return GetActorLocation() + GetActorRotation().RotateVector(LocalPosition);
}

bool AGridMazeManager::IsValidPosition(int32 X, int32 Y)
{
    return X >= 0 && X < GridSize && Y >= 0 && Y < GridSize;
}

bool AGridMazeManager::IsCorrectNextStep(int32 X, int32 Y)
{
    if (!CorrectPath.IsValidIndex(CurrentPathIndex))
    {
        UE_LOG(LogTemp, Error, TEXT("CurrentPathIndex %d is invalid for path length %d"), CurrentPathIndex, CorrectPath.Num());
        return false;
    }

    FIntPoint ExpectedStep = CorrectPath[CurrentPathIndex];
    bool bIsCorrect = (X == ExpectedStep.X && Y == ExpectedStep.Y);

    UE_LOG(LogTemp, Warning, TEXT("Checking step (%d, %d) vs expected (%d, %d) at index %d: %s"),
        X, Y, ExpectedStep.X, ExpectedStep.Y, CurrentPathIndex,
        bIsCorrect ? TEXT("CORRECT") : TEXT("WRONG"));

    return bIsCorrect;
}

void AGridMazeManager::PlaySound(USoundBase* Sound)
{
    if (Sound && GetWorld())
    {
        UGameplayStatics::PlaySoundAtLocation(GetWorld(), Sound, GetActorLocation());
    }
}

void AGridMazeManager::ApplyTileColors()
{
    // FIXED: Instead of calling UpdateFromManagerColors(), just refresh tile states
    for (AGridTile* Tile : GridTiles)
    {
        if (Tile && IsValid(Tile))
        {
            // Force tile to re-evaluate its color using current state
            ETileState CurrentTileState = Tile->GetTileState();
            Tile->SetTileState(CurrentTileState);  // This will apply manager colors
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("Applied manager colors to %d tiles"), GridTiles.Num());
}

void AGridMazeManager::DestroyAllTiles()
{
    // Delete regular tiles
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

    // Find remaining tiles in world and delete them
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

    // Delete attached actors
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

// ====== Tile State Management (GridTile Integration) ======
void AGridMazeManager::SetAllTilesWaiting()
{
    for (AGridTile* Tile : GridTiles)
    {
        if (Tile && IsValid(Tile))
        {
            Tile->SetTileState(ETileState::Default); // Gray (waiting)
        }
    }
    UE_LOG(LogTemp, Warning, TEXT("All tiles set to WAITING (gray)"));
}

void AGridMazeManager::SetAllTilesReady()
{
    for (AGridTile* Tile : GridTiles)
    {
        if (Tile && IsValid(Tile))
        {
            Tile->SetTileState(ETileState::Start); // Green (ready)
        }
    }
    UE_LOG(LogTemp, Warning, TEXT("All tiles set to READY (green)"));
}

void AGridMazeManager::ResetToStartPosition()
{
    CurrentPathIndex = 0;
    SetPuzzleState(EPuzzleState::Playing);

    // Restore colors while keeping progress record
    if (bKeepProgressOnFail)
    {
        RestoreProgressColors();
    }
    else
    {
        SetAllTilesReady();
    }

    UE_LOG(LogTemp, Warning, TEXT("Reset to start position - Ready to continue"));
}

// GridMazeManager.cpp�� �߰��� CreateTilesInternal �Լ�

void AGridMazeManager::CreateTilesInternal()
{
    if (!TileClass || GridSize <= 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot create tiles - TileClass is null or GridSize is invalid"));
        return;
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("World is null - cannot create tiles"));
        return;
    }

    // Prevent recursive calls
    static bool bIsCreatingTiles = false;
    if (bIsCreatingTiles)
    {
        UE_LOG(LogTemp, Warning, TEXT("Already creating tiles - preventing recursive call"));
        return;
    }

    bIsCreatingTiles = true;

    // Clear existing tiles first
    DestroyAllTiles();
    GridTiles.SetNum(GridSize * GridSize);

    UE_LOG(LogTemp, Warning, TEXT("Creating %dx%d grid (%d total tiles)"), GridSize, GridSize, GridSize * GridSize);

    // Create grid tiles
    for (int32 X = 0; X < GridSize; X++)
    {
        for (int32 Y = 0; Y < GridSize; Y++)
        {
            FVector SpawnLocation = CalculateTilePosition(X, Y);

            FActorSpawnParameters SpawnParams;
            SpawnParams.Owner = this;
            SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

            AGridTile* NewTile = World->SpawnActor<AGridTile>(
                TileClass, SpawnLocation, FRotator::ZeroRotator, SpawnParams);

            if (NewTile && IsValid(NewTile))
            {
                // Set up tile properties
                NewTile->SetOwner(this);
                NewTile->AttachToActor(this, FAttachmentTransformRules::KeepWorldTransform);
                NewTile->SetOwnerManager(this);
                NewTile->SetGridPosition(X, Y);


                // Store in grid array
                int32 Index = Y * GridSize + X;
                if (GridTiles.IsValidIndex(Index))
                {
                    GridTiles[Index] = NewTile;
                }

                // Set initial state
                NewTile->SetTileState(ETileState::Default);

                UE_LOG(LogTemp, Log, TEXT("Created tile at (%d, %d) - Index: %d"), X, Y, Index);
            }
            else
            {
                UE_LOG(LogTemp, Error, TEXT("Failed to create tile at (%d, %d)"), X, Y);
            }
        }
    }

    bIsCreatingTiles = false;

    UE_LOG(LogTemp, Warning, TEXT("Grid creation completed - %d tiles created successfully"), GridTiles.Num());
}

void AGridMazeManager::CompleteReset()
{
    SetPuzzleState(EPuzzleState::Ready);
    TimeRemaining = PuzzleTimeLimit;
    CurrentPathIndex = 0;
    bGamePaused = false;

    // ���� �ʱ�ȭ - ��� ���� ��� ����
    ClearProgressHistory();   // CompletedSteps, FailedSteps ����
    SetAllTilesReady();      // ��� Ÿ�� �ʷϻ����� (Ready ����)

    CustomResetLogic();

    UE_LOG(LogTemp, Warning, TEXT("Complete reset - Ready for countdown start"));
}