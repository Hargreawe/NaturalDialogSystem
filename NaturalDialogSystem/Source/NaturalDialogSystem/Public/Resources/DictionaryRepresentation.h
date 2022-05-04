// Created by Michal Chamula. All rights reserved.

#pragma once

#include "CoreMinimal.h"

#include "Resources.h"
#include "UObject/Object.h"
#include "DictionaryRepresentation.generated.h"

/**
 * Custom representation of data in dictionary
 * Any representation is OK for any solution
 * E.g. you can split words in buckets, where one bucket is for words of length == 1, next bucket is for words of length == 2, ...
 * By this representation we can optimize selection of words to check in string metrics, or when we trying to find keywords, etc.
 * You can create your representation for custom optimization and select it in project settings
 */
UCLASS(Abstract)
class NATURALDIALOGSYSTEM_API UDictionaryRepresentation : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * Is called after object creation
	 * Initialize dictionary properties before words registration
	 */
	virtual void InitializeDictionary() {};

	/**
	 * Function called from UDictionarySubsystem
	 * There are processing all words from game content
	 * @param Word - registered word from data tables or your custom word for the dictionary
	 * @param FromDataTable - Data table, where is word used
	 */
	virtual void RegisterWord(const FString& Word, const UDataTable* FromDataTable)
	{
		check(0 && "Set a valid dictionary representation");
	}

	/** Return list of all words in dictionary */
	virtual TArray<FString> GetListOfWords() const
	{
		check(0 && "Set a valid dictionary representation");
		return {};
	}

	virtual TArray<FString> GetListOfWordsOfLen(const int32 WordLen) const
	{
		check(0 && "Set a valid dictionary representation");
		return {};
	}

	/** Return word data */
	virtual const FDictionaryData* GetWordData(const FString& Word) const
	{
		check(0 && "Set a valid dictionary representation");
		return nullptr;
	}

};
