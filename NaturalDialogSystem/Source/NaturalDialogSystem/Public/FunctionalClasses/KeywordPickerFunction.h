// Created by Michal Chamula. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "KeywordPickerFunction.generated.h"

class UPlayerNaturalDialogComponent;
/**
 * Function parses the sentence into words and then uses the algorithm to select the keywords
 * Object outer is dictionary subsystem, there are stored dict data for your algorithm
 * By creating child class, you can define your own algorithm for pickup keywords from sentence
 */
UCLASS(Abstract, Blueprintable)
class NATURALDIALOGSYSTEM_API UKeywordPickerFunction : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * Called from UDictionarySubsystem after object creation
	 * Initialize object properties before trying pickup keywords
	 */
	virtual void InitializeKeywordPicker() {}

	/**
	 * Picks keywords from input player sentence
	 * Algorithm takes the input and from dict data assume which words are keywords from the input 
	 * @param DialogComponent - player component, which is asking for keywords
	 * @param Input - words from sentence to pickup keywords from them
	 * @return - keywords copied from input
	 */
	virtual TArray<FString> PickKeyWords(const UPlayerNaturalDialogComponent* DialogComponent, const TArray<FString>& Input)
	{
		check(0 && "Must be overridden");
		return TArray<FString>();
	}
};
