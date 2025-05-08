// PickupActor.h
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interaction/InteractableInterface.h"
#include "Interaction/InteractionEnums.h"
#include "Gameplay/Item.h"
#include "PickupActor.generated.h"

UCLASS()
class DISTRICT_TEST_API APickupActor : public AActor, public IInteractableInterface
{
    GENERATED_BODY()
public:
    APickupActor();
protected:
    virtual void BeginPlay() override;
public:
    virtual void Tick(float DeltaTime) override;
    // ������ ������
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup")
    TSubclassOf<UItem> ItemClass;
    // ����� ���� ������ �̸� (�������� ������ �⺻ �̸� ���)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup")
    FString CustomItemName;
    // ����� ���� ������ ���� (�������� ������ �⺻ ���� ���)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup")
    FString CustomItemDescription;
    // �޽� ������Ʈ
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UStaticMeshComponent* MeshComponent;
    // ��ȣ�ۿ� ���� (SphereComponent)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USphereComponent* InteractionSphere;
    // �������̽� ����
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
    void Interact(AActor* Interactor);
    virtual void Interact_Implementation(AActor* Interactor) override;
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
    bool CanInteract(AActor* Interactor);
    virtual bool CanInteract_Implementation(AActor* Interactor) override;
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
    FString GetInteractionText();
    virtual FString GetInteractionText_Implementation() override;

    // ��ȣ�ۿ� ���� ���� (�߰��� �κ�)
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
    EInteractionType GetInteractionType();
    virtual EInteractionType GetInteractionType_Implementation() override;

    // ���� ���� ����
    UFUNCTION(BlueprintCallable, Category = "Pickup")
    bool PickupItem(AActor* Interactor);
    // ���� �̺�Ʈ (�������Ʈ���� Ȯ�� ����)
    UFUNCTION(BlueprintImplementableEvent, Category = "Pickup")
    void OnPickedUp(AActor* Interactor);
    // ���� ȿ����
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup")
    class USoundBase* PickupSound;
    // ������ �Ⱦ� ���� �� ȣ���
    UFUNCTION(BlueprintCallable, Category = "Pickup")
    void OnPickupSuccess(AActor* Interactor);
    // ����׿� �ð�ȭ
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    bool bDrawDebug = true;
};