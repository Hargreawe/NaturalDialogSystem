// Created by Michal Chamula. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "FunctionalClasses/KeywordPickerFunction.h"
#include "Tf_idf_PickerFunction.generated.h"

class UDictionaryRepresentation;


DECLARE_LOG_CATEGORY_EXTERN(Log_Tf_Idf_PickerFunction, Log, All);

/**
 * Implementation of Term Frequency — Inverse Document Frequency (TF-IDF) algorithm for pick keywords
 * Followed step by step @see - http://taozhaojie.github.io/2015/06/12/tfidf/
 * @see https://www.analyticsvidhya.com/blog/2020/02/quick-introduction-bag-of-words-bow-tf-idf/      - Here is all info about this algorithm
 */
UCLASS()
class NATURALDIALOGSYSTEM_API UTf_idf_PickerFunction : public UKeywordPickerFunction
{
	GENERATED_BODY()
	
public:
	/**
	* Called from UDictionarySubsystem after object creation
	* Initialize object properties before trying pickup keywords
	*/
	virtual void InitializeKeywordPicker() override;

	/**
	* Picks keywords from input player sentence
	* Algorithm takes the input and from dict data assume which words are keywords from the input 
	* @param DialogComponent - asking component
	* @param Input - words from sentence to pickup keywords from them
	* @return - keywords copied from input
	*/
	virtual TArray<FString> PickKeyWords(const UPlayerNaturalDialogComponent* DialogComponent, const TArray<FString>& Input) override;
	
private:	
	/** Data cached from dictionary subsystem */
	TWeakObjectPtr<const UDictionaryRepresentation> DictionaryData;

	int32 TablesCount;
};
