// Created by Michal Chamula. All rights reserved.


#include "DefaultClasses/DefaultReplyHelperFunction.h"

#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetStringLibrary.h"
#include "Core/DictionarySubsystem.h"
#include "FunctionalClasses/DictionaryWordPickerFunction.h"
#include "Resources/NaturalDialogSystemLibrary.h"

void UDefaultReplyHelperFunction::InitializeDialogReplyHelper()
{
	Super::InitializeDialogReplyHelper();

	CachedDictionarySubsystem = UGameplayStatics::GetGameInstance(this)->GetSubsystem<UDictionarySubsystem>();
	if (CachedDictionarySubsystem)
	{
		PickerFunction = CachedDictionarySubsystem->GetWordPickerFunction();
	}
}

bool UDefaultReplyHelperFunction::FindAskOptions(const FText& Input, const UNpcNaturalDialogComponent* NpcNaturalDialogComponent, TSet<FString>& OutOptions)
{
	const FString& InputString = Input.ToString();
	OutOptions.Empty();

	if (UPlayerNaturalDialogComponent* PlayerNaturalDialogComponent = Cast<UPlayerNaturalDialogComponent>(GetOuter()))
	{
		PlayerNaturalDialogComponent->RegisterInitialTables(NpcNaturalDialogComponent);
	}

	if (!InputString.IsEmpty() && PickerFunction)
	{
		const TArray<FString> Sentences = UNaturalDialogSystemLibrary::SplitToSentences(InputString);
		if (Sentences.Num() > 0)
		{
			const FString& LastSentence = Sentences.Last();
			const FString OutputBuilder = InputString.Left(InputString.Len() - LastSentence.Len());

			// Last sentence of input split into words
			// const TArray<FString> SentenceWords = UKismetStringLibrary::ParseIntoArray(LastSentence, TEXT(" "), true);
			const TArray<FString> SentenceWords = UNaturalDialogSystemLibrary::SplitSentenceIntoNormalizedTerms(LastSentence);

			if (SentenceWords.Num() > 0)
			{
				// Delete words from last input which are not used anymore
				LastInputs.SetNum(SentenceWords.Num(), false);

				bool bChainBroken = false;
				for (int32 WordId = 0; WordId < SentenceWords.Num(); WordId++)
				{
					// Fix the word from player input
					// const FString& NormalizedWord = UNaturalDialogSystemLibrary::NormalizeTerm(SentenceWords[WordId]);
					const FString& NormalizedWord = SentenceWords[WordId];
					const FString DictionaryWord = PickerFunction->PickWordFromDictionary(NormalizedWord);

					// #todo ...check this
					const FString& FixedWord = DictionaryWord.IsEmpty() ? SentenceWords[WordId] : DictionaryWord;

					if (bChainBroken)
					{
						// Chain is broken, we have to cache new word data from last chained word
						LastInputs[WordId] = GetFilteredWordData(NormalizedWord, NpcNaturalDialogComponent, WordId);
					}
					else
					{
						const FString& CachedWord = LastInputs[WordId].GetWord();
						if (!LastInputs[WordId].IsValid() || !CachedWord.Equals(NormalizedWord))
						{
							// Something is wrong, break the chain of cached words
							bChainBroken = true;
							LastInputs[WordId] = GetFilteredWordData(NormalizedWord, NpcNaturalDialogComponent, WordId);
						}
					}
				}

				const FOptionData& FoundOptions = LastInputs.Last();

				for (const FString& InOption : FoundOptions.GetOutputOptions())
				{
					OutOptions.Add(OutputBuilder + ' ' + InOption);
				}

				if (OutOptions.Num() > 0)
				{
					return true;
				}
			}
		}
	}

	return false;
}

bool UDefaultReplyHelperFunction::FindBestOption(const FText& Input, const UNpcNaturalDialogComponent* NpcNaturalDialogComponent, FText& OutOption)
{
	// ...
	return false;
}

FOptionData UDefaultReplyHelperFunction::GetFilteredWordData(const FString& Word, const UNpcNaturalDialogComponent* NpcNaturalDialogComponent, const int32 InputWordQueue) const
{
	FOptionData Result;
	const int32 PreviousWordQIndex = InputWordQueue - 1;

	if (PreviousWordQIndex == INDEX_NONE)
	{
		// Retrieve data from NPC component
		if (const UPlayerNaturalDialogComponent* PlayerNaturalDialogComponent = Cast<UPlayerNaturalDialogComponent>(GetOuter()))
		{
			const TSet<const UDataTable*> DataTables = PlayerNaturalDialogComponent->GetDialogTables_Const(NpcNaturalDialogComponent);
			for (const UDataTable* DT : DataTables)
			{
				DT->ForeachRow<FNaturalDialogRow>(TEXT(""), [&Word, &Result, DT](const FName& Key, const FNaturalDialogRow& Row)
				{
					// ~ Begin extracting of first word from ask sentence
					FString FirstWord;
					FirstWord.Reserve(15);

					// We only need first character of ask sentence
					for (const TCHAR Char : Row.Ask.ToString())
					{
						if (Char == TEXT(' '))
						{
							break;
						}
						FirstWord += Char;
					}

					FirstWord = UNaturalDialogSystemLibrary::NormalizeTerm(FirstWord); // Normalize word
					// ~ End extracting of first word from ask sentence

					// First word is matched, we can add it as an output option
					const int32 NumOfChars = Word.Len();
					for (int32 CharIdx = 0; CharIdx < NumOfChars; CharIdx++)
					{
						// Some character doesn't matched, break the check
						if (FirstWord.IsValidIndex(CharIdx) && Word[CharIdx] != FirstWord[CharIdx])
						{
							break;
						}

						// All characters matched, add as new possible option
						if (CharIdx == NumOfChars - 1)
						{
							Result.Add(DT, Key);
						}
					}
				});
			}
		}
	}
	else
	{
		const FOptionData& PreviousWordData = LastInputs[PreviousWordQIndex];

		// Check all previous data 
		for (const TPair<const UDataTable*, TSet<FName>>& TableOptions : PreviousWordData.GetOptions())
		{
			for (const FName Option : TableOptions.Value)
			{
				const FNaturalDialogRow* RowData = TableOptions.Key->FindRow<FNaturalDialogRow>(Option, TEXT(""));
				if (RowData)
				{
					const TArray<FString> AskWords = UKismetStringLibrary::ParseIntoArray(RowData->Ask.ToString(), TEXT(" "), true);
					if (AskWords.IsValidIndex(InputWordQueue))
					{
						const FString& CurrentlyWord = AskWords[InputWordQueue];
						const int32 NumOfChars = Word.Len();

						for (int32 CharIdx = 0; CharIdx < NumOfChars; CharIdx++)
						{
							// Some character doesn't matched, break the check
							// todo crash
							if (Word[CharIdx] != CurrentlyWord[CharIdx])
							{
								break;
							}

							// All characters matched, add as new possible option
							if (CharIdx == NumOfChars - 1)
							{
								Result.Add(TableOptions.Key, Option);
							}
						}
					}
				}
			}
		}
	}

	return Result;
}
