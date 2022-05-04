// Fill out your copyright notice in the Description page of Project Settings.


#include "Resources/Resources.h"

#include "Core/NpcNaturalDialogComponent.h"
#include "Core/PlayerNaturalDialogComponent.h"
#include "GameFramework/PlayerController.h"

int FDictionaryData::NumOfWords = 1;


FNaturalDialogRow::FNaturalDialogRow(const FText& InText)
{
	Answer.Add(InText);
}

FNaturalDialogRow::FNaturalDialogRow(const FNaturalDialogRow_Base& Other)
{
	Answer = Other.Answer;
}

FDialogTaskPlayerData::FDialogTaskPlayerData()
	: PlayerPawn(nullptr), NaturalDialogSystemComponent(nullptr), PlayerController(nullptr), PlayerState(nullptr) {}

FDialogTaskPlayerData::FDialogTaskPlayerData(APlayerController* InPlayerController)
	: PlayerPawn(nullptr), NaturalDialogSystemComponent(nullptr), PlayerController(InPlayerController), PlayerState(nullptr)
{
	if (InPlayerController)
	{
		PlayerPawn = InPlayerController->GetPawn();
		PlayerState = InPlayerController->PlayerState;
	}

	if (PlayerPawn)
	{
		NaturalDialogSystemComponent = PlayerPawn->FindComponentByClass<UPlayerNaturalDialogComponent>();
	}
}

bool FDialogTaskPlayerData::IsValid() const
{
	return PlayerPawn && PlayerController && PlayerState;
}

FDialogTaskNPCData::FDialogTaskNPCData()
	: OwningActor(nullptr), OwningController(nullptr), NaturalDialogSystemComponent(nullptr) {}

FDialogTaskNPCData::FDialogTaskNPCData(AActor* InOwningActor)
	: OwningActor(InOwningActor)
{
	if (InOwningActor)
	{
		OwningController = InOwningActor->GetInstigatorController();
		NaturalDialogSystemComponent = InOwningActor->FindComponentByClass<UNpcNaturalDialogComponent>();
	}
}

bool FDialogTaskNPCData::IsValid() const
{
	return OwningActor && NaturalDialogSystemComponent;
}
