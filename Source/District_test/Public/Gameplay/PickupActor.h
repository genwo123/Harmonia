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

    // 메시 컴포넌트 (다시 루트로)
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
    class UStaticMeshComponent* MeshComponent;

    // 상호작용 영역
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
    class USphereComponent* InteractionSphere;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup")
    TSubclassOf<UItem> ItemClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup")
    FString CustomItemName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup")
    FString CustomItemDescription;

    // 인터페이스 구현
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
    void Interact(AActor* Interactor);
    virtual void Interact_Implementation(AActor* Interactor) override;

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
    bool CanInteract(AActor* Interactor);
    virtual bool CanInteract_Implementation(AActor* Interactor) override;

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
    FString GetInteractionText();
    virtual FString GetInteractionText_Implementation() override;

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
    EInteractionType GetInteractionType();
    virtual EInteractionType GetInteractionType_Implementation() override;

    UFUNCTION(BlueprintCallable, Category = "Pickup")
    bool PickupItem(AActor* Interactor);

    UFUNCTION(BlueprintImplementableEvent, Category = "Pickup")
    void OnPickedUp(AActor* Interactor);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup")
    class USoundBase* PickupSound;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* SceneComponent;



    // 메시 회전 설정
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance", meta = (ExposeOnSpawn = true))
    FRotator MeshRotation = FRotator::ZeroRotator;

    // 회전 관련 함수들
    UFUNCTION(BlueprintCallable, Category = "Appearance")
    void ApplyMeshRotation();

    UFUNCTION(BlueprintCallable, Category = "Appearance")
    void SetMeshYawRotation(float Yaw);

    virtual void OnConstruction(const FTransform& Transform) override;

    UFUNCTION(BlueprintCallable, Category = "Pickup")
    void OnPickupSuccess(AActor* Interactor);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    bool bDrawDebug = true;

#if WITH_EDITOR
    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};