#include "Gameplay/HintFrame.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"

AHintFrame::AHintFrame()
{
    PrimaryActorTick.bCanEverTick = false;

    FrameMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FrameMesh"));
    RootComponent = FrameMesh;

    FrontImagePoint = CreateDefaultSubobject<USceneComponent>(TEXT("FrontImagePoint"));
    FrontImagePoint->SetupAttachment(RootComponent);
    FrontImagePoint->SetRelativeLocation(FVector(1.0f, 0.0f, 0.0f)); // 액자 앞면

    BackTextRoot = CreateDefaultSubobject<USceneComponent>(TEXT("BackTextRoot"));
    BackTextRoot->SetupAttachment(RootComponent);
    BackTextRoot->SetRelativeLocation(FVector(-1.0f, 0.0f, 0.0f)); // 액자 뒷면
    BackTextRoot->SetRelativeRotation(FRotator(0.0f, 180.0f, 0.0f));
}

void AHintFrame::BeginPlay()
{
    Super::BeginPlay();
}

void AHintFrame::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);
    UpdateBackTextsFromData();
}

void AHintFrame::Interact_Implementation(AActor* Interactor)
{
    OnFrameInteracted(Interactor);
}

bool AHintFrame::CanInteract_Implementation(AActor* Interactor)
{
    if (!Interactor) return false;

    float Distance = FVector::Dist(GetActorLocation(), Interactor->GetActorLocation());
    return Distance <= InteractionDistance;
}

FString AHintFrame::GetInteractionText_Implementation()
{
    return TEXT("FreamText");
}

EInteractionType AHintFrame::GetInteractionType_Implementation()
{
    return EInteractionType::Read;
}

FHintFrameData AHintFrame::GetHintData()
{
    if (HintDataTable && !HintID.IsEmpty())
    {
        FHintFrameData* Data = HintDataTable->FindRow<FHintFrameData>(FName(*HintID), TEXT(""));
        if (Data)
        {
            return *Data;
        }
    }
    return FHintFrameData();
}

void AHintFrame::UpdateBackTextsFromData()
{
    // 기존 텍스트 컴포넌트 제거
    for (UTextRenderComponent* Comp : BackTextComponents)
    {
        if (Comp && IsValid(Comp))
        {
            Comp->DestroyComponent();
        }
    }
    BackTextComponents.Empty();

    // 데이터 테이블에서 정보 가져오기
    FHintFrameData Data = GetHintData();

    // 뒷면 텍스트 생성
    for (int32 i = 0; i < Data.BackTexts.Num(); i++)
    {
        const FBackTextPlacement& Placement = Data.BackTexts[i];

        FString CompName = FString::Printf(TEXT("BackText_%d"), i);
        UTextRenderComponent* TextComp = NewObject<UTextRenderComponent>(this, *CompName);

        if (TextComp)
        {
            TextComp->SetupAttachment(BackTextRoot);
            TextComp->RegisterComponent();

            TextComp->SetText(FText::FromString(Placement.Text));
            TextComp->SetRelativeLocation(Placement.RelativeLocation);
            TextComp->SetRelativeRotation(Placement.RelativeRotation);
            TextComp->SetWorldSize(Placement.FontSize);
            TextComp->SetTextRenderColor(FColor::Black);
            TextComp->SetHorizontalAlignment(EHTA_Center);
            TextComp->SetVerticalAlignment(EVRTA_TextCenter);

            BackTextComponents.Add(TextComp);
        }
    }
}