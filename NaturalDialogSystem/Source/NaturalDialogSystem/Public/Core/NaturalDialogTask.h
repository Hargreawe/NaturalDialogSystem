// Created by Michal Chamula. All rights reserved.

#pragma once

#include "CoreMinimal.h"

#include "GameFramework/PlayerController.h"
#include "Resources/Resources.h"
#include "UObject/Object.h"
#include "NaturalDialogTask.generated.h"

class UPlayerNaturalDialogComponent;
class UNpcNaturalDialogComponent;


DECLARE_LOG_CATEGORY_EXTERN(LogNaturalDialogTask, All, Log);

/**
 * 
 */
UCLASS(Abstract, Blueprintable)
class NATURALDIALOGSYSTEM_API UNaturalDialogTask : public UObject
{
	GENERATED_BODY()
	
	friend class UPlayerNaturalDialogComponent;

public:
	UNaturalDialogTask();

public:
	/** Force task finish execution */
	UFUNCTION(BlueprintCallable, Category="Natural dialog task")
	void FinishTaskExecution();

	/** Returns all player struct for external references */
	UFUNCTION(BlueprintCallable, Category="Natural dialog task")
	const FDialogTaskPlayerData& GetPlayerData() const { return CachedPlayerData; }

	/** Returns all NPC struct for external references */
	UFUNCTION(BlueprintCallable, meta=(DisplayName = "Get NPC Data"), Category="Natural dialog task")
	const FDialogTaskNPCData& GetNPCData() const { return CachedNPCData; }

	/** Returns whether this task has network authority */
	UFUNCTION(BlueprintCallable, Category="Natural dialog task")
	bool HasAuthority() const { return bHasAuthority; }

	// UObject overrides
	virtual UWorld* GetWorld() const override;
	virtual int32 GetFunctionCallspace(UFunction* Function, FFrame* Stack) override;
	virtual bool CallRemoteFunction(UFunction* Function, void* Parameters, FOutParmRec* OutParams, FFrame* Stack) override;
	virtual bool IsSupportedForNetworking() const override { return true; }
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	// ~ UObject overrides

	virtual void ExecuteTask(APlayerController* AskingPlayer, AActor* OwningActor);

protected:
	virtual void StartTaskExecution_Internal();
	virtual void FinishTaskExecution_Internal();
	virtual void InitializeTaskData_Internal(APlayerController* AskingPlayer, AActor* OwningActor);
	virtual bool IsTaskValid() const { return CachedPlayerData.IsValid() && CachedNPCData.IsValid(); }
	
	UFUNCTION(Client, WithValidation, Reliable)
	void Client_FinishTask();
	
	/**
	* Event fired when task is started
	* If we force a reply from NPC and this reply contains any task in the array, the task is then executed
	* Here can player define own logic purposes, like ask player if he take the quest from NPC, or do something (shake camera, start sequence, ...)
	* After task execution, we need call function FinishTaskExecution(), to finish task in a game
	* @Warning - NaturalDialogTask is executed on SERVER site and the task owning client
	* @param PlayerData - Stored Player external references, can be get using function GetPlayerData()
	* @param NPCData - Stored NPC external references, can be get using function GetNPCData()
	*/
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "On Task Executed"), Category="Natural dialog task")
	void ReceiveOnTaskExecuted(const FDialogTaskPlayerData& PlayerData, const FDialogTaskNPCData& NPCData);

	/**
	* Executed when task is finishing execution
	* Before task object is executed and destroyed, add some finish login here, for your game
	* @Warning - NaturalDialogTask is executed only on SERVER site, not on client site
	*/
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "On Task Finished"), Category="Natural dialog task")
	void ReceiveOnTaskFinished();

	UFUNCTION()
	void OnRep_ReplicatedOuter();
	
private:
	/** Stored external references for easy to use dialog task */
	UPROPERTY(Replicated)
	FDialogTaskPlayerData CachedPlayerData;

	/** Stored external references for easy to use dialog task */
	UPROPERTY(Replicated)
	FDialogTaskNPCData CachedNPCData;
	
	UPROPERTY(ReplicatedUsing=OnRep_ReplicatedOuter)
	UObject* ReplicatedOuter;
	
	/** True, if the task was executed on the PC (client or server) */
	uint8 bHasStarted : 1;

	/**
	 * True, if with the object is manipulated on server, otherwise false
	 * Set on object spawn with value true
	 * Client objects contain only default false value
	 */
	uint8 bHasAuthority : 1;

	/** If true, task is pending to kill and is already called event ReceiveOnTaskFinished() */
	uint8 bPendingFinish : 1;

	/**
	 * If the ReplicatedOuter is not set yet via replication and we want to finish task on client,
	 * we mark this object to pending kill and when replication comes and this is true,
	 * then is again called function FinishTaskExecution from client
	 */
	uint8 bIsWaitingToFinishOnClient : 1;	
};
