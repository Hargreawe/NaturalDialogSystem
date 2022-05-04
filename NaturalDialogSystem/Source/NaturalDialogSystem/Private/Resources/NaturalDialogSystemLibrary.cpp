// Created by Michal Chamula. All rights reserved.


#include "Resources/NaturalDialogSystemLibrary.h"
#include "Resources/Resources.h"
#include "NaturalDialogSystem/External/utf8proc.h"


TSet<UDataTable*> UNaturalDialogSystemLibrary::GetListOfDialogDataTables()
{
	// Finds all tables in content
	const TArray<UDataTable*> FoundTables = GetListOfContentObjects_Recursive<UDataTable>();

	TSet<UDataTable*> Result;
	Result.Reserve(FoundTables.Num());

	for (UDataTable* Table : FoundTables)
	{
		// We sort out tables with correct natural dialog struct
		if (Table && Table->GetRowStruct()->IsChildOf(FNaturalDialogRow::StaticStruct()))
		{
			Result.Add(Table);
		}
	}

	return Result;
}

TArray<FString> UNaturalDialogSystemLibrary::SplitSentenceIntoNormalizedTerms(const FString& Input)
{
	TArray<FString> Result;
	Result.Reserve(20);

	FString CachedWord;
	CachedWord.Reset(20);

	for (const CHAR Character : Input)
	{
		const CHAR Normalized = NormalizeCharacter(Character);
		if (IsSpaceChar(Normalized))
		{
			if (!CachedWord.IsEmpty())
			{
				Result.Add(CachedWord);
				CachedWord.Reset(20);
			}
		}
		else if (Normalized != NULL_CHARACTER)
		{
			CachedWord += Normalized;
		}
	}

	if (!CachedWord.IsEmpty())
	{
		Result.Add(CachedWord);
	}

	return Result;
}

FString UNaturalDialogSystemLibrary::NormalizeTerm(const FString& Input)
{
	FString Result;
	Result.Reserve(Input.Len());

	for (const TCHAR Character : Input)
	{
		const TCHAR NormalizedChar = NormalizeCharacter(Character);
		if (NormalizedChar != SPACE_CHARACTER)
		{
			Result += NormalizedChar;
		}
	}

	return Result;
}

TArray<FString> UNaturalDialogSystemLibrary::SplitToSentences(const FString& Input)
{
	TArray<FString> Result;

	FWideStringBuilderBase StringBuilderBase;
	
	FString Sentence;

	for (const TCHAR Character : Input)
	{
		if (IsSentenceSeparator(Character))
		{
			if (!Sentence.IsEmpty())
			{
				Sentence += Character;
				Result.Add(Sentence);
				Sentence.Reset(18);
			}
		}
		else
		{
			Sentence += Character;
		}
	}

	if (!Sentence.IsEmpty())
	{
		Result.Add(Sentence);
	}

	return Result;
}

FString UNaturalDialogSystemLibrary::NormalizeWord(const FString& Input)
{
	return NormalizeTerm(Input);
}

CHAR UNaturalDialogSystemLibrary::NormalizeCharacter(CHAR Character)
{
	if (IsSpaceChar(Character) || IsTagCharacter(Character))
	{
		return Character;
	}

	if (IsSentenceSeparator(Character))
	{
		return SPACE_CHARACTER;
	}

	if (IsSpecialChar(Character))
	{
		return NULL_CHARACTER;
	}

	return utf8proc_tolower(Character);
}
