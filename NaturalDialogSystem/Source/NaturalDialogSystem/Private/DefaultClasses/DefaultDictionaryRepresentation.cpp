// Created by Michal Chamula. All rights reserved.


#include "DefaultClasses/DefaultDictionaryRepresentation.h"

#include "Resources/NaturalDialogSystemLibrary.h"
#include "Resources/Resources.h"

void UDefaultDictionaryRepresentation::InitializeDictionary()
{
	Super::InitializeDictionary();

	// In every bucket are words of same len, in first value are words with len == 1, at second with len == 2, ...
	DictionaryData.SetNum(10);
}

void UDefaultDictionaryRepresentation::RegisterWord(const FString& Word, const UDataTable* FromDataTable)
{
	const int32 WordLen = Word.Len();
	if (WordLen > 0)
	{
		const int32 FixedLen = WordLen - 1;

		if (DictionaryData.Num() <= FixedLen)
		{
			// If bucket with (len == WordLen) doesn't exist, we have to extend the array
			DictionaryData.AddDefaulted(WordLen - DictionaryData.Num());
		}

		FDictionaryData* Data = DictionaryData[FixedLen].Find(Word);
		if (Data)
		{
			// We found data in table, now we add new occurence into these data
			Data->AddOccurence(FromDataTable);
		}
		else
		{
			// Or we just create new data in dictionary
			DictionaryData[FixedLen].Add(UNaturalDialogSystemLibrary::NormalizeTerm(Word), FDictionaryData(FromDataTable));
		}
	}
}

TArray<FString> UDefaultDictionaryRepresentation::GetListOfWords() const
{
	TArray<FString> Result;

	for (const TMap<FString, FDictionaryData>& Map : DictionaryData)
	{
		TArray<FString> Temp;
		Map.GetKeys(Temp);
		Result.Append(Temp);
	}

	return Result;
}

TArray<FString> UDefaultDictionaryRepresentation::GetListOfWordsOfLen(const int32 WordLen) const
{
	TArray<FString> OutKeys;
	const int32 FixedWordLen = WordLen - 1;

	if (DictionaryData.IsValidIndex(FixedWordLen))
	{
		DictionaryData[FixedWordLen].GetKeys(OutKeys);
	}

	return OutKeys;
}


const FDictionaryData* UDefaultDictionaryRepresentation::GetWordData(const FString& Word) const
{
	const int32 FixedWordLen = Word.Len() - 1;

	if (DictionaryData.IsValidIndex(FixedWordLen))
	{
		return DictionaryData[FixedWordLen].Find(Word);
	}

	return nullptr;
}
