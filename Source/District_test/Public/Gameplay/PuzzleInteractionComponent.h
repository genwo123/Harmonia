// PuzzleInteractionComponent.h
#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PuzzleInteractionComponent.generated.h"

UCLASS(ClassGroup = (Puzzle), meta = (BlueprintSpawnableComponent))
class DISTRICT_TEST_API UPuzzleInteractionComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UPuzzleInteractionComponent();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // ���� �� ������Ʈ�� ��ħ�뿡 ��ġ�Ǿ� �ִ��� ����
    UPROPERTY(BlueprintReadOnly, Category = "Puzzle")
    class APedestal* CurrentPedestal;

    // ������Ʈ�� ����� �� �ִ��� ����
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Puzzle")
    bool bCanBePickedUp;

    // ������Ʈ�� ��ħ�뿡 ���� �� �ִ��� ����
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Puzzle")
    bool bCanBePlacedOnPedestal;

    // ������Ʈ ���� �Լ� (�÷��̾ ȣ��)
    UFUNCTION(BlueprintCallable, Category = "Interaction")
    bool PickUp(AActor* Picker);

    // ������Ʈ �������� �Լ� (�÷��̾ ȣ��)
    UFUNCTION(BlueprintCallable, Category = "Interaction")
    bool PutDown(FVector Location, FRotator Rotation);

    // ��ħ�뿡 ��ġ �Լ�
    UFUNCTION(BlueprintCallable, Category = "Interaction")
    bool PlaceOnPedestal(class APedestal* Pedestal);

    // ��ħ�뿡�� ���� �Լ�
    UFUNCTION(BlueprintCallable, Category = "Interaction")
    bool RemoveFromPedestal();

    // �� ������Ʈ�� ��� �ִ� ���� (�Ϲ������� �÷��̾� ĳ����)
    UPROPERTY(BlueprintReadOnly, Category = "Interaction")
    AActor* HoldingActor;
};