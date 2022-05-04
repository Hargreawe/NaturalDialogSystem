// Created by Michal Chamula. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "FunctionalClasses/StringDistanceFunction.h"
#include "LevenshteinDistanceFunction.generated.h"

/**
 * 
 */
UCLASS()
class NATURALDIALOGSYSTEM_API ULevenshteinDistanceFunction : public UStringDistanceFunction
{
	GENERATED_BODY()

public:
	virtual uint32 GetStringDistance(const FString& InputA, const FString& InputB) const override;
};
