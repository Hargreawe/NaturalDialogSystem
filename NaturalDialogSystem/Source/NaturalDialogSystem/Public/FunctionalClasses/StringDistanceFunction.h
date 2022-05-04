// Created by Michal Chamula. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "StringDistanceFunction.generated.h"

/**
 * Function takes two string and measure metric distance between them
 * You can create your own algorithm for measure string distances, by creating child class and set it into plugin settings
 * In plugin is created default algorithm (Levenshtein distance)
 */
UCLASS(Abstract, Blueprintable)
class NATURALDIALOGSYSTEM_API UStringDistanceFunction : public UObject
{
	GENERATED_BODY()

public:
	/**
	* Called from UDictionaryWordPickerFunction after object creation
	* Initialize object properties before trying pickup keywords
	*/
	virtual void InitializeDistFunction() {}

	/**
	 * Override this function to create your own algorithm for measure string distances
	 * @param InputA - First string to measure with second param
	 * @param InputB - Second string to measure with first param
	 * @return - Result distance value between input strings
	 */
	virtual uint32 GetStringDistance(const FString& InputA, const FString& InputB) const
	{
		check(0 && "Must be overridden");
		return 0;
	}
};
