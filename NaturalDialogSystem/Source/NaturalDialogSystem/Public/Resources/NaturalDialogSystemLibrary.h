// Created by Michal Chamula. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "NaturalDialogSystemLibrary.generated.h"

#define NULL_CHARACTER 0
#define SPACE_CHARACTER 32
#define TAG_CHARACTER 36
#define A_CHARACTER 65

/**
 * 
 */
UCLASS()
class NATURALDIALOGSYSTEM_API UNaturalDialogSystemLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()


public:
	/**
	 * Returns list of all data tables in a game, with dialog struct
	 * @warning - for optimization, cache this data, because this list all assets in project content, that's mean this is slow operation
	 */
	static TSet<UDataTable*> GetListOfDialogDataTables();

	static TArray<FString> SplitSentenceIntoNormalizedTerms(const FString& Input);
	
	static FString NormalizeTerm(const FString& Input);

	static TArray<FString> SplitToSentences(const FString& Input);

	UFUNCTION(BlueprintCallable, BlueprintPure)
	static FString NormalizeWord(const FString& Input);
	
private:
	static CHAR NormalizeCharacter(CHAR Character);
	
	/** Returns true, if character equals to any special character */
	FORCEINLINE static bool IsSpecialChar(const TCHAR Character) { return Character < A_CHARACTER; }
	/** Returns true, if character equals to any special character */
	FORCEINLINE static bool IsSpaceChar(const TCHAR Character) { return Character == SPACE_CHARACTER; }
	/** Returns true, if character equals to $ */
	FORCEINLINE static bool IsTagCharacter(const TCHAR Character) { return Character == TAG_CHARACTER; }
	/** Returns true, if character equals to .!? */
	FORCEINLINE static bool IsSentenceSeparator(const TCHAR Character) { return Character == 33 || Character == 46 || Character == 63; }
	
	/**
	 * Find all assets of the given type recursively in project content folder
	 * @return - List of found assets references (like data tables, materials, textures, ...)
	 */
	template<typename T>
	static TArray<T*> GetListOfContentObjects_Recursive();

	/**
	 * Find asset by given file in project content folder
	 * @param File - File which we want to find in assets in format (Path/File)
	 * @return - Find result of assets reference (like data table, material, texture, ...)
	 */
	template<class T>
	static T* GetElementDataTable(const FString& File);
};

template<typename T>
TArray<T*> UNaturalDialogSystemLibrary::GetListOfContentObjects_Recursive()
{
	TArray<T*> Result;
	Result.Reserve(10);

	const FString ElementsDir = FPaths::ProjectContentDir();

	TArray<FString> OutAssets;
	IFileManager::Get().FindFilesRecursive(OutAssets, *ElementsDir, TEXT("*"), true, false);

	for (const FString& AssetName : OutAssets)
	{
		T* LoadedAsset = GetElementDataTable<T>(AssetName);
		if (LoadedAsset)
		{
			Result.Add(LoadedAsset);
		}
	}

	return Result;
}

template<class T>
T* UNaturalDialogSystemLibrary::GetElementDataTable(const FString& File)
{
	FString FileName;
	FString FilePath;

	int32 DotPosition = -1;
	if (File.FindLastChar('/', DotPosition))
	{
		FilePath = File.Left(DotPosition);
		FileName = File.Right(File.Len() - DotPosition - 1);
	}

	const int32 Index = FilePath.Find(TEXT("Content"));
	FilePath.RightInline(FilePath.Len() - Index - 8);

	if (FileName.FindChar('.', DotPosition))
	{
		FileName.LeftInline(DotPosition);
	}

	const FString FullName = FPaths::Combine(TEXT("/Game"), FilePath, FileName) + TEXT(".") + FileName;
	UObject* Object = StaticLoadObject(UObject::StaticClass(), nullptr, *FullName);
	return Cast<T>(Object);
}
