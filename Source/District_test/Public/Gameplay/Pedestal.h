// Pedestal.h
#pragma once
#include "CoreMinimal.h"
#include "Interaction/InteractableActor.h"
#include "Components/SphereComponent.h"
#include "Gameplay/PuzzleArea.h"
#include "Pedestal.generated.h"

// 받침대 상태 열거형
UENUM(BlueprintType)
enum class EPedestalState : uint8
{
    Empty,        // 비어있음
    Occupied      // 오브제가 설치됨
};

UCLASS()
class DISTRICT_TEST_API APedestal : public AInteractableActor
{
    GENERATED_BODY()
public:
    APedestal();

    UFUNCTION(BlueprintCallable, Category = "Pedestal")
    AActor* GetPlacedObject() const { return PlacedObject; }

    // 상호작용 오버라이드 (밀기, 회전, 오브제 설치 등)
    virtual void Interact_Implementation(AActor* Interactor) override;
    // 받침대 위에 오브제 설치
    UFUNCTION(BlueprintCallable, Category = "Pedestal")
    bool PlaceObject(AActor* Object);
    // 받침대 위의 오브제 제거
    UFUNCTION(BlueprintCallable, Category = "Pedestal")
    AActor* RemoveObject();
    // 받침대 회전
    UFUNCTION(BlueprintCallable, Category = "Pedestal")
    void Rotate(float Degrees = 90.0f);
    // 받침대 밀기 (특정 방향으로 한 칸)
    UFUNCTION(BlueprintCallable, Category = "Pedestal")
    bool Push(FVector Direction);
    // 퍼즐 에리어 설정
    UFUNCTION(BlueprintCallable, Category = "Pedestal")
    void SetPuzzleArea(APuzzleArea* PuzzleArea);
    // 그리드 위치 설정
    UFUNCTION(BlueprintCallable, Category = "Pedestal")
    void SetGridPosition(int32 Row, int32 Column);
    // 현재 그리드 위치 가져오기
    UFUNCTION(BlueprintCallable, Category = "Pedestal")
    void GetGridPosition(int32& OutRow, int32& OutColumn) const;
protected:
    // 현재 받침대 상태
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pedestal")
    EPedestalState CurrentState;
    // 받침대 위에 설치된 오브제
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pedestal")
    AActor* PlacedObject;
    // 소속된 퍼즐 에리어 (밀기, 회전 시 참조)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pedestal")
    APuzzleArea* OwnerPuzzleArea;
    // 현재 그리드 위치
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pedestal")
    int32 GridRow;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pedestal")
    int32 GridColumn;
    // 받침대 초기화 (그리드 위치 계산)
    virtual void BeginPlay() override;
    // 소속된 퍼즐 에리어 찾기
    void FindOwnerPuzzleArea();
};