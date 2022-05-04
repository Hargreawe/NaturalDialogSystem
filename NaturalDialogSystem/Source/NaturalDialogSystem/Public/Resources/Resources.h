// Created by Michal Chamula. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Sound/DialogueWave.h"
#include "Resources.generated.h"

class AController;
class UNaturalDialogTask;
class UPlayerNaturalDialogComponent;
class UNpcNaturalDialogComponent;

#define MAX_MATRIX_SPARSE_VALUE 0.15f
#define MAX_LEN_DIFF 3

// For more information see #FDialogMetricRow definition
#define CREATE_MATRIX_VECTOR { 1.f, FMath::FRandRange(1.f - MAX_MATRIX_SPARSE_VALUE, 1.f + MAX_MATRIX_SPARSE_VALUE) }

/**
* @Key - Data table where is keyword found
*/
using FKeywordsData = TMap<const UDataTable*, TArray<FName>>;


//using FDialogMetricRow = TMap<FName, TArray<float>>;
using FDialogMetric = TMap<const UDataTable*, struct FDialogMetricRow>;


#define MIN_KEYWORDS_COUNT 3

struct FDialogMetricRow
{
	FDialogMetricRow()
		: Dummy(0)
	{
		Values.Reserve(20);
	}

	void Add(const FName InNewRowName, const int32 NumOfAnswers)
	{
		Values.SetNum(NumOfAnswers);
		for (int32 i = 0; i < NumOfAnswers; i++)
		{
			Values[i].Add(InNewRowName, CREATE_MATRIX_VECTOR);
		}
	}

	/**
	 * Returns eval value of the table replies data
	 * #todo ... improve the algorithm !!!
	 */
	float GetEvalValue(const FName EvalName, const int32 AnswerIndex) const
	{
		if (Values.IsValidIndex(AnswerIndex))
		{
			float Result = 1.f;

			const TArray<float>* FoundRow = Values[AnswerIndex].Find(EvalName);
			if (FoundRow)
			{
				for (const float InValue : *FoundRow)
				{
					Result *= InValue;
				}
			}

			return Result;
		}

		return Dummy;
	}

	TArray<float*> GetAllWearinessValues()
	{
		TArray<float*> Result;
		for (int32 i = 0; i < Values.Num(); i++)
		{
			for (TPair<FName, TArray<float>>& InPair : Values[i])
			{
				Result.Add(&InPair.Value[0]);
			}
		}
		return Result;
	}

	float& GetWearinessValueRef(const FName RowName, const int32 AnswerIndex)
	{
		if (Values.IsValidIndex(AnswerIndex))
		{
			TArray<float>* FoundRow = Values[AnswerIndex].Find(RowName);
			return FoundRow ? (*FoundRow)[0] : Dummy;
		}

		return Dummy;
	}

	int32 GetNumOfAnswers() const
	{
		return Values.Num();
	}

private:
	/**
	 * Used for metric presentation
	 * Map values is a vector of properties
	 * 
	 * Elements explanation:
	 * 1. Array of answers with properties:
	 * -------------------------
	 * 1. element - weariness value (the more is answer used, the smaller the value is)
	 * 2. element - randomize value (should be changed in every response)
	 * 3. ...
	 */
	TArray<TMap<FName, TArray<float>>> Values;

	float Dummy;
};

USTRUCT(BlueprintType, Blueprintable)
struct FNaturalDialogResult
{
	GENERATED_BODY()

	FNaturalDialogResult()
		: Table(nullptr), AnswerIndex(0) {}

	FNaturalDialogResult(const UDataTable* InTable, const FName InRowName, const int32 InAnswerIndex)
		: Table(InTable), RowName(InRowName), AnswerIndex(InAnswerIndex) {}
	
	const UDataTable* Table;
	FName RowName;
	int32 AnswerIndex;
};

USTRUCT(BlueprintType, Blueprintable)
struct FNaturalDialogAnswer
{
	GENERATED_BODY()

	FNaturalDialogAnswer() {}

	FNaturalDialogAnswer(const FText& InText)
		: Answer(InText) {}

	/** Text answer representation for player question */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText Answer;

	/**
	* When answer is used, the sound is played as dialog wave
	* Dialog can be used for many NPCs in game with different voices
	* Key - Define the actor of voice
	* Value - The voice of an actor, which is used
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TMap<TSoftClassPtr<AActor>, TSoftObjectPtr<UDialogueWave>> AnswerWave;
};

/**
* Struct used for table data with only answer
* It is used in NaturalDialogSystemComponent
* May contains data with invalid replies
*/
USTRUCT(Blueprintable, BlueprintType)
struct FNaturalDialogRow_Base : public FTableRowBase
{
	GENERATED_BODY()

	FNaturalDialogRow_Base() {};

	FNaturalDialogRow_Base(const FText& InText)
	{
		Answer.Add(InText);
	}

	/** Define answer to ask, which can be replied to player */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FNaturalDialogAnswer> Answer;
};

UENUM(BlueprintType)
enum class EDialogTableAction : uint8
{
	Add,
	Remove
};

USTRUCT(BlueprintType)
struct FNaturalDialogTableAction
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EDialogTableAction Action;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UDataTable* DialogTable;
};

/**
 * Struct used for table data for natural dialog system
 * Use for data table asset, to make new dialog data library for NPC
 */
USTRUCT(Blueprintable, BlueprintType)
struct FNaturalDialogRow : public FNaturalDialogRow_Base
{
	GENERATED_BODY()

	FNaturalDialogRow() {};

	FNaturalDialogRow(const FText& InText);

	FNaturalDialogRow(const FNaturalDialogRow_Base& Other);

	/** Define the ask, which is known for NPCs */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText Ask;

	/** Tasks will be executed, when NPC reply row answer */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<TSoftClassPtr<UNaturalDialogTask>> DialogTasks;
	
	/** When row is used as answer, these dialog tables are registered in UPlayerNaturalDialogComponent */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FNaturalDialogTableAction> DialogTables;
};

USTRUCT(Blueprintable)
struct FNaturalDialogRow_Keyword : public FNaturalDialogRow
{
	GENERATED_BODY()

	/** Keywords to match with ask */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FText> Keywords;
	
	/** To avoid wrong replies define how many keywords are needed to match this reply */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 MinKeywordsMatch = 3;
};

/**
 * Used in dictionary subsystem, it holds all data of one word
 */
USTRUCT()
struct FDictionaryData
{
	GENERATED_BODY()

	FDictionaryData() {}

	FDictionaryData(const UDataTable* InDataTable)
	{
		AddOccurence(InDataTable);
	}

	void AddOccurence(const UDataTable* InDataTable)
	{
		NumOfWords++;

		int32* TableOccurenceValue = TableOccurence.Find(InDataTable);
		if (TableOccurenceValue)
		{
			*TableOccurenceValue += 1;
		}
		else
		{
			TableOccurence.Add(InDataTable, 1);
		}
	}

	/** Get num of all tables where term is found */
	int32 GetTableOccurenceCount() const { return TableOccurence.Num(); }

	/** Return count of all words in system */
	static int32 GetNumOfWords() { return FDictionaryData::NumOfWords; }

	/** Returns set of tables where term is found */
	TSet<const UDataTable*> GetTables() const
	{
		TArray<const UDataTable*> Result;
		TableOccurence.GetKeys(Result);
		return TSet<const UDataTable*>(Result);
	}

	/** Returns term occurence in data table */
	int32 GetOccurenceCount(const UDataTable* InDataTable) const
	{
		const int32* TableOccurenceValue = TableOccurence.Find(InDataTable);
		if (TableOccurenceValue)
		{
			return *TableOccurenceValue;
		}
		return 0;
	}

	/** Used for UDictionarySubsystem, after game restarts */
	static void ResetNumOfWords() { FDictionaryData::NumOfWords = 0; }

private:
	UPROPERTY()
	TMap<const UDataTable*, int32> TableOccurence;

	static int32 NumOfWords;
};

struct FReplyData
{
	FReplyData()
		: NumOfMatchedKeywords(0), InTable(nullptr), AnswerIndex(0), AbsoluteError(0) {}

	FReplyData(const UDataTable* InDataTable)
		: NumOfMatchedKeywords(0), InTable(InDataTable), AnswerIndex(0), AbsoluteError(0) {}

	FReplyData(const int32 KeywordsMatchCount, const UDataTable* InDataTable, const FName InRow, int32 InAnswerIndex, int32 InAbsoluteError)
		: NumOfMatchedKeywords(KeywordsMatchCount), InTable(InDataTable), RowName(InRow), AnswerIndex(InAnswerIndex), AbsoluteError(InAbsoluteError) {};

	int NumOfMatchedKeywords;
	const UDataTable* InTable;
	FName RowName;
	int32 AnswerIndex;
	int32 AbsoluteError;


	inline bool operator==(const FReplyData& Other) const { return InTable == Other.InTable; }

	inline bool operator<(const FReplyData& Other) const { return NumOfMatchedKeywords < Other.NumOfMatchedKeywords; }

	__forceinline bool IsValid() const
	{
		return InTable && !RowName.IsNone();
	}
	// void operator=(const FReplyData& Other)
	// {
	// 	NumOfMatchedKeywords = Other.NumOfMatchedKeywords;
	// 	InTable = Other.InTable;
	// 	RowName = Other.RowName;
	// }
};

/**
 * Struct contains data about player character data
 * Used as param in dialog task, there are stored important references for function ExecuteTask()
 * Can be get from NaturalDialogTask object using function GetPlayerData()
 */
USTRUCT(BlueprintType)
struct FDialogTaskPlayerData
{
	GENERATED_BODY()

	FDialogTaskPlayerData();

	explicit FDialogTaskPlayerData(APlayerController* InPlayerController);

	/** Reference to the player character, which is asking for reply */
	UPROPERTY(BlueprintReadOnly)
	APawn* PlayerPawn;

	/** Reference to the player natural dialog component, which is asking for reply */
	UPROPERTY(BlueprintReadOnly)
	UPlayerNaturalDialogComponent* NaturalDialogSystemComponent;

	/** Reference to the player controller, which is asking for reply */
	UPROPERTY(BlueprintReadOnly)
	APlayerController* PlayerController;

	/** Reference to the player state, which is asking for reply */
	UPROPERTY(BlueprintReadOnly)
	APlayerState* PlayerState;

	bool IsValid() const;
};

/**
 * Struct contains data about task owner character
 * Used as param in dialog task, there are stored important references for function ExecuteTask()
 * Can be get from NaturalDialogTask object using function GetNPCData()
 */
USTRUCT(BlueprintType)
struct FDialogTaskNPCData
{
	GENERATED_BODY()

	FDialogTaskNPCData();

	explicit FDialogTaskNPCData(AActor* InOwningActor);

	/** Reference to the task creator */
	UPROPERTY(BlueprintReadOnly)
	AActor* OwningActor;

	/** Reference to the task creator */
	UPROPERTY(BlueprintReadOnly)
	AController* OwningController;

	/** Reference to the task component, where task was created from */
	UPROPERTY(BlueprintReadOnly)
	UNpcNaturalDialogComponent* NaturalDialogSystemComponent;

	bool IsValid() const;
};
