// Created by Michal Chamula. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "DeveloperSettings/Public/Engine/DeveloperSettings.h"

#include "FunctionalClasses/DictionaryWordPickerFunction.h"
#include "FunctionalClasses/KeywordPickerFunction.h"
#include "Resources/DictionaryRepresentation.h"
#include "NaturalDialogSystemSettings.generated.h"


/**
 * 
 */
UCLASS(Config = Engine, DefaultConfig)
class NATURALDIALOGSYSTEM_API UNaturalDialogSystemSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UNaturalDialogSystemSettings();

protected:
	/** If true, UDictionarySubsystem will log more info into output log */
	UPROPERTY(EditAnywhere, config, Category = "Debug")
	uint8 bDebugDictionary : 1;

	/** Defines string metric function class, for evaluate distance of two strings in UDictionarySubsystem */
	UPROPERTY(EditAnywhere, config, Category = "Functions")
	TSubclassOf<UDictionaryWordPickerFunction> DictionaryWordPickerFunctionClass;

	/** Defines keyword function class, which evaluate input sentence and picks keywords for response */
	UPROPERTY(EditAnywhere, config, Category = "Functions")
	TSubclassOf<UKeywordPickerFunction> KeywordPickerFunctionClass;

	/** Defines dictionary representation of words */
	UPROPERTY(EditAnywhere, config, Category = "Functions")
	TSubclassOf<UDictionaryRepresentation> DictionaryRepresentationClass;

public:
	/** Returns true, if dictionary debug is enabled  */
	FORCEINLINE bool GetDebugDictionary() const { return bDebugDictionary; }

	/** Returns string distance function class */
	FORCEINLINE TSubclassOf<UDictionaryWordPickerFunction> GetDictionaryWordPickerFunctionClass() const { return DictionaryWordPickerFunctionClass; }

	/** Returns keyword picker function class */
	FORCEINLINE TSubclassOf<UKeywordPickerFunction> GetKeywordPickerFunctionClass() const { return KeywordPickerFunctionClass; }

	/** Returns dictionary representation class */
	FORCEINLINE TSubclassOf<UDictionaryRepresentation> GetDictionaryRepresentationClass() const { return DictionaryRepresentationClass; }
};
