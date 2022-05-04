// Created by Michal Chamula. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Resources/DictionaryRepresentation.h"
#include "DefaultDictionaryRepresentation.generated.h"

struct FDictionaryData;


/**
 * 
 */
UCLASS()
class NATURALDIALOGSYSTEM_API UDefaultDictionaryRepresentation : public UDictionaryRepresentation
{
	GENERATED_BODY()

public:
	virtual void InitializeDictionary() override;
	virtual void RegisterWord(const FString& Word, const UDataTable* FromDataTable) override;
	
	virtual TArray<FString> GetListOfWords() const override;
	virtual TArray<FString> GetListOfWordsOfLen(const int32 WordLen) const override;
	virtual const FDictionaryData* GetWordData(const FString& Word) const override;
	
protected:
	TArray<TMap<FString, FDictionaryData>> DictionaryData;
};
