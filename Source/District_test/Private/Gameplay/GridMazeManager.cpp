// GridMazeManager.cpp - 완전한 정리된 버전
#include "Gameplay/GridMazeManager.h"
#include "Gameplay/GridTile.h"
#include "Gameplay/MazeDisplay.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Sound/SoundBase.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

#if WITH_EDITOR
#include "Editor.h"
#include "UnrealEd.h"
#endif

AGridMazeManager::AGridMazeManager()
{
    PrimaryActorTick.bCanEverTick = true;
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
}

void AGridMazeManager::BeginPlay()
{
    Super::BeginPlay();

    // 모든 타일을 완전히 제거
    UE_LOG(LogTemp, Warning, TEXT("BeginPlay: Clearing ALL tiles"));
    ClearAllTiles(TEXT(""));

    //  람다 함수 문법 수정 (this 캡처 추가)
    FTimerHandle DelayTimer;
    GetWorld()->GetTimerManager().SetTimer(DelayTimer, [this]()
        {
            this->SetupDifficultySettings();
            this->CreateGameGrid();
            this->GenerateCorrectPath();

            if (bAutoFindDisplay && !ConnectedDisplay)
            {
                this->FindConnectedDisplay();
            }

            this->SetMazeState(EMazeState::Ready);

        }, 0.1f, false);
}

AGridMazeManager::~AGridMazeManager()
{
    // 소멸자에서 정리
    UE_LOG(LogTemp, Warning, TEXT("GridMazeManager destructor called"));
}

void AGridMazeManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    UE_LOG(LogTemp, Warning, TEXT("GridMazeManager EndPlay called - cleaning up tiles"));

    // 모든 소유한 타일들 강제 삭제
    if (GetWorld())
    {
        TArray<AActor*> AllGridTiles;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), AGridTile::StaticClass(), AllGridTiles);

        int32 DeletedCount = 0;
        for (AActor* Actor : AllGridTiles)
        {
            if (Actor && Actor->GetOwner() == this)
            {
                Actor->Destroy();
                DeletedCount++;
            }
        }

        UE_LOG(LogTemp, Warning, TEXT("Deleted %d tiles during EndPlay"), DeletedCount);
    }

    GridTiles.Empty();

    Super::EndPlay(EndPlayReason);
}

void AGridMazeManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // 타이머 업데이트
    if (CurrentState == EMazeState::Running && bHasTimeLimit)
    {
        UpdateTimer(DeltaTime);
    }
}

// ====== UNIFIED GRID MANAGEMENT ======
void AGridMazeManager::CreateGameGrid()
{
    ClearAllTiles(TEXT("GameTile"));
    CreateTiles(TEXT("GameTile"), true); // 델리게이트 연결
    UE_LOG(LogTemp, Log, TEXT("Game grid created: %dx%d (%d tiles)"), GridSize, GridSize, GridTiles.Num());
}

void AGridMazeManager::CreateEditorGrid()
{
#if WITH_EDITOR
    ClearAllTiles(TEXT("EditorPreview"));
    CreateTiles(TEXT("EditorPreview"), false); // 델리게이트 연결 안함
    UE_LOG(LogTemp, Log, TEXT("Editor preview created: %dx%d (%d tiles)"), GridSize, GridSize, GridTiles.Num());
#endif
}

void AGridMazeManager::CreateTiles(const FString& TileTag, bool bConnectDelegates)
{
    if (!TileClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("TileClass is not set in GridMazeManager!"));
        return;
    }

    GridTiles.Empty();
    GridTiles.SetNum(GridSize * GridSize);

    for (int32 X = 0; X < GridSize; X++)
    {
        for (int32 Y = 0; Y < GridSize; Y++)
        {
            FVector SpawnLocation = CalculateTileWorldPosition(X, Y);

            // 스폰 파라미터 설정
            FActorSpawnParameters SpawnParams;
            SpawnParams.Owner = this;
            SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

            AGridTile* NewTile = GetWorld()->SpawnActor<AGridTile>(
                TileClass,
                SpawnLocation,
                FRotator::ZeroRotator,
                SpawnParams
            );

            if (NewTile)
            {
                //  핵심: 타일을 Manager에 부착해서 계층 구조 만들기
                NewTile->AttachToActor(this, FAttachmentTransformRules::KeepWorldTransform);

                // 기본 설정
                NewTile->SetGridPosition(X, Y);
                NewTile->Tags.Add(*TileTag);

                // 게임용일 때만 Manager 연결 및 델리게이트 연결
                if (bConnectDelegates)
                {
                    NewTile->SetOwnerManager(this);
                    NewTile->OnTileStepped.AddDynamic(this, &AGridMazeManager::OnTileStepped);
                }

                int32 Index = CoordinateToIndex(X, Y);
                GridTiles[Index] = NewTile;

                // 시작점과 목표점 설정
                if (FVector2D(X, Y) == StartPosition)
                {
                    NewTile->SetTileState(ETileState::Start);
                }
                else if (FVector2D(X, Y) == GoalPosition)
                {
                    NewTile->SetTileState(ETileState::Goal);
                }
            }
        }
    }
}



void AGridMazeManager::ClearAllTiles(const FString& TileTag)
{
    UE_LOG(LogTemp, Warning, TEXT("Clearing tiles with tag: %s"), *TileTag);

    if (!GetWorld()) return;

    // 1. GridTiles 배열의 타일들 삭제
    for (AGridTile* Tile : GridTiles)
    {
        if (Tile && IsValid(Tile))
        {
            if (TileTag.IsEmpty() || Tile->Tags.Contains(*TileTag))
            {
                UE_LOG(LogTemp, Log, TEXT("Destroying tile: %s"), *Tile->GetName());
                Tile->Destroy();
            }
        }
    }

    // 2. 모든 GridTile 검색해서 내 소유인 것들 삭제
    TArray<AActor*> AllGridTiles;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AGridTile::StaticClass(), AllGridTiles);

    int32 RemovedCount = 0;
    for (AActor* Actor : AllGridTiles)
    {
        if (Actor && Actor->GetOwner() == this)
        {
            AGridTile* GridTile = Cast<AGridTile>(Actor);
            if (GridTile && (TileTag.IsEmpty() || GridTile->Tags.Contains(*TileTag)))
            {
                UE_LOG(LogTemp, Log, TEXT("Destroying owned tile: %s"), *GridTile->GetName());
                GridTile->Destroy();
                RemovedCount++;
            }
        }
    }

    GridTiles.Empty();

    UE_LOG(LogTemp, Warning, TEXT("Cleared tiles - Removed: %d"), RemovedCount);
}
// ====== LEGACY FUNCTIONS (호환성을 위해 유지) ======
void AGridMazeManager::CreateGrid()
{
    CreateGameGrid();
}

void AGridMazeManager::ClearGrid()
{
    ClearAllTiles(TEXT("GameTile"));
}

void AGridMazeManager::EditorCreateGrid()
{
#if WITH_EDITOR
    EditorClearGrid();
    SetupDifficultySettings();
    CreateEditorGrid();
    EditorGeneratePath();
    UE_LOG(LogTemp, Warning, TEXT("Editor Grid Created: %dx%d"), GridSize, GridSize);
#endif
}

void AGridMazeManager::EditorClearGrid()
{
#if WITH_EDITOR
    ClearAllTiles(TEXT("EditorPreview"));
    UE_LOG(LogTemp, Warning, TEXT("Editor Grid Cleared"));
#endif
}

void AGridMazeManager::EditorGeneratePath()
{
#if WITH_EDITOR
    GenerateCorrectPath();
    UpdatePathVisualization();
    UE_LOG(LogTemp, Warning, TEXT("Editor Path Generated"));
#endif
}

// ====== CLEAN EDITOR PREVIEW FUNCTIONS ======
#if WITH_EDITOR
void AGridMazeManager::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);

    UWorld* World = GetWorld();
    if (!World || World->WorldType != EWorldType::Editor)
        return;

    // 이미 타일이 있으면 생성하지 않음
    if (GridTiles.Num() > 0)
    {
        bool bHasValidTiles = false;
        for (AGridTile* Tile : GridTiles)
        {
            if (Tile && IsValid(Tile))
            {
                bHasValidTiles = true;
                break;
            }
        }
        if (bHasValidTiles)
        {
            UE_LOG(LogTemp, Log, TEXT("Tiles already exist, skipping OnConstruction"));
            return;
        }
    }

    if (bShowPreviewInEditor && TileClass && GridSize > 0)
    {
        RefreshEditorPreview();
    }
}

void AGridMazeManager::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);

    UWorld* World = GetWorld();
    if (!World || World->WorldType != EWorldType::Editor)
        return;

    if (!bAutoUpdatePreview)
        return;

    if (PropertyChangedEvent.Property)
    {
        FString PropertyName = PropertyChangedEvent.Property->GetName();

        //  Transform 관련 모든 프로퍼티 제외 (더 강화)
        TArray<FString> TransformProperties = {
            TEXT("RelativeLocation"), TEXT("RelativeRotation"), TEXT("RelativeScale3D"),
            TEXT("Location"), TEXT("Rotation"), TEXT("Scale3D"),
            TEXT("Transform"), TEXT("ActorTransform"), TEXT("RootComponent"),
            TEXT("AttachParent"), TEXT("AttachSocketName")
        };

        if (TransformProperties.Contains(PropertyName))
        {
            UE_LOG(LogTemp, Log, TEXT("Ignoring transform property change: %s"), *PropertyName);
            return; // Transform 관련 변경은 완전히 무시
        }

        TArray<FString> UpdateProperties = {
            TEXT("GridSize"), TEXT("TileClass")
        };

        if (UpdateProperties.Contains(PropertyName))
        {
            FTimerHandle UpdateTimer;
            GetWorld()->GetTimerManager().SetTimer(UpdateTimer, [this]()
                {
                    RefreshEditorPreview();
                }, 0.1f, false); // 0.01f → 0.1f로 증가
        }
    }
}

#endif


void AGridMazeManager::SetStartAndGoalTiles()
{
    // 이전 시작/목표점 초기화
    for (AGridTile* Tile : GridTiles)
    {
        if (Tile && (Tile->GetTileState() == ETileState::Start || Tile->GetTileState() == ETileState::Goal))
        {
            Tile->SetTileState(ETileState::Default);
        }
    }

    // 새로운 시작/목표점 설정
    AGridTile* StartTile = GetTileAt(ManualStartPosition.X, ManualStartPosition.Y);
    AGridTile* GoalTile = GetTileAt(ManualGoalPosition.X, ManualGoalPosition.Y);

    if (StartTile)
    {
        StartTile->SetTileState(ETileState::Start);
        StartPosition = ManualStartPosition;
    }
    if (GoalTile)
    {
        ClearAllTiles(TEXT(""));
        GoalTile->SetTileState(ETileState::Goal);
        GoalPosition = ManualGoalPosition;
    }

    UE_LOG(LogTemp, Warning, TEXT("Start: (%d, %d), Goal: (%d, %d)"),
        (int32)ManualStartPosition.X, (int32)ManualStartPosition.Y,
        (int32)ManualGoalPosition.X, (int32)ManualGoalPosition.Y);
}


void AGridMazeManager::RefreshEditorPreview()
{
#if WITH_EDITOR
    UWorld* World = GetWorld();
    if (!World || World->WorldType != EWorldType::Editor)
        return;

    if (!bShowPreviewInEditor)
    {
        ClearAllTiles(TEXT("EditorPreview"));
        return;
    }

    if (!TileClass || GridSize <= 0 || GridSize > 10)
    {
        UE_LOG(LogTemp, Log, TEXT("GridMazeManager: Invalid settings, skipping preview"));
        return;
    }

    //  중복 생성 방지 체크 추가
    bool bAlreadyHasTiles = false;
    for (AGridTile* Tile : GridTiles)
    {
        if (Tile && IsValid(Tile) && Tile->Tags.Contains(TEXT("EditorPreview")))
        {
            bAlreadyHasTiles = true;
            break;
        }
    }

    if (bAlreadyHasTiles)
    {
        return; // 이미 에디터 프리뷰 타일이 있으면 생성하지 않음
    }

    // 설정 업데이트 및 타일 생성
    SetupDifficultySettings();
    CreateEditorGrid();

    // 경로 생성 및 표시
    if (bShowPathInPreview)
    {
        GenerateCorrectPath();
        ShowPreviewPath();
    }

    UE_LOG(LogTemp, Log, TEXT("Editor preview refreshed: %dx%d grid"), GridSize, GridSize);
#endif
}

void AGridMazeManager::ManualCreateTiles()
{
    UE_LOG(LogTemp, Warning, TEXT("Manual tile creation started"));

    // 기존 타일들 완전 제거
    ManualClearAllTiles();

    // 설정 적용
    SetupDifficultySettings();

    // 타일 생성 (에디터에서는 EditorPreview 태그 사용)
    UWorld* World = GetWorld();
    if (World && World->WorldType == EWorldType::Editor)
    {
        CreateTiles(TEXT("EditorPreview"), false);
    }
    else
    {
        CreateTiles(TEXT("GameTile"), true);
    }

    UE_LOG(LogTemp, Warning, TEXT("Manual tile creation completed: %d tiles"), GridTiles.Num());
}

void AGridMazeManager::ManualClearAllTiles()
{
    UE_LOG(LogTemp, Warning, TEXT("Manual clear all tiles"));

    // 모든 태그의 타일들 제거
    ClearAllTiles(TEXT(""));

    // 혹시 남은 GridTile들도 강제 제거
    TArray<AActor*> AllGridTiles;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AGridTile::StaticClass(), AllGridTiles);

    for (AActor* Actor : AllGridTiles)
    {
        if (Actor && Actor->GetOwner() == this)
        {
            Actor->Destroy();
        }
    }

    GridTiles.Empty();
}
void AGridMazeManager::ShowPreviewPath()
{
#if WITH_EDITOR
    if (!bShowPathInPreview || CorrectPath.Num() == 0)
        return;

    // 정답 경로 타일들을 힌트 색상으로 표시 (시작/끝 제외)
    for (int32 i = 1; i < CorrectPath.Num() - 1; i++)
    {
        FVector2D PathPoint = CorrectPath[i];
        AGridTile* PathTile = GetTileAt(PathPoint.X, PathPoint.Y);
        if (PathTile)
        {
            PathTile->SetTileState(ETileState::Hint);
        }
    }
#endif
}

// ====== PATH GENERATION ======
void AGridMazeManager::GenerateCorrectPath()
{
    CorrectPath.Empty();

    // 에디터에서 설정된 타일들의 bIsPartOfCorrectPath를 읽어서 경로 생성
    for (int32 X = 0; X < GridSize; X++)
    {
        for (int32 Y = 0; Y < GridSize; Y++)
        {
            AGridTile* Tile = GetTileAt(X, Y);
            if (Tile && Tile->IsCorrectPath())
            {
                CorrectPath.Add(FVector2D(X, Y));
            }
        }
    }

    // 자동 시작점/목표점 설정 제거
    /*
    if (CorrectPath.Num() > 0)
    {
        StartPosition = CorrectPath[0];
        GoalPosition = CorrectPath.Last();

        AGridTile* StartTile = GetTileAt(StartPosition.X, StartPosition.Y);
        AGridTile* GoalTile = GetTileAt(GoalPosition.X, GoalPosition.Y);

        if (StartTile) StartTile->SetTileState(ETileState::Start);
        if (GoalTile) GoalTile->SetTileState(ETileState::Goal);
    }
    */

    UE_LOG(LogTemp, Warning, TEXT("Correct path generated from editor settings: %d tiles"), CorrectPath.Num());
}

void AGridMazeManager::UpdatePathVisualization()
{
#if WITH_EDITOR
    if (!bShowPathPreview) return;

    // 모든 타일을 기본 상태로 리셋
    for (AGridTile* Tile : GridTiles)
    {
        if (Tile)
        {
            FVector2D TilePos = Tile->GetGridPosition();
            if (TilePos != StartPosition && TilePos != GoalPosition)
            {
                Tile->SetTileState(ETileState::Default);
            }
        }
    }

    // 정답 경로 표시
    for (int32 i = 0; i < CorrectPath.Num(); i++)
    {
        FVector2D PathPoint = CorrectPath[i];
        AGridTile* Tile = GetTileAt(PathPoint.X, PathPoint.Y);
        if (Tile)
        {
            if (PathPoint == StartPosition)
            {
                Tile->SetTileState(ETileState::Start);
            }
            else if (PathPoint == GoalPosition)
            {
                Tile->SetTileState(ETileState::Goal);
            }
            else
            {
                Tile->SetTileState(ETileState::Hint);
            }
        }
    }
#endif
}

// ====== DIFFICULTY SETTINGS ======
void AGridMazeManager::SetupDifficultySettings()
{
    switch (Difficulty)
    {
    case EMazeDifficulty::Easy:
        GridSize = 3;
        TimeLimit = 60.0f;
        StartPosition = FVector2D(0, 0);
        GoalPosition = FVector2D(2, 2);
        break;

    case EMazeDifficulty::Normal:
        GridSize = 5;
        TimeLimit = 180.0f;
        StartPosition = FVector2D(0, 0);
        GoalPosition = FVector2D(4, 4);
        break;

    case EMazeDifficulty::Hard:
        GridSize = 7;
        TimeLimit = 300.0f;
        StartPosition = FVector2D(0, 0);
        GoalPosition = FVector2D(6, 6);
        break;

    case EMazeDifficulty::Expert:
        GridSize = 8;
        TimeLimit = 420.0f;
        StartPosition = FVector2D(0, 0);
        GoalPosition = FVector2D(7, 7);
        break;

    case EMazeDifficulty::Custom:
        GridSize = CustomGridSize;
        // 커스텀의 경우 시작/목표점은 경로에서 자동 추출됨
        break;
    }
}

// ====== UTILITY FUNCTIONS ======
FVector AGridMazeManager::CalculateTileWorldPosition(int32 X, int32 Y)
{
    float TotalSize = TileSize + TileSpacing;
    float OffsetX = (GridSize - 1) * TotalSize * 0.5f;
    float OffsetY = (GridSize - 1) * TotalSize * 0.5f;

    FVector Position = GetActorLocation();
    Position.X += (X * TotalSize) - OffsetX;
    Position.Y += (Y * TotalSize) - OffsetY;

    return Position;
}

int32 AGridMazeManager::CoordinateToIndex(int32 X, int32 Y)
{
    return Y * GridSize + X;
}

FVector2D AGridMazeManager::IndexToCoordinate(int32 Index)
{
    int32 X = Index % GridSize;
    int32 Y = Index / GridSize;
    return FVector2D(X, Y);
}

bool AGridMazeManager::IsInEditorMode() const
{
    UWorld* World = GetWorld();
    return (World && World->WorldType == EWorldType::Editor);
}

// ====== GAME CONTROL ======
void AGridMazeManager::StartMaze()
{
    if (CurrentState != EMazeState::Ready)
    {
        ResetMaze();
    }

    SetMazeState(EMazeState::Running);
    TimeRemaining = TimeLimit;
    CurrentPathIndex = 0;

    // 첫 번째 힌트 표시 (옵션이 켜져있다면)
    if (bShowHints && CorrectPath.Num() > 1)
    {
        AGridTile* FirstTile = GetTileAt(CorrectPath[1].X, CorrectPath[1].Y);
        if (FirstTile)
        {
            FirstTile->ShowHint(2.0f);
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("Maze started!"));
}

void AGridMazeManager::ResetMaze()
{
    SetMazeState(EMazeState::Ready);
    TimeRemaining = TimeLimit;
    CurrentPathIndex = 0;
    AttemptCount++;

    // 모든 타일을 기본 상태로 리셋
    for (AGridTile* Tile : GridTiles)
    {
        if (Tile)
        {
            FVector2D TilePos = Tile->GetGridPosition();

            if (TilePos == StartPosition)
            {
                Tile->SetTileState(ETileState::Start);
            }
            else if (TilePos == GoalPosition)
            {
                Tile->SetTileState(ETileState::Goal);
            }
            else
            {
                Tile->SetTileState(ETileState::Default);
            }
        }
    }

    BroadcastMazeData();
    UE_LOG(LogTemp, Warning, TEXT("Maze reset! Attempt: %d"), AttemptCount);
}

void AGridMazeManager::PauseMaze()
{
    if (CurrentState == EMazeState::Running)
    {
        SetMazeState(EMazeState::Ready);
    }
}

void AGridMazeManager::ResumeMaze()
{
    if (CurrentState == EMazeState::Ready)
    {
        SetMazeState(EMazeState::Running);
    }
}

void AGridMazeManager::UseHint()
{
    if (!bShowHints || UsedHints >= MaxHints || CurrentState != EMazeState::Running)
        return;

    UsedHints++;

    // 다음에 밟아야 할 타일 힌트 표시
    if (CurrentPathIndex < CorrectPath.Num())
    {
        FVector2D NextPos = CorrectPath[CurrentPathIndex];
        AGridTile* NextTile = GetTileAt(NextPos.X, NextPos.Y);
        if (NextTile)
        {
            NextTile->ShowHint(3.0f);
            OnHintUsed(MaxHints - UsedHints);
        }
    }
}

// ====== TILE INTERACTION ======
void AGridMazeManager::OnTileStepped(AGridTile* SteppedTile, AActor* SteppedActor)
{
    if (CurrentState != EMazeState::Running || !SteppedTile)
        return;

    FVector2D TilePos = SteppedTile->GetGridPosition();

    UE_LOG(LogTemp, Warning, TEXT("Tile stepped: (%d, %d), Expected: (%d, %d)"),
        (int32)TilePos.X, (int32)TilePos.Y,
        CurrentPathIndex < CorrectPath.Num() ? (int32)CorrectPath[CurrentPathIndex].X : -1,
        CurrentPathIndex < CorrectPath.Num() ? (int32)CorrectPath[CurrentPathIndex].Y : -1);

    // 현재 단계에서 밟아야 할 올바른 타일인지 확인
    if (IsCorrectNextStep(TilePos.X, TilePos.Y))
    {
        HandleCorrectStep(SteppedTile);
    }
    else
    {
        HandleWrongStep(SteppedTile);
    }

    // 이벤트 브로드캐스트
    OnTileActivated.Broadcast(TilePos.X, TilePos.Y);
}

bool AGridMazeManager::IsCorrectNextStep(int32 X, int32 Y)
{
    if (CurrentPathIndex >= CorrectPath.Num())
        return false;

    FVector2D ExpectedPosition = CorrectPath[CurrentPathIndex];
    return (ExpectedPosition.X == X && ExpectedPosition.Y == Y);
}

void AGridMazeManager::HandleCorrectStep(AGridTile* Tile)
{
    Tile->SetTileState(ETileState::Correct);
    CurrentPathIndex++;
    PlaySound(CorrectStepSound);

    UE_LOG(LogTemp, Warning, TEXT("Correct step! Progress: %d/%d"), CurrentPathIndex, CorrectPath.Num());

    // 목표에 도달했는지 확인
    if (CurrentPathIndex >= CorrectPath.Num())
    {
        HandleMazeSuccess();
    }
    else
    {
        BroadcastMazeData();
    }
}

void AGridMazeManager::HandleWrongStep(AGridTile* Tile)
{
    Tile->SetTileState(ETileState::Wrong);
    PlaySound(WrongStepSound);

    UE_LOG(LogTemp, Warning, TEXT("Wrong step! Resetting..."));

    if (bResetOnWrongStep)
    {
        // 1초 후 리셋
        FTimerHandle ResetTimer;
        GetWorld()->GetTimerManager().SetTimer(ResetTimer, [this]()
            {
                ResetMaze();
            }, 1.0f, false);
    }
}

void AGridMazeManager::HandleMazeSuccess()
{
    SetMazeState(EMazeState::Success);
    PlaySound(SuccessSound);
    OnMazeComplete.Broadcast(true);

    UE_LOG(LogTemp, Warning, TEXT("Maze completed successfully!"));
}

void AGridMazeManager::HandleMazeFailure()
{
    SetMazeState(EMazeState::Failed);
    PlaySound(FailureSound);
    OnMazeComplete.Broadcast(false);

    if (bAutoResetOnTimeout)
    {
        FTimerHandle ResetTimer;
        GetWorld()->GetTimerManager().SetTimer(ResetTimer, [this]()
            {
                ResetMaze();
            }, 2.0f, false);
    }
}

// ====== TIMER & STATE ======
void AGridMazeManager::UpdateTimer(float DeltaTime)
{
    TimeRemaining -= DeltaTime;

    if (TimeRemaining <= 0.0f)
    {
        TimeRemaining = 0.0f;
        SetMazeState(EMazeState::TimeOut);
        HandleMazeFailure();
    }

    BroadcastMazeData();
}

void AGridMazeManager::BroadcastMazeData()
{
    OnMazeDataUpdate.Broadcast(TimeRemaining, AttemptCount, CurrentState);
}

void AGridMazeManager::SetMazeState(EMazeState NewState)
{
    if (CurrentState != NewState)
    {
        CurrentState = NewState;
        OnMazeStateChanged(NewState);
        BroadcastMazeData();
    }
}

// ====== UTILITY ======
AGridTile* AGridMazeManager::GetTileAt(int32 X, int32 Y)
{
    if (!IsValidGridPosition(X, Y))
        return nullptr;

    int32 Index = CoordinateToIndex(X, Y);
    return GridTiles.IsValidIndex(Index) ? GridTiles[Index] : nullptr;
}

bool AGridMazeManager::IsValidGridPosition(int32 X, int32 Y)
{
    return (X >= 0 && X < GridSize && Y >= 0 && Y < GridSize);
}

FVector2D AGridMazeManager::GetPlayerGridPosition() const
{
    if (CurrentPathIndex > 0 && CurrentPathIndex <= CorrectPath.Num())
    {
        return CorrectPath[CurrentPathIndex - 1];
    }
    return StartPosition;
}

void AGridMazeManager::PlaySound(USoundBase* Sound)
{
    if (bPlaySounds && Sound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, Sound, GetActorLocation());
    }
}

void AGridMazeManager::FindConnectedDisplay()
{
    TArray<AActor*> FoundDisplays;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AMazeDisplay::StaticClass(), FoundDisplays);

    if (FoundDisplays.Num() > 0)
    {
        ConnectedDisplay = Cast<AMazeDisplay>(FoundDisplays[0]);
        if (ConnectedDisplay)
        {
            UE_LOG(LogTemp, Log, TEXT("Auto-connected to MazeDisplay"));
        }
    }
}