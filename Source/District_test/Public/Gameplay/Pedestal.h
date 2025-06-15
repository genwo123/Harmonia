// Pedestal.h
#pragma once
#include "CoreMinimal.h"
#include "Interaction/InteractableActor.h"
#include "Gameplay/PuzzleArea.h"
#include "Components/SphereComponent.h"
#include "Pedestal.generated.h"

// ��ħ�� ���� ������
UENUM(BlueprintType)
enum class EPedestalState : uint8
{
    Empty,        // �������
    Occupied      // �������� ��ġ��
};

UCLASS(BlueprintType, Blueprintable)
class DISTRICT_TEST_API APedestal : public AInteractableActor
{
    GENERATED_BODY()
public:
    APedestal();

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components", meta = (AllowPrivateAccess = "true"))
    USceneComponent* AttachmentPoint;

    // ��ȣ�ۿ� �������̵� (�б�, ȸ��, ������ ��ġ ��)
    virtual void Interact_Implementation(AActor* Interactor) override;

    // ��ħ�� ���� ������ ��ġ
    UFUNCTION(BlueprintCallable, Category = "Pedestal")
    bool PlaceObject(AActor* Object);

    // ��ħ�� ���� ������ ����
    UFUNCTION(BlueprintCallable, Category = "Pedestal")
    AActor* RemoveObject();

    // ��ħ�� ���� ������ ��������
    UFUNCTION(BlueprintCallable, Category = "Pedestal")
    AActor* GetPlacedObject() const { return PlacedObject; }

    // ��ħ�� ȸ��
    UFUNCTION(BlueprintCallable, Category = "Pedestal")
    void Rotate(float Degrees = 90.0f);

    // ��ħ�� �б� (Ư�� �������� �� ĭ)
    UFUNCTION(BlueprintCallable, Category = "Pedestal")
    bool Push(FVector Direction);

    // ���� ������ ����
    UFUNCTION(BlueprintCallable, Category = "Pedestal")
    void SetPuzzleArea(APuzzleArea* PuzzleArea);

    // �׸��� ��ġ ����
    UFUNCTION(BlueprintCallable, Category = "Pedestal")
    void SetGridPosition(int32 Row, int32 Column);

    // ���� �׸��� ��ġ ��������
    UFUNCTION(BlueprintCallable, Category = "Pedestal")
    void GetGridPosition(int32& OutRow, int32& OutColumn) const;

    // �׸��� �߾ӿ� �ڵ����� �����ϴ� �Լ�
    UFUNCTION(BlueprintCallable, Category = "Pedestal")
    void SnapToGridCenter();

    // ��ȣ�ۿ� ���� - �������Ʈ���� ��ġ ���� ����
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Interaction")
    USphereComponent* InteractionSphere;

    // �����Ϳ��� ������ Ÿ�� ���� ������ �� �׸��� ��ǥ
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid", meta = (ExposeOnSpawn = true, AllowedClasses = "PuzzleArea"))
    TObjectPtr<APuzzleArea> TargetPuzzleArea;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid", meta = (ExposeOnSpawn = true))
    int32 TargetGridRow = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid", meta = (ExposeOnSpawn = true))
    int32 TargetGridColumn = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pedestal")
    bool bObjectFollowsRotation = true;


    // Ư�� �׸��� ��ġ�� �̵��ϴ� �Լ�
    UFUNCTION(BlueprintCallable, Category = "Grid")
    bool MoveToGridPosition(int32 NewRow, int32 NewColumn);

#if WITH_EDITOR
    // �����Ϳ��� ���� �̵� �� ȣ��Ǵ� �Լ�
    virtual void PostEditMove(bool bFinished) override;
#endif

protected:
    // ���� ��ħ�� ����
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pedestal")
    EPedestalState CurrentState;

    // ��ħ�� ���� ��ġ�� ������
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pedestal")
    AActor* PlacedObject;

    // �Ҽӵ� ���� ������ (�б�, ȸ�� �� ����)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pedestal")
    APuzzleArea* OwnerPuzzleArea;

    // ���� �׸��� ��ġ
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pedestal")
    int32 GridRow;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pedestal")
    int32 GridColumn;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
    bool bAutoSnapToGrid = true;


    // ��ħ�� �ʱ�ȭ (�׸��� ��ġ ���)
    virtual void BeginPlay() override;
    virtual void OnConstruction(const FTransform& Transform) override;

    // �Ҽӵ� ���� ������ ã��
    void FindOwnerPuzzleArea();

    // ���� �� ������ �����ϴ� �Լ�
    void ClearPreviousCell();

    // ������ �̺�Ʈ �ڵ鷯
    UFUNCTION()
    void OnInteractionSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
        bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnInteractionSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
};