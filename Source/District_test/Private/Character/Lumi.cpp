// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Lumi.h"

// Sets default values
ALumi::ALumi()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ALumi::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ALumi::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ALumi::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

