// Created by Michal Chamula. All rights reserved.

#pragma once

#include "CoreMinimal.h"

#include "Core/DictionarySubsystem.h"
#include "FunctionalClasses/DictionaryWordPickerFunction.h"
#include "DefaultDictionaryPickerFunction.generated.h"


DECLARE_LOG_CATEGORY_EXTERN(Log_DefaultDictPickerFunction, Log, All);

/**
 * 
 */
UCLASS()
class NATURALDIALOGSYSTEM_API UDefaultDictionaryPickerFunction : public UDictionaryWordPickerFunction
{
	GENERATED_BODY()

public:
	virtual void InitializeWordPicker() override;
	virtual FString PickWordFromDictionary(const FString& Input) const override;
	
private:
	/** Cached outer dictionary */
	TWeakObjectPtr<const UDictionarySubsystem> CachedDictionarySubsystem;
};
