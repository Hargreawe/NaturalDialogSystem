// Created by Michal Chamula. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "DictionaryWordPickerFunction.generated.h"

class UStringDistanceFunction;


DECLARE_LOG_CATEGORY_EXTERN(LogDictionaryWordPickerFunction, Log, All);

/**
 * 
 */
UCLASS(Abstract, Blueprintable)
class NATURALDIALOGSYSTEM_API UDictionaryWordPickerFunction : public UObject
{
	GENERATED_BODY()

public:
	UDictionaryWordPickerFunction();

protected:
	/** Define your own string distance function class */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category= "Dictionary word picker")
	TSubclassOf<UStringDistanceFunction> StringDistanceFunction;

public:
	/**
	* Called from UDictionarySubsystem after object creation
	* Initialize object properties before trying pickup keywords
	*/
	virtual void InitializeWordPicker();

	/**
	* Picks keywords from input player sentence
	* Algorithm takes the input and from dict data assume which words are keywords from the input 
	* @param Input - words from sentence to pickup keywords from them
	* @return - keywords copied from input
	*/
	virtual FString PickWordFromDictionary(const FString& Input) const
	{
		check(0 && "Must be overridden");
		return FString();
	}

protected:
	/** Helper function, creates string metric object instance */
	void CreateStringMetricFunctionObject(const TSubclassOf<UStringDistanceFunction> MetricClass);

protected:
	/**
	 * Strong ref to string metric singleton function
	 * Levensthein function is used as default string metric function
	 * Override InitializeWordPicker() function to create your own string metric instance
	 */
	UPROPERTY()
	UStringDistanceFunction* StringMetricDistanceFunctionInstance;
};
