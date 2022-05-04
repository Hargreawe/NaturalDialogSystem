// Created by Michal Chamula. All rights reserved.


#include "DefaultClasses/Tf_idf_PickerFunction.h"
#include "Core/DictionarySubsystem.h"
#include "Kismet/KismetMathLibrary.h"
#include "Resources/DictionaryRepresentation.h"
#include "Resources/NaturalDialogSystemLibrary.h"


DEFINE_LOG_CATEGORY(Log_Tf_Idf_PickerFunction);

void UTf_idf_PickerFunction::InitializeKeywordPicker()
{
	Super::InitializeKeywordPicker();

	if (const UDictionarySubsystem* Subsystem = Cast<UDictionarySubsystem>(GetOuter()))
	{
		DictionaryData = Subsystem->GetDictionary();
	}

	TablesCount = UNaturalDialogSystemLibrary::GetListOfDialogDataTables().Num();
}

TArray<FString> UTf_idf_PickerFunction::PickKeyWords(const UPlayerNaturalDialogComponent* DialogComponent, const TArray<FString>& Input)
{
	TArray<FString> Result;
	TArray<FString> CopiedInput = TSet<FString>(Input).Array(); // Remove duplicates

	if (DictionaryData.IsValid())
	{
		if (Input.IsValidIndex(0))
		{
			TArray<float> Tf_Idf_Value;
			Tf_Idf_Value.Reserve(Input.Num());

			// Compute Tf-Idf values for input words
			for (const FString& Word : CopiedInput)
			{
				const FDictionaryData* DictData = DictionaryData->GetWordData(Word);
				if (DictData)
				{
					int32 TermOccurence = 0;
					for (const UDataTable* OccurenceInTable : DictData->GetTables())
					{
						TermOccurence += DictData->GetOccurenceCount(OccurenceInTable);
					}

					const float Tf_Value = static_cast<float>(TermOccurence) / static_cast<float>(FDictionaryData::GetNumOfWords());
					const float Idf_Value = FMath::LogX(10, (static_cast<float>(TablesCount) / static_cast<float>(DictData->GetTableOccurenceCount())));
					const float TfIdf_Value = Tf_Value * Idf_Value; // The result Tf_Idf value

					Tf_Idf_Value.Add(TfIdf_Value);
					UE_LOG(Log_Tf_Idf_PickerFunction, Log, TEXT("Tf_Idf value for word is (%s = %f)"), *Word, TfIdf_Value);
				}
				else
				{
					UE_LOG(Log_Tf_Idf_PickerFunction, Error, TEXT("Dict data for word (%s) not found"), *Word);
				}
			}

			// Find the max value of array
			int32 IndexOfMaxValue = -1;
			float ArrMaxValue = 0.f;
			UKismetMathLibrary::MaxOfFloatArray(Tf_Idf_Value, IndexOfMaxValue, ArrMaxValue);

			// In some cases, we use input, because player can ask "Who you are", and it can be found in reply
			// Invalid input is, when the sentence doesnt contains at least MIN_KEYWORDS_COUNT words, now it is 3, so we need the sentence with at least 3 words
			if (Input.Num() <= MIN_KEYWORDS_COUNT)
			{
				for(const FString& InputElement : Input)
				{
					Result.Add(UNaturalDialogSystemLibrary::NormalizeTerm(InputElement));
				}
			}
			else if (ArrMaxValue > 0 && Input.Num() > MIN_KEYWORDS_COUNT)
			{
				if (Tf_Idf_Value.IsValidIndex(IndexOfMaxValue))
				{
					// Defines how many elements takes from input array, e.g.> for value 0.5 takes half of array
					const float Precision = UKismetMathLibrary::MapRangeClamped(Tf_Idf_Value.Num(), 4, 10, 1.f, 0.5f);

					// Add the first occurence
					Result.Add(UNaturalDialogSystemLibrary::NormalizeTerm(CopiedInput[IndexOfMaxValue]));
					UE_LOG(Log_Tf_Idf_PickerFunction, Log, TEXT("Selecting (%s) as first keyword"), *CopiedInput[IndexOfMaxValue]);
					Tf_Idf_Value.RemoveAt(IndexOfMaxValue);
					CopiedInput.RemoveAt(IndexOfMaxValue);

					// Select keywords using computed value
					const int32 NumOfRepeats = FMath::RoundToInt((Tf_Idf_Value.Num() * Precision) - 1);
					for (int32 i = 0; i < NumOfRepeats; i++)
					{
						// Add other occurence, but only if max_value is greater than 0, or we still have not MIN_KEYWORDS_COUNT elements in array
						UKismetMathLibrary::MaxOfFloatArray(Tf_Idf_Value, IndexOfMaxValue, ArrMaxValue);
						if (Input.IsValidIndex(IndexOfMaxValue) && (i <= MIN_KEYWORDS_COUNT || ArrMaxValue > 0))
						{
							const FString& InWord = CopiedInput[IndexOfMaxValue];
							Result.Add(UNaturalDialogSystemLibrary::NormalizeTerm(InWord));
							UE_LOG(Log_Tf_Idf_PickerFunction, Log, TEXT("Next keyword is (%s)"), *InWord);
							CopiedInput.RemoveAt(IndexOfMaxValue);
							Tf_Idf_Value.RemoveAt(IndexOfMaxValue);
						}
					}
				}
			}
		}
	}
	else
	{
		UE_LOG(Log_Tf_Idf_PickerFunction, Error, TEXT("No dictionary data found"));
	}

	return Result;
}
