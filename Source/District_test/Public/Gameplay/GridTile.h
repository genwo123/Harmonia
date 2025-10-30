// GridTile.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/SphereComponent.h"
#include "Interaction/InteractableInterface.h"
#include "GridTile.generated.h"

UENUM(BlueprintType)
enum class ETileState : uint8
{
    Inactive    UMETA(DisplayName = "Inactive"),      // 비활성화 (꺼짐)
    Preview     UMETA(DisplayName = "Preview"),       // 미리보기 (파란색)
    Ready       UMETA(DisplayName = "Ready"),         // 대기 상태 (주황색)
    FirstStep   UMETA(DisplayName = "FirstStep"),     // 첫 스텝 표시 (파란색 깜박임)
    Correct     UMETA(DisplayName = "Correct"),       // 정답 (초록색)
    Wrong       UMETA(DisplayName = "Wrong")          // 오답 (빨간색)
};

class AGridTile;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTileSteppedDelegate, AGridTile*, SteppedTile, AActor*, SteppedActor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTileStateChangedDelegate, ETileState, NewState);

UCLASS(Blueprintable, BlueprintType)
class DISTRICT_TEST_API AGridTile : public AActor, public IInteractableInterface
{
    GENERATED_BODY()

public:
    AGridTile();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
    UStaticMeshComponent* TileMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
    UStaticMeshComponent* PillarMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
    UPointLightComponent* TileLight;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
    USphereComponent* InteractionSphere;

public:
    // ============ BP에서 설정 가능한 색상들 ============

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile Colors|Inactive")
    FLinearColor InactiveColor = FLinearColor(0.1f, 0.1f, 0.1f, 1.0f);  // 회색

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile Colors|Preview")
    FLinearColor PreviewColor = FLinearColor(0.2f, 0.5f, 1.0f, 1.0f);  // 파란색

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile Colors|Ready")
    FLinearColor ReadyColor = FLinearColor(1.0f, 0.6f, 0.2f, 1.0f);  // 주황색

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile Colors|FirstStep")
    FLinearColor FirstStepColor = FLinearColor(0.3f, 0.7f, 1.0f, 1.0f);  // 밝은 파란색

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile Colors|Correct")
    FLinearColor CorrectColor = FLinearColor(0.2f, 1.0f, 0.2f, 1.0f);  // 초록색

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile Colors|Wrong")
    FLinearColor WrongColor = FLinearColor(1.0f, 0.2f, 0.2f, 1.0f);  // 빨간색

    // ============ 빛 세기 설정 ============

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Light Settings")
    float InactiveLightIntensity = 0.0f;  // 꺼진 상태

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Light Settings")
    float PreviewLightIntensity = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Light Settings")
    float ReadyLightIntensity = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Light Settings")
    float FirstStepLightIntensity = 2500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Light Settings")
    float CorrectLightIntensity = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Light Settings")
    float WrongLightIntensity = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Light Settings")
    float LightRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Light Settings")
    float LightHeight = 100.0f;

    // ============ 애니메이션 설정 ============

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings")
    bool bEnableBlinking = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings")
    float BlinkSpeed = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings")
    bool bBlinkOnFirstStep = true;  // 첫 스텝일 때 깜박임

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings")
    bool bBlinkOnWrong = true;  // 오답일 때 깜박임

    // ============ 사운드 설정 ============

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound Settings")
    bool bPlayInteractionSound = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound Settings")
    class USoundBase* InteractionSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound Settings")
    class USoundBase* CorrectSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound Settings")
    class USoundBase* WrongSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound Settings")
    float SoundVolume = 1.0f;

    // ============ 현재 상태 ============

    UPROPERTY(BlueprintReadOnly, Category = "Current State")
    ETileState CurrentState = ETileState::Inactive;

    UPROPERTY(BlueprintReadOnly, Category = "Current State")
    int32 GridX = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Current State")
    int32 GridY = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Current State")
    bool bIsBlinking = false;

    UPROPERTY(BlueprintReadOnly, Category = "Current State")
    bool bIsActivated = false;

    UPROPERTY(BlueprintReadOnly, Category = "Current State")
    bool bCanBeInteracted = true;

public:
    // ============ 이벤트 델리게이트 ============

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnTileSteppedDelegate OnTileStepped;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnTileStateChangedDelegate OnTileStateChanged;

    UPROPERTY(BlueprintReadOnly, Category = "Manager")
    class AGridMazeManager* OwnerManager;

    // ============ BP에서 호출 가능한 주요 함수들 ============

    // 타일 상태 변경
    UFUNCTION(BlueprintCallable, Category = "Tile Control")
    void SetTileState(ETileState NewState);

    // 현재 상태 가져오기
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Tile Information")
    ETileState GetTileState() const { return CurrentState; }

    // 그리드 위치 설정
    UFUNCTION(BlueprintCallable, Category = "Tile Control")
    void SetGridPosition(int32 X, int32 Y);

    // 그리드 위치 가져오기
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Tile Information")
    FVector2D GetGridPosition() const { return FVector2D(GridX, GridY); }

    // 매니저 설정
    UFUNCTION(BlueprintCallable, Category = "Manager")
    void SetOwnerManager(class AGridMazeManager* Manager);

    // 타일 두께 설정
    UFUNCTION(BlueprintCallable, Category = "Tile Settings")
    void SetTileThickness(float NewThickness);

    // ============ 빛 제어 함수들 ============

    // 빛 켜기
    UFUNCTION(BlueprintCallable, Category = "Light Control")
    void TurnOnLight(FLinearColor Color, float Intensity);

    // 빛 끄기
    UFUNCTION(BlueprintCallable, Category = "Light Control")
    void TurnOffLight();

    // 빛 색상만 변경
    UFUNCTION(BlueprintCallable, Category = "Light Control")
    void SetLightColor(FLinearColor NewColor);

    // 빛 세기만 변경
    UFUNCTION(BlueprintCallable, Category = "Light Control")
    void SetLightIntensity(float NewIntensity);

    // ============ 애니메이션 제어 ============

    // 깜박임 시작
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void StartBlinking(float Duration = 0.0f);  // 0이면 무한

    // 깜박임 중지
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void StopBlinking();

    // ============ 유틸리티 함수들 ============

    // 기본 상태로 리셋
    UFUNCTION(BlueprintCallable, Category = "Appearance")
    void ResetToDefault();

    // 사운드 재생
    UFUNCTION(BlueprintCallable, Category = "Sound")
    void PlayTileSound(USoundBase* Sound);

    UFUNCTION(BlueprintCallable, Category = "Sound")
    void PlayCorrectSound();

    UFUNCTION(BlueprintCallable, Category = "Sound")
    void PlayWrongSound();

    // 상호작용 가능 여부 설정
    UFUNCTION(BlueprintCallable, Category = "Interaction")
    void SetCanBeInteracted(bool bCanInteract);

    // 현재 색상 가져오기
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Information")
    FLinearColor GetCurrentStateColor() const;

    // 현재 빛 세기 가져오기
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Information")
    float GetCurrentLightIntensity() const;

    // ============ InteractableInterface 구현 ============

    virtual void Interact_Implementation(AActor* Interactor) override;
    virtual bool CanInteract_Implementation(AActor* Interactor) override;
    virtual FString GetInteractionText_Implementation() override;
    virtual EInteractionType GetInteractionType_Implementation() override;

    // ============ BP 이벤트들 ============

    UFUNCTION(BlueprintImplementableEvent, Category = "Blueprint Events")
    void OnPlayerInteracted(AActor* Player);

    UFUNCTION(BlueprintImplementableEvent, Category = "Blueprint Events")
    void OnStateChanged(ETileState OldState, ETileState NewState);

    UFUNCTION(BlueprintImplementableEvent, Category = "Blueprint Events")
    void OnCorrectInteraction();

    UFUNCTION(BlueprintImplementableEvent, Category = "Blueprint Events")
    void OnWrongInteraction();

    UFUNCTION(BlueprintImplementableEvent, Category = "Blueprint Events")
    void OnLightTurnedOn(FLinearColor Color, float Intensity);

    UFUNCTION(BlueprintImplementableEvent, Category = "Blueprint Events")
    void OnLightTurnedOff();

protected:
    FTimerHandle BlinkTimerHandle;

private:
    void UpdateBlinking(float DeltaTime);
    void ForceMobilitySettings();
    void ApplyStateVisuals(ETileState State);
};