// Created by Michal Chamula. All rights reserved.

#pragma once

#include "CoreMinimal.h"

#include "PlayerNaturalDialogComponent.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "DictionarySubsystem.generated.h"

class UDictionaryWordPickerFunction;
class UDictionaryRepresentation;
class UKeywordPickerFunction;


DECLARE_LOG_CATEGORY_EXTERN(LogDictSubsystem, Log, All);

/**
 * 
 */
UCLASS()
class NATURALDIALOGSYSTEM_API UDictionarySubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	/**
	 * Initialize all dictionary settings
	 * Spawn friendly classes instances
	 * Checks if all developer settings are set
	 */
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/**
	 * Checks if all developers settings are set correctly in project settings
	 * If no, shows message log for editor
	 */
	virtual void Deinitialize() override;

	TArray<FString> GenerateKeywords(const UPlayerNaturalDialogComponent* DialogComponent, const FString& InputText);

	/**
	 * Returns dictionary word data object
	 * @return - Custom dictionary word object representation
	 */
	FORCEINLINE const UDictionaryRepresentation* GetDictionary() const { return DictionaryData; }

	FORCEINLINE const UDictionaryWordPickerFunction* GetWordPickerFunction() const { return DictionaryWordPickerFunctionInstance; }
	
private:
	/** Helper function for dictionary object construction */
	void ConstructDictObject(const TSubclassOf<UDictionaryRepresentation> DictClass);

	/** Helper function for keyword picker object construction */
	void ConstructKeywordPickerObject(const TSubclassOf<UKeywordPickerFunction> KeywordPickerClass);

	/** Helper function for dictionary word picker object construction */
	void ConstructDictionaryWordPickerFunctionObject(const TSubclassOf<UDictionaryWordPickerFunction> StringFunctionClass);


	/**
	 * Register all words from data table into dict subsystem
	 * These words are later used for find game key words
	 * @param InTable - table which is parsed
	 */
	void RegisterWordsFromTable(const UDataTable* InTable);

private:
	/** Cached all dialog tables in game */
	UPROPERTY()
	TSet<UDataTable*> GameNaturalDialogTables;

	/** Strong ref for dictionary data, parsed from game data tables */
	UPROPERTY()
	UDictionaryRepresentation* DictionaryData;

	/** Strong ref to word picker singleton function  */
	UPROPERTY()
	UDictionaryWordPickerFunction* DictionaryWordPickerFunctionInstance;

	/** Strong ref to keyword picker singleton function  */
	UPROPERTY()
	UKeywordPickerFunction* KeywordPickerFunctionInstance;
};
