// Created by Michal Chamula. All rights reserved.


#include "DefaultClasses/DefaultDictionaryPickerFunction.h"

#include "DefaultClasses/LevenshteinDistanceFunction.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Resources/DictionaryRepresentation.h"


DEFINE_LOG_CATEGORY(Log_DefaultDictPickerFunction);

void UDefaultDictionaryPickerFunction::InitializeWordPicker()
{
	Super::InitializeWordPicker();

	CachedDictionarySubsystem = Cast<UDictionarySubsystem>(GetOuter());
}

FString UDefaultDictionaryPickerFunction::PickWordFromDictionary(const FString& Input) const
{
	FString Result;
	const int32 InputLen = Input.Len();
	const UDictionarySubsystem* DictionarySubsystem = nullptr;

	// Cache dictionary subsystem which we use
	if (CachedDictionarySubsystem.IsValid())
	{
		DictionarySubsystem = CachedDictionarySubsystem.Get();
	}
	else
	{
		UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
		if (GameInstance)
		{
			DictionarySubsystem = GameInstance->GetSubsystem<UDictionarySubsystem>();
		}
	}

	// If is dict subsystem invalid, we don't allow words picking
	if (!DictionarySubsystem)
	{
		UE_LOG(Log_DefaultDictPickerFunction, Error, TEXT("Failed to retrieve dictionary subsystem"));
		return Result;
	}

	// Retrieve word from dict data
	if (InputLen > 0)
	{
		const UDictionaryRepresentation* DictionaryRepresentation = DictionarySubsystem->GetDictionary();
		if (ensure(DictionaryRepresentation && StringMetricDistanceFunctionInstance))
		{
			//const FString NormalizedInput = UNaturalDialogSystemLibrary::NormalizeTerm(Input);
			int32 MinErrorC = MAX_int32;
			FString WordWithMinEvaluation;
			for (int32 SubstituteLen = 0; SubstituteLen < MAX_LEN_DIFF; SubstituteLen++)
			{
				// Find all words of len
				TArray<FString> Words = DictionaryRepresentation->GetListOfWordsOfLen(InputLen + SubstituteLen);
				if (SubstituteLen > 0)
				{
					Words.Append(DictionaryRepresentation->GetListOfWordsOfLen(InputLen - SubstituteLen));
				}

				TArray<int32> WordsEvaluation;
				WordsEvaluation.Reserve(Words.Num());

				for (const FString& Word : Words)
				{
					// const FString NormalizedWord = UNaturalDialogSystemLibrary::NormalizeTerm(Word);
					const int32 Evaluation = StringMetricDistanceFunctionInstance->GetStringDistance(Word, Input);
					if (Evaluation == 0)
					{
						return Word; // <==== End here, found correct word
					}

					WordsEvaluation.Add(Evaluation);
				}

				// Save found word data
				int32 Index;
				int32 NewMinErrorC;
				UKismetMathLibrary::MinOfIntArray(WordsEvaluation, Index, NewMinErrorC);

				if (Index != INDEX_NONE && NewMinErrorC < MinErrorC && NewMinErrorC < Words[Index].Len() - 1)
				{
					MinErrorC = NewMinErrorC;
					WordWithMinEvaluation = Words[Index];
				}

				// Check stop criteria if word len is too different from input, we use this stop criteria 
				if (MinErrorC <= SubstituteLen)
				{
					break;
				}
			}

			return WordWithMinEvaluation;
		}
	}
	else
	{
		UE_LOG(Log_DefaultDictPickerFunction, Warning, TEXT("Empty input text"));
	}

	return Result;
}
