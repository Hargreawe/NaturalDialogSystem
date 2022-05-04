// Created by Michal Chamula. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "NaturalDialogTask.h"
#include "Components/ActorComponent.h"
#include "FunctionalClasses/DialogReplyFunction.h"
#include "NpcNaturalDialogComponent.generated.h"


DECLARE_LOG_CATEGORY_EXTERN(LogNpcNaturalDialogComponent, Log, All);

/**
 * If player need to communicate with NPC using natural dialog system, the NPC needs this component and is used as dialog receiver
 * Component holds data tables, metrics, and other useful data which are used for player answers
 * These asks from player are send into UDictionarySubsystem to process data from vocabulary and receive array of keywords we are searching for in data tables
 * After we receive keywords, we combine them with metrics and other properties to get the best answer for player
 */
UCLASS(Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class NATURALDIALOGSYSTEM_API UNpcNaturalDialogComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UNpcNaturalDialogComponent();

protected:
	/**
	 * Set of dialog data, we are using in dialogs
	 * You can register new dialog tables for NPC using function RegisterNewDialogData()
	 * @warning - the more elements it contains, the complexity raise
	 * meta=(RequiredAssetDataTags = "RowStructure=NaturalDialogRow")
	 */
	UPROPERTY(EditAnywhere, Category="Natural Dialog Component")
	TSet<UDataTable*> InitialDialogTables;

public:
	UFUNCTION(BlueprintCallable)
	TSet<UDataTable*> GetInitialDialogTables() const { return InitialDialogTables; }
	
	UFUNCTION(BlueprintCallable)
	bool HasInitialDialogTables() const { return InitialDialogTables.Num() > 0; }
};
