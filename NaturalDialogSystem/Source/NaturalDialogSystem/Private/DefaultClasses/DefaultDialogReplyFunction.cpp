// Created by Michal Chamula. All rights reserved.


#include "DefaultClasses/DefaultDialogReplyFunction.h"
#include "Core/PlayerNaturalDialogComponent.h"
#include "DefaultClasses/LevenshteinDistanceFunction.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Resources/DictionaryRepresentation.h"
#include "Resources/NaturalDialogSystemLibrary.h"

#define MIN_COMBINATION 2

DEFINE_LOG_CATEGORY(Log_DefaultDialogReplyFunction);

UDefaultDialogReplyFunction::UDefaultDialogReplyFunction()
{
	MetricInterval = 10.f;
}

void UDefaultDialogReplyFunction::InitializeDialogReplyPicker()
{
	Super::InitializeDialogReplyPicker();

	// Initialize all important references
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	if (GameInstance)
	{
		DictionarySubsystem = GameInstance->GetSubsystem<UDictionarySubsystem>();
		if (DictionarySubsystem.IsValid())
		{
			DictionaryRepresentation = DictionarySubsystem.Get()->GetDictionary();
		}
	}

	const TSubclassOf<UStringDistanceFunction> StringFuncClass = StringDistanceFunctionClass ? StringDistanceFunctionClass : ULevenshteinDistanceFunction::StaticClass();
	StringDistanceFunction = NewObject<UStringDistanceFunction>(this, StringFuncClass);
	StringDistanceFunction->GetStringDistance(TEXT("A"), TEXT("B")); // Override check

	OwnerComponent = Cast<UPlayerNaturalDialogComponent>(GetOuter());

	// Handle defaults table registration
	if (DefaultResponses)
	{
		HandleNewTableRegistration(DefaultResponses);
	}
	else
	{
		UE_LOG(Log_DefaultDialogReplyFunction, Warning, TEXT("Default responses table is invalid"));
	}

	if (ensure(OwnerComponent.IsValid()))
	{
		OwnerComponent.Get()->OnDataTableAdded.AddUniqueDynamic(this, &UDefaultDialogReplyFunction::HandleNewTableRegistration);
		OwnerComponent.Get()->OnDataTableRemoved.AddUniqueDynamic(this, &UDefaultDialogReplyFunction::HandleRegisteredTableRemoved);
	}

	if (GetWorld())
	{
		GetWorld()->GetTimerManager().SetTimer(MatrixWearinessHandler, this, &UDefaultDialogReplyFunction::HandleMatrixWeariness, MetricInterval, true);
	}
}

bool UDefaultDialogReplyFunction::GenerateReply(const FString& Sentence, const UNpcNaturalDialogComponent* NpcNaturalDialogComponent, FNaturalDialogResult& ResultAnswer)
{
	bool Result = false;
	ResultAnswer = FNaturalDialogResult();

	if (!DictionarySubsystem.IsValid())
	{
		const UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
		DictionarySubsystem = GameInstance->GetSubsystem<UDictionarySubsystem>();
		UE_LOG(Log_DefaultDialogReplyFunction, Warning, TEXT("Invalid dictionary subsystem reference, setting new one"));
	}

	if (!DictionaryRepresentation.IsValid())
	{
		if (DictionarySubsystem.IsValid())
		{
			DictionaryRepresentation = DictionarySubsystem.Get()->GetDictionary();
			UE_LOG(Log_DefaultDialogReplyFunction, Warning, TEXT("Invalid dictionary representation reference, setting new one"));
		}

		if (!DictionaryRepresentation.IsValid())
		{
			UE_LOG(Log_DefaultDialogReplyFunction, Error, TEXT("Invalid dictionary representation ptr value"));
			return Result;
		}
	}

	if (DictionarySubsystem.IsValid() && OwnerComponent.IsValid())
	{
		// Generated keywords of input sentence
		const TArray<FString> Keywords = DictionarySubsystem.Get()->GenerateKeywords(OwnerComponent.Get(), Sentence);

#if !UE_BUILD_SHIPPING

		FString StringDebugKeywords = "";
		StringDebugKeywords.Reserve(Keywords.Num() * 10);
		for (const FString& Keyword : Keywords)
		{
			StringDebugKeywords += Keyword + TEXT(" ");
		}
		UE_LOG(LogPlayerNaturalDialogComponent, Log, TEXT("Founds keywords are: %s"), *StringDebugKeywords);

#endif

		if (Keywords.IsValidIndex(0))
		{
			const TSet<const UDataTable*> TableSet = FindBestTableSet(NpcNaturalDialogComponent, Keywords);

#if !UE_BUILD_SHIPPING

			FString DebugMessage = TEXT("Selected tables for reply: { ");
			DebugMessage.Reserve(100);

			for (const UDataTable* Table : TableSet)
			{
				DebugMessage += Table->GetName() + " ";
			}

			DebugMessage += "}";
			UE_LOG(Log_DefaultDialogReplyFunction, Log, TEXT("%s"), *DebugMessage);

#endif

			// Now we check only selected tables
			if (TableSet.Num() > 0)
			{
				TArray<FReplyData> ReplyData;
				ReplyData.Reserve(TableSet.Num());

				for (const UDataTable* OutTable : TableSet)
				{
					// Find row with most keyword match
					OutTable->ForeachRow<FNaturalDialogRow_Keyword>("Searching for data from keywords", [&ReplyData, OutTable, &Keywords, this](const FName& Key, const FNaturalDialogRow_Keyword& Value)
					{
						int32 MatchCount = 0;
						int32 AbsoluteError = 0;
						const TArray<FText>& RowKeywords = Value.Keywords;

						for (const FString& InputKeyword : Keywords)
						{
							for (const FText& RowKeyword : RowKeywords)
							{
								const FString NormalizedRowKeywords = UNaturalDialogSystemLibrary::NormalizeTerm(RowKeyword.ToString());
								const int32 StringLenDifference = FMath::Abs(InputKeyword.Len() - NormalizedRowKeywords.Len());
								const int32 StringDistance = StringDistanceFunction->GetStringDistance(InputKeyword, NormalizedRowKeywords);
								const int32 StringError = FMath::Abs(StringDistance - StringLenDifference);
								if (StringError == 0)
								{
									AbsoluteError += StringLenDifference;
									MatchCount++;
									break;
								}
							}
						}

						for (int32 AnswerIndex = 0; AnswerIndex < Value.Answer.Num(); AnswerIndex++)
						{
							const FReplyData TempData = FReplyData(MatchCount, OutTable, Key, AnswerIndex, AbsoluteError);
							const int32 DataIndex = ReplyData.Find(TempData);

							// If any reply data with the same data table contains reply with equals num of keywords, then add this data asn new possibility to response, from these data we select with the hightest metric value
							if (MatchCount > 0 && (DataIndex == INDEX_NONE || ReplyData[DataIndex].NumOfMatchedKeywords == TempData.NumOfMatchedKeywords))
							{
								// Allow only rows with min keywords match
								if(TempData.NumOfMatchedKeywords >= Value.MinKeywordsMatch)
								{
									ReplyData.Add(TempData);
								}
							}
							else if (DataIndex != INDEX_NONE && ReplyData[DataIndex].NumOfMatchedKeywords < TempData.NumOfMatchedKeywords)
							{
								ReplyData[DataIndex] = TempData;
							}
						}
					});
				}

				// Find all elements with highness value
				const FReplyData BestReplyData = FindBestReply(ReplyData);

				// We need at least 3 matched keywords to select correct response, or if keywords are matched, because we can say only "hi"
				// if (BestReplyData.NumOfMatchedKeywords >= MIN_KEYWORDS_COUNT || BestReplyData.NumOfMatchedKeywords == Keywords.Num())
				if (BestReplyData.IsValid())
				{
					Result = true;
					ResultAnswer = FNaturalDialogResult(BestReplyData.InTable, BestReplyData.RowName, BestReplyData.AnswerIndex);

					// Decrease metric value
					ModifyMetricValue(BestReplyData.InTable, BestReplyData.RowName, BestReplyData.AnswerIndex);
				}
			}
		}
		else
		{
			UE_LOG(Log_DefaultDialogReplyFunction, Warning, TEXT("Empty keywords input array"));
		}

		// DEFAULT REPLIES
		// Implement default reply
		// Used if no reply was found for player input
		if (!Result && DefaultResponses)
		{
			// Fill table data
			TArray<FReplyData> ReplyData;
			DefaultResponses->ForeachRow<FNaturalDialogRow_Base>("Searching for data from keywords", [&ReplyData, this](const FName& Key, const FNaturalDialogRow_Base& Value)
			{
				for (int32 i = 0; i < Value.Answer.Num(); i++)
				{
					ReplyData.Add(FReplyData(0, DefaultResponses, Key, i, 0));
				}
			});

			if (ReplyData.IsValidIndex(0))
			{
				// Find the best, by using metric
				const FReplyData BestReplyData = FindBestReply(ReplyData);

				ResultAnswer = FNaturalDialogResult(BestReplyData.InTable, BestReplyData.RowName, BestReplyData.AnswerIndex);

				// Decrease metric value
				ModifyMetricValue(BestReplyData.InTable, BestReplyData.RowName, BestReplyData.AnswerIndex);
			}
		}
	}

	return Result;
}

TSet<const UDataTable*> UDefaultDialogReplyFunction::FindBestTableSet(const UNpcNaturalDialogComponent* NpcNaturalDialogComponent, const TArray<FString>& Keywords) const
{
	TSet<const UDataTable*> Result;

	if (!Keywords.IsValidIndex(0))
	{
		return Result;
	}

	TArray<TSet<const UDataTable*>> Combinations;
	Combinations.Reserve(20);

	if (Keywords.Num() < MIN_COMBINATION)
	{
		// Just use tables of the one keyword, we have found
		const FDictionaryData* InitialDictData = DictionaryRepresentation.Get()->GetWordData(Keywords[0]);
		Combinations.Add(InitialDictData->GetTables());
	}
	else
	{
		// Find all combinations of keywords
		for (int32 Size = MIN_COMBINATION; Size <= Keywords.Num(); Size ++)
		{
			TArray<FString> Data;
			Data.SetNum(Size);
			Combinations.Append(CombinationUtil(Keywords, Data, 0, Keywords.Num() - 1, 0, Size));
		}
	}

	TArray<int32> CachedSizes;
	CachedSizes.SetNum(Combinations.Num());

	// Intersect on tables which are available for NPC
	for (int32 i = 0; i < Combinations.Num(); i++)
	{
		Combinations[i] = Combinations[i].Intersect(OwnerComponent.Get()->GetDialogTables_Const(NpcNaturalDialogComponent));
		CachedSizes[i] = Combinations[i].Num();
	}

	// Find tables with max value
	int32 MaxIndex = INDEX_NONE;
	int32 MaxValue = -1;
	UKismetMathLibrary::MaxOfIntArray(CachedSizes, MaxIndex, MaxValue);

	if (MaxIndex != INDEX_NONE)
	{
		return Combinations[MaxIndex];
	}

	return Result;
}

void UDefaultDialogReplyFunction::HandleNewTableRegistration(const UDataTable* NewTable)
{
	if (NewTable)
	{
		FDialogMetricRow& StoredValue = Metric.Add(NewTable, FDialogMetricRow());

		if (NewTable->GetRowStruct()->IsChildOf(FNaturalDialogRow_Base::StaticStruct()))
		{
			// FNaturalDialogRow_Base for now appears only one time, for default response values
			NewTable->ForeachRow<FNaturalDialogRow_Base>("Creating metrics values", [&StoredValue](const FName& Key, const FNaturalDialogRow_Base& Value)
			{
				const int32 NumOfAnswers = Value.Answer.Num();
				StoredValue.Add(Key, NumOfAnswers);
			});
		}

		UE_LOG(Log_DefaultDialogReplyFunction, Log, TEXT("Creating metric values for table %s"), *NewTable->GetName());
	}
}

void UDefaultDialogReplyFunction::HandleRegisteredTableRemoved(const UDataTable* NewTable)
{
	Metric.Remove(NewTable);
	UE_LOG(Log_DefaultDialogReplyFunction, Log, TEXT("Removing metric values for table %s"), *NewTable->GetName());
}

void UDefaultDialogReplyFunction::HandleMatrixWeariness()
{
	for (TPair<const UDataTable*, FDialogMetricRow> Pair : Metric)
	{
		for (float* MetricValue : Pair.Value.GetAllWearinessValues())
		{
			if (FMath::RandBool())
			{
				*MetricValue *= FMath::Clamp(*MetricValue * 1.1f, 0.f, 1.f);
			}
		}
	}

	// Log metric values into log
	// Slow operation, but perfect for debugging
	LogMetricValues();
}

FReplyData UDefaultDialogReplyFunction::FindBestReply(const TArray<FReplyData>& AllReplies) const
{
	FReplyData Result;

	if (AllReplies.IsValidIndex(0))
	{
		// Find all elements with highness value
		TArray<int32> ElementIndexes;
		int32 MaxValue = INDEX_NONE;
		int32 MinError = INT_MAX;
		for (int32 i = 0; i < AllReplies.Num(); i++)
		{
			const int32 NewMaxValue = AllReplies[i].NumOfMatchedKeywords;
			const int32 NewError = AllReplies[i].AbsoluteError;

			if (NewMaxValue == MaxValue && NewError == MinError)
			{
				ElementIndexes.Add(i);
			}
			else if (NewMaxValue >= MaxValue && NewError < MinError) // Find with maximum keyword match, but with minimum len distance error
			{
				MaxValue = NewMaxValue;
				MinError = NewError;
				ElementIndexes.Reset(5);
				ElementIndexes.Add(i);
			}
		}

		// Now from all max values we choose one with best metric value
		const FReplyData* BestResultData = &AllReplies[ElementIndexes[0]];

		for (int32 i = 1; i < ElementIndexes.Num(); i++)
		{
			const FDialogMetricRow* BestRow = Metric.Find(BestResultData->InTable);
			const FDialogMetricRow* CurrentRow = Metric.Find(AllReplies[i].InTable);
			if (ensure(BestRow && CurrentRow))
			{
				for (int32 AnswerIndex = 0; AnswerIndex < CurrentRow->GetNumOfAnswers(); AnswerIndex++)
				{
					const float BestValue = BestRow->GetEvalValue(BestResultData->RowName, BestResultData->AnswerIndex);
					const float CurrentValue = CurrentRow->GetEvalValue(AllReplies[i].RowName, AnswerIndex);
					if (BestValue < CurrentValue)
					{
						BestResultData = &AllReplies[i];
					}
				}
			}
		}

		Result = *BestResultData;
	}

	return Result;
}

void UDefaultDialogReplyFunction::ModifyMetricValue(const UDataTable* InTable, const FName InRow, const int32 AnswerIndex)
{
	if (InTable)
	{
		FDialogMetricRow* MetricData = Metric.Find(InTable);
		if (MetricData)
		{
			float& MetricValue = MetricData->GetWearinessValueRef(InRow, AnswerIndex);
			if (MetricCurve)
			{
				// Curve metric calculation
				const float NewValue = MetricCurve->GetFloatValue(MetricValue);
				UE_LOG(Log_DefaultDialogReplyFunction, Log, TEXT("Metric has changed (%s | %s), (old value = %f, new value = %f)"), *InTable->GetName(), *InRow.ToString(), MetricValue, NewValue);
				MetricValue = NewValue;
			}
			else
			{
				// Default metric calculation
				const float NewValue = FMath::Clamp(MetricValue * 0.5f, 0.01f, 1.f);
				UE_LOG(Log_DefaultDialogReplyFunction, Log, TEXT("Metric has changed (%s | %s), (old value = %f, new value = %f)"), *InTable->GetName(), *InRow.ToString(), MetricValue, NewValue);
				MetricValue = NewValue;

				UE_LOG(Log_DefaultDialogReplyFunction, Warning, TEXT("Curve isn ont set, for metric calculation"));
			}

			// Log metric values into log
			// Slow operation, but perfect for debugging
			LogMetricValues();
		}
		else
		{
			UE_LOG(Log_DefaultDialogReplyFunction, Error, TEXT("Metric doesn't contain data table data (%s)"), *InTable->GetName());
		}
	}
	else
	{
		UE_LOG(Log_DefaultDialogReplyFunction, Error, TEXT("Input table is not valid in function ModifyMetricValue()"));
	}
}

void UDefaultDialogReplyFunction::LogMetricValues()
{
	// #todo ... finish this

#if !UE_BUILD_SHIPPING

	// static FString Rows = TEXT("   Rows > ");
	// static FString Columns = TEXT("V Tables ");
	// static FString VertSeparator = TEXT("||");
	// static FString HorizSeparator = TEXT("=");
	//
	// TArray<int32> TablesLens;
	// for (const TPair<const UDataTable*, FDialogMetricRow>& Table : Metric)
	// {
	// 	TablesLens.Add(Table.Value.Num());
	// }
	//
	// int32 TempIndex;
	// const int32 NumOfColumns = FMath::Max(TablesLens, &TempIndex);
	// const int32 Abc = FString::FromInt(NumOfColumns).Len();
	//
	// FString Result = "Metric values\n" + Rows + VertSeparator + "\n"; // First line
	// Result.Append(Columns + VertSeparator);                           // Second line
	//
	// for (int32 i = 0; i < NumOfColumns; i++)
	// {
	// 	Result.Append(" Row " + FString::FromInt(i + 1) + " ");
	// }

#endif
}

TArray<TSet<const UDataTable*>> UDefaultDialogReplyFunction::CombinationUtil(const TArray<FString>& InArray, TArray<FString> Data, const int32 Start, const int32 End, const int32 Index, const int32 CombinationSize) const
{
	TArray<TSet<const UDataTable*>> Result;

	// Current combination is ready
	if (Index == CombinationSize)
	{
		if (Data.IsValidIndex(0))
		{
			const FDictionaryData* InitialDictData = DictionaryRepresentation.Get()->GetWordData(Data[0]);
			TSet<const UDataTable*> TableSet = InitialDictData->GetTables();

			for (int32 i = 1; i < Data.Num(); i++)
			{
				const FDictionaryData* DictData = DictionaryRepresentation.Get()->GetWordData(Data[i]);
				TableSet = TableSet.Intersect(DictData->GetTables());
			}

			Result.Add(TableSet);
		}

		return Result;
	}

	// replace index with all possible
	// elements. The condition "end-i+1 >= r-index"
	// makes sure that including one element
	// at index will make a combination with
	// remaining elements at remaining positions
	for (int i = Start; i <= End && End - i + 1 >= CombinationSize - Index; i++)
	{
		Data[Index] = InArray[i];
		Result.Append(CombinationUtil(InArray, Data, i + 1, End, Index + 1, CombinationSize));
	}

	return Result;
}
