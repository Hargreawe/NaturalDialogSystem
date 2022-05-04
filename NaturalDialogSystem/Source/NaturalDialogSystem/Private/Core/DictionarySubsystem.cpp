// Created by Michal Chamula. All rights reserved.


#include "Core/DictionarySubsystem.h"
#include "Module/NaturalDialogSystemSettings.h"
#include "DefaultClasses/DefaultDictionaryPickerFunction.h"
#include "DefaultClasses/DefaultDictionaryRepresentation.h"
#include "DefaultClasses/LevenshteinDistanceFunction.h"
#include "DefaultClasses/Tf_idf_PickerFunction.h"
#include "FunctionalClasses/DictionaryWordPickerFunction.h"
#include "Logging/MessageLog.h"
#include "Misc/UObjectToken.h"
#include "Resources/DictionaryRepresentation.h"
#include "Resources/NaturalDialogSystemLibrary.h"
#include "Resources/Resources.h"


DEFINE_LOG_CATEGORY(LogDictSubsystem);

void UDictionarySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Create an instance for string distance function, we will use it for words correction
	const UNaturalDialogSystemSettings* Settings = GetDefault<UNaturalDialogSystemSettings>();

	// DICTIONARY REPRESENTATION ----------------------------------------------------------------------------------------------------------------------------------

	// Create dictionary representation object instance
	const TSubclassOf<UDictionaryRepresentation> DictRepresentationClass = Settings->GetDictionaryRepresentationClass();
	if (DictRepresentationClass)
	{
		ConstructDictObject(DictRepresentationClass);
		UE_LOG(LogDictSubsystem, Log, TEXT("For dictionary representation is used %s class"), *DictRepresentationClass->GetName());
	}
	else
	{
		UClass* DefaultClass = UDefaultDictionaryRepresentation::StaticClass();
		ConstructDictObject(DefaultClass);
		UE_LOG(LogDictSubsystem, Warning, TEXT("String metric function is not set in project settings, using default %s"), *DefaultClass->GetName());
	}

	// DICTIONARY WORD PICKER ----------------------------------------------------------------------------------------------------------------------------------

	// Create dictionary word picker instance
	const TSubclassOf<UDictionaryWordPickerFunction> DictionaryWordPickerClass = Settings->GetDictionaryWordPickerFunctionClass();
	if (DictionaryWordPickerClass)
	{
		ConstructDictionaryWordPickerFunctionObject(DictionaryWordPickerClass);
		UE_LOG(LogDictSubsystem, Log, TEXT("For string distance function was used %s class"), *DictionaryWordPickerClass->GetName());
	}
	else
	{
		UClass* DefaultClass = UDefaultDictionaryPickerFunction::StaticClass();
		ConstructDictionaryWordPickerFunctionObject(DefaultClass);
		UE_LOG(LogDictSubsystem, Warning, TEXT("String metric function is not set in project settings, using default %s"), *DefaultClass->GetName());
	}

	// KEYWORD PICKER ----------------------------------------------------------------------------------------------------------------------------------

	// Create keyword picker function instance
	const TSubclassOf<UKeywordPickerFunction> KeywordPickerClass = Settings->GetKeywordPickerFunctionClass();
	if (KeywordPickerClass)
	{
		ConstructKeywordPickerObject(KeywordPickerClass);
		UE_LOG(LogDictSubsystem, Log, TEXT("For keyword picker was used %s class"), *KeywordPickerClass->GetName());
	}
	else
	{
		UClass* DefaultClass = UTf_idf_PickerFunction::StaticClass();
		ConstructKeywordPickerObject(DefaultClass);
		UE_LOG(LogDictSubsystem, Warning, TEXT("String metric function is not set in project settings, using default %s"), *DefaultClass->GetName());
	}

	// OTHER ----------------------------------------------------------------------------------------------------------------------------------

	// Cache all dialog tables to subsystem after game start
	GameNaturalDialogTables = UNaturalDialogSystemLibrary::GetListOfDialogDataTables();
	FDictionaryData::ResetNumOfWords();

	// Iterate all tables and register all words from them
	for (const UDataTable* Table : GameNaturalDialogTables)
	{
		RegisterWordsFromTable(Table);
	}
}

void UDictionarySubsystem::Deinitialize()
{
	Super::Deinitialize();

#if WITH_EDITOR

	const UNaturalDialogSystemSettings* Settings = GetDefault<UNaturalDialogSystemSettings>();

	FMessageLog Log = FMessageLog(TEXT("PIE"));

	if (!Settings->GetDictionaryRepresentationClass())
	{
		const FString Warning = TEXT("Natural dialog system: Set DictionaryRepresentation class in project settings");
		Log.SuppressLoggingToOutputLog().Warning()->AddToken(FUObjectToken::Create(this))->AddToken(FTextToken::Create(FText::AsCultureInvariant(Warning)));
	}

	if (!Settings->GetDictionaryWordPickerFunctionClass())
	{
		const FString Warning = TEXT("Natural dialog system: Set StringDistanceFunction class in project settings");
		Log.SuppressLoggingToOutputLog().Warning()->AddToken(FUObjectToken::Create(this))->AddToken(FTextToken::Create(FText::AsCultureInvariant(Warning)));
	}

	if (!Settings->GetKeywordPickerFunctionClass())
	{
		const FString Warning = TEXT("Natural dialog system: Set KeywordPickerFunction class in project settings");
		Log.SuppressLoggingToOutputLog().Warning()->AddToken(FUObjectToken::Create(this))->AddToken(FTextToken::Create(FText::AsCultureInvariant(Warning)));
	}

	Log.Open(EMessageSeverity::Info);

#endif
}

TArray<FString> UDictionarySubsystem::GenerateKeywords(const UPlayerNaturalDialogComponent* DialogComponent, const FString& InputText)
{
	TArray<FString> Result;

	if (!DictionaryWordPickerFunctionInstance)
	{
		UClass* DefaultClass = ULevenshteinDistanceFunction::StaticClass();
		ConstructDictionaryWordPickerFunctionObject(DefaultClass);
		UE_LOG(LogDictSubsystem, Warning, TEXT("Strig metric function is not set in project settings, using default %s"), *DefaultClass->GetName());
	}

	if (!KeywordPickerFunctionInstance)
	{
		UClass* DefaultClass = UTf_idf_PickerFunction::StaticClass();
		ConstructKeywordPickerObject(DefaultClass);
		UE_LOG(LogDictSubsystem, Warning, TEXT("String metric function is not set in project settings, using default %s"), *DefaultClass->GetName());
	}

	TArray<FString> FixedWords;
	for (const FString& InputWord : UNaturalDialogSystemLibrary::SplitSentenceIntoNormalizedTerms(InputText))
	{
		const FString FixedWord = DictionaryWordPickerFunctionInstance->PickWordFromDictionary(InputWord);
		if (FixedWord.Len() > 0)
		{
			FixedWords.Add(FixedWord);
			UE_LOG(LogDictSubsystem, Log, TEXT("Word (%s) was fixed to (%s)"), *InputWord, *FixedWord);
		}
		else
		{
			UE_LOG(LogDictSubsystem, Warning, TEXT("Word (%s) was not recognized"), *InputWord);
		}
	}

	// The result array of keywords
	const TArray<FString> PickedKeyWords = KeywordPickerFunctionInstance->PickKeyWords(DialogComponent, FixedWords);
	return PickedKeyWords;
}

void UDictionarySubsystem::ConstructDictObject(const TSubclassOf<UDictionaryRepresentation> DictClass)
{
	DictionaryData = NewObject<UDictionaryRepresentation>(this, DictClass);
	DictionaryData->InitializeDictionary();

	// Check if instance has overriden abstract function
	DictionaryData->RegisterWord(TEXT(""), nullptr);
}

void UDictionarySubsystem::ConstructKeywordPickerObject(const TSubclassOf<UKeywordPickerFunction> KeywordPickerClass)
{
	KeywordPickerFunctionInstance = NewObject<UKeywordPickerFunction>(this, KeywordPickerClass);
	KeywordPickerFunctionInstance->InitializeKeywordPicker();

	// Check if instance has overriden abstract function
	KeywordPickerFunctionInstance->PickKeyWords(nullptr, TArray<FString>());
}

void UDictionarySubsystem::ConstructDictionaryWordPickerFunctionObject(const TSubclassOf<UDictionaryWordPickerFunction> StringFunctionClass)
{
	DictionaryWordPickerFunctionInstance = NewObject<UDictionaryWordPickerFunction>(this, StringFunctionClass);
	DictionaryWordPickerFunctionInstance->InitializeWordPicker();

	// Check if instance has overriden abstract function
	DictionaryWordPickerFunctionInstance->PickWordFromDictionary(TEXT(""));
}

void UDictionarySubsystem::RegisterWordsFromTable(const UDataTable* InTable)
{
	if (ensure(InTable))
	{
		if (!ensure(DictionaryData))
		{
			ConstructDictObject(UDefaultDictionaryRepresentation::StaticClass());
		}

		// Iterate all rows from table and register all words into system
		if (InTable->GetRowStruct()->IsChildOf(FNaturalDialogRow_Keyword::StaticStruct()))
		{
			InTable->ForeachRow<FNaturalDialogRow_Keyword>(TEXT("Words registration"), [=](const FName& Key, const FNaturalDialogRow_Keyword& Value)
			{
				TArray<FString> AllWords = UNaturalDialogSystemLibrary::SplitSentenceIntoNormalizedTerms(Value.Ask.ToString());
				
				for(int32 i = 0 ; i < Value.Answer.Num() ; i++)
				{
					AllWords.Append(UNaturalDialogSystemLibrary::SplitSentenceIntoNormalizedTerms(Value.Answer[i].Answer.ToString()));
				}
				
				for(const FText& Keyword : Value.Keywords)
				{
					AllWords.Add(UNaturalDialogSystemLibrary::NormalizeTerm(Keyword.ToString()));
				}
			
				for (const FString& Word : AllWords)
				{
					DictionaryData->RegisterWord(Word, InTable);
				}
			});
		}
		else
		{
			InTable->ForeachRow<FNaturalDialogRow>(TEXT("Words registration"), [=](const FName& Key, const FNaturalDialogRow& Value)
			{
				TArray<FString> AllWords = UNaturalDialogSystemLibrary::SplitSentenceIntoNormalizedTerms(Value.Ask.ToString());

				for(int32 i = 0 ; i < Value.Answer.Num() ; i++)
				{
					AllWords.Append(UNaturalDialogSystemLibrary::SplitSentenceIntoNormalizedTerms(Value.Answer[i].Answer.ToString()));
				}
			
				for (const FString& Word : AllWords)
				{
					DictionaryData->RegisterWord(Word, InTable);
				}
			});
		}
	}
}
