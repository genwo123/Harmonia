// Pedestal.h
#pragma once
#include "CoreMinimal.h"
#include "Interaction/InteractableActor.h"
#include "Components/SphereComponent.h"
#include "Gameplay/PuzzleArea.h"
#include "Pedestal.generated.h"

// ��ħ�� ���� ������
UENUM(BlueprintType)
enum class EPedestalState : uint8
{
    Empty,        // �������
    Occupied      // �������� ��ġ��
};

UCLASS()
class DISTRICT_TEST_API APedestal : public AInteractableActor
{
    GENERATED_BODY()
public:
    APedestal();

    UFUNCTION(BlueprintCallable, Category = "Pedestal")
    AActor* GetPlacedObject() const { return PlacedObject; }

    // ��ȣ�ۿ� �������̵� (�б�, ȸ��, ������ ��ġ ��)
    virtual void Interact_Implementation(AActor* Interactor) override;
    // ��ħ�� ���� ������ ��ġ
    UFUNCTION(BlueprintCallable, Category = "Pedestal")
    bool PlaceObject(AActor* Object);
    // ��ħ�� ���� ������ ����
    UFUNCTION(BlueprintCallable, Category = "Pedestal")
    AActor* RemoveObject();
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
    // ��ħ�� �ʱ�ȭ (�׸��� ��ġ ���)
    virtual void BeginPlay() override;
    // �Ҽӵ� ���� ������ ã��
    void FindOwnerPuzzleArea();
};