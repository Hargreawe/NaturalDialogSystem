// Created by Michal Chamula. All rights reserved.


#include "FunctionalClasses/DictionaryWordPickerFunction.h"
#include "DefaultClasses/LevenshteinDistanceFunction.h"


DEFINE_LOG_CATEGORY(LogDictionaryWordPickerFunction);

UDictionaryWordPickerFunction::UDictionaryWordPickerFunction()
{
	StringDistanceFunction = ULevenshteinDistanceFunction::StaticClass();
}

void UDictionaryWordPickerFunction::InitializeWordPicker()
{
	if (StringDistanceFunction)
	{
		CreateStringMetricFunctionObject(StringDistanceFunction);
		UE_LOG(LogDictionaryWordPickerFunction, Log, TEXT("Used %s as StringDistanceFunction"), *GetClass()->GetName());
	}
	else
	{
		CreateStringMetricFunctionObject(ULevenshteinDistanceFunction::StaticClass());
		UE_LOG(LogDictionaryWordPickerFunction, Warning, TEXT("StringDistanceFunction is null in class %s"), *GetClass()->GetName());
	}
}

void UDictionaryWordPickerFunction::CreateStringMetricFunctionObject(const TSubclassOf<UStringDistanceFunction> MetricClass)
{
	StringMetricDistanceFunctionInstance = NewObject<UStringDistanceFunction>(this, MetricClass);
	StringMetricDistanceFunctionInstance->InitializeDistFunction();

	// Check if instance has overriden abstract function
	StringMetricDistanceFunctionInstance->GetStringDistance(TEXT(""), TEXT(""));
}
