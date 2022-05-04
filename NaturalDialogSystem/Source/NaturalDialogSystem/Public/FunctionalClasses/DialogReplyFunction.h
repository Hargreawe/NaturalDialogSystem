// Created by Michal Chamula. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Resources/Resources.h"
#include "UObject/NoExportTypes.h"
#include "DialogReplyFunction.generated.h"

class UNpcNaturalDialogComponent;
/**
 * Function trying to find the best answer for player dialog input
 * Instance is created every UNaturalDialogSystemComponent and active during game session
 * Takes keywords, returned from UDictionarySubsystem and make a reply for user using virtual function
 */
UCLASS(Abstract, Blueprintable)
class NATURALDIALOGSYSTEM_API UDialogReplyFunction : public UObject
{
	GENERATED_BODY()

public:
	/**
	* Called from UPlayerNaturalDialogComponent after object creation
	* Initialize object properties before trying pickup reply
	*/
	virtual void InitializeDialogReplyPicker() {};

	/**
	 * Picks the best answer for player input using found keywords
	 * @param Sentence - Sentence from player input
	 * @param NpcNaturalDialogComponent - Npc, we are asking for reply
	 * @param ResultAnswer - Answer result from table
	 * @return - Returns True, if was reply found, otherwise false, then is used default data table from NaturalDialogSystemComponent
	 */
	virtual bool GenerateReply(const FString& Sentence, const UNpcNaturalDialogComponent* NpcNaturalDialogComponent, FNaturalDialogResult& ResultAnswer)
	{
		ResultAnswer = FNaturalDialogResult();
		check(0 && "Must be overridden");
		return false;
	}
};
