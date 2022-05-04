// Created by Michal Chamula. All rights reserved.

#pragma once

#include "CoreMinimal.h"

#include "Resources/Resources.h"
#include "UObject/NoExportTypes.h"
#include "ReplyHelperFunction.generated.h"

class UNpcNaturalDialogComponent;


DECLARE_LOG_CATEGORY_EXTERN(LogReplyHelperFunction, Log, All);

/**
 * 
 */
UCLASS(Abstract, Blueprintable)
class NATURALDIALOGSYSTEM_API UReplyHelperFunction : public UObject
{
	GENERATED_BODY()

public:
	/**
	* Called from UPlayerNaturalDialogComponent after object creation
	* Initialize object properties before trying to find ask options
	*/
	virtual void InitializeDialogReplyHelper() {};

	/**
	 * Generate all possible ask options for player input
	 * Uses player input text, to find out which options are appropriate
	 * Generated output can be shown in UI as help for player when he don't know how to continue in asking
	 * Output is an array of text, which consists of (Input + next possible word)
	 * e.g.: When input is "Where can I", the result is an array OutOptions = {"Where can I go", "Where can I find", "Where can I do", ...}
	 * These options are generated from owning tables of player using UNpcNaturalDialogComponent
	 * So the output is not the whole possible sentence, but only a "Input + help word"
	 * @param Input - Player input which already used
	 * @param NpcNaturalDialogComponent - Defines which NPC we are asking for
	 * @param OutOptions - All possible options, described above
	 * @return - True, if we have found at least one help word, otherwise false
	 */
	virtual bool FindAskOptions(const FText& Input, const UNpcNaturalDialogComponent* NpcNaturalDialogComponent, TSet<FString>& OutOptions)
	{
		check(0 && "Must be overridden");
		return false;
	}


	/**
	 * Generate possible ask options for player input (only ONE)
	 * Uses player input text, to find out which options are appropriate
	 * Generated output can be shown in UI as help for player when he don't know how to continue in asking
	 * Output is a text, which consists of (Input + next possible word)
	 * e.g.: When input is "Where can I", the result is text OutOption = "Where can I go"
	 * These options are generated from owning tables of player using UNpcNaturalDialogComponent
	 * So the output is not the whole possible sentence, but only a "Input + help word"
	 * @param Input - Player input which already used
	 * @param NpcNaturalDialogComponent - Defines which NPC we are asking for
	 * @param OutOption - The best possible option
	 * @return - True, if we have found help word, otherwise false
	 */
	virtual bool FindBestOption(const FText& Input, const UNpcNaturalDialogComponent* NpcNaturalDialogComponent, FText& OutOption)
	{
		check(0 && "Must be overridden");
		return false;
	}
};
