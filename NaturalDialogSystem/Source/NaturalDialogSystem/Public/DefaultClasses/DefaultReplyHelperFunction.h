// Created by Michal Chamula. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "FunctionalClasses/ReplyHelperFunction.h"
#include "DefaultReplyHelperFunction.generated.h"

class UDictionarySubsystem;
class UDictionaryWordPickerFunction;

struct FUsedOptions
{
	FUsedOptions(const UDataTable* InTable, const FName) {}

	const UDataTable* Table;
	TMap<FName, int32> Metric;
};

struct FOptionData
{
	FOptionData() {}
	explicit FOptionData(const FString& InWord) : Word(InWord) {}

	__forceinline void Add(const UDataTable* InTable, const FName Row)
	{
		TSet<FName>* Rows = Options.Find(InTable);
		if(Rows)
		{
			Rows->Add(Row);
		}
		else
		{
			Options.Add(InTable, TSet<FName>( {Row} ));
		}

		FNaturalDialogRow* RowData = InTable->FindRow<FNaturalDialogRow>(Row, TEXT("Row"));
		if(ensure(RowData))
		{
			OutputOptions.Add(RowData->Ask.ToString());
		}
	}

	__forceinline bool IsValid() const { return !GetWord().IsEmpty() || GetOptions().Num() > 0; }
	
	const FString& GetWord() const { return Word; }
	const TMap<const UDataTable*, TSet<FName>>& GetOptions() const { return Options; }
	const TSet<FString>& GetOutputOptions() const { return OutputOptions; }

private:
	/** Word associated to output options */
	FString Word;
	/** Output options */
	TMap<const UDataTable*, TSet<FName>> Options;

	TSet<FString> OutputOptions;
};

/**
 * 
 */
UCLASS()
class NATURALDIALOGSYSTEM_API UDefaultReplyHelperFunction : public UReplyHelperFunction
{
	GENERATED_BODY()

public:
	virtual void InitializeDialogReplyHelper() override;
	
	virtual bool FindAskOptions(const FText& Input, const UNpcNaturalDialogComponent* NpcNaturalDialogComponent, TSet<FString>& OutOptions) override;

	virtual bool FindBestOption(const FText& Input, const UNpcNaturalDialogComponent* NpcNaturalDialogComponent, FText& OutOption) override;

protected:
	FOptionData GetFilteredWordData(const FString& Word, const UNpcNaturalDialogComponent* NpcNaturalDialogComponent, const int32 InputWordQueue) const;
	
private:
	TArray<FOptionData> LastInputs;
	
	/** Strong ref to word picker singleton function  */
	UPROPERTY()
	UDictionarySubsystem* CachedDictionarySubsystem;

	UPROPERTY()
	const UDictionaryWordPickerFunction* PickerFunction;
};
