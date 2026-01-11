#include "Interaction/DialogueTrigger.h"
#include "Components/BoxComponent.h"
#include "Character/HamoniaCharacter.h"
#include "Core/DialogueManagerComponent.h"

ADialogueTrigger::ADialogueTrigger()
{
    PrimaryActorTick.bCanEverTick = false;

    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
    TriggerBox->SetupAttachment(RootComponent);
    TriggerBox->SetBoxExtent(FVector(100.0f, 100.0f, 100.0f));
    TriggerBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    TriggerBox->SetCollisionObjectType(ECC_WorldStatic);
    TriggerBox->SetCollisionResponseToAllChannels(ECR_Ignore);
    TriggerBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
}

void ADialogueTrigger::BeginPlay()
{
    Super::BeginPlay();
    TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &ADialogueTrigger::OnTriggerBeginOverlap);
}

void ADialogueTrigger::OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
    UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (!bIsActive)
    {
        return;
    }

    if (bOneTimeOnly && bHasTriggered)
    {
        return;
    }

    AHamoniaCharacter* Player = Cast<AHamoniaCharacter>(OtherActor);
    if (!Player)
    {
        return;
    }

    UDialogueManagerComponent* DM = Player->GetDialogueManagerComponent();
    if (!DM)
    {
        return;
    }

    FString SavedDialogueID = DM->GetLastDialogueID();
    FString DialogueToPlay = SavedDialogueID.IsEmpty() ? DialogueID : SavedDialogueID;

    bool bSuccess = DM->StartDialogue(DialogueToPlay);

    if (bSuccess)
    {
        if (!SavedDialogueID.IsEmpty())
        {
            DM->SaveLastDialogueID("");
        }

        bHasTriggered = true;

        if (bOneTimeOnly)
        {
            TriggerBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        }
    }
}

void ADialogueTrigger::ResetTrigger()
{
    bHasTriggered = false;

    if (TriggerBox)
    {
        TriggerBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    }
}