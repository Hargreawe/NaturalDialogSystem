// Created by Michal Chamula. All rights reserved.


#include "Core/NaturalDialogTask.h"
#include "Core/PlayerNaturalDialogComponent.h"
#include "Engine/NetDriver.h"
#include "Net/UnrealNetwork.h"

DEFINE_LOG_CATEGORY(LogNaturalDialogTask);

UNaturalDialogTask::UNaturalDialogTask()
{
	bHasStarted = false;
	bHasAuthority = false;
	bPendingFinish = false;
	bIsWaitingToFinishOnClient = false;
}

void UNaturalDialogTask::FinishTaskExecution()
{
	UPlayerNaturalDialogComponent* CastedReplicatedOuter = Cast<UPlayerNaturalDialogComponent>(ReplicatedOuter);
	if (ensure(CastedReplicatedOuter))
	{
		CastedReplicatedOuter->Server_FinishDialogTaskExecution(this);
	}
	else
	{
		bIsWaitingToFinishOnClient = true;
	}
}

UWorld* UNaturalDialogTask::GetWorld() const
{
	if (HasAllFlags(RF_ClassDefaultObject))
	{
		// If we are a CDO, we must return nullptr instead of calling Outer->GetWorld() to fool UObject::ImplementsGetWorld.
		return nullptr;
	}
	return GetOuter()->GetWorld();
}

int32 UNaturalDialogTask::GetFunctionCallspace(UFunction* Function, FFrame* Stack)
{
	if (HasAnyFlags(RF_ClassDefaultObject) || !IsSupportedForNetworking())
	{
		// This handles absorbing authority/cosmetic
		return GEngine->GetGlobalFunctionCallspace(Function, this, Stack);
	}
	check(GetOuter() != nullptr);
	return GetOuter()->GetFunctionCallspace(Function, Stack);
}

bool UNaturalDialogTask::CallRemoteFunction(UFunction* Function, void* Parameters, FOutParmRec* OutParams, FFrame* Stack)
{
	check(!HasAnyFlags(RF_ClassDefaultObject));
	check(GetOuter() != nullptr);

	bool bProcessed = false;

	if (const UPlayerNaturalDialogComponent* Owner = Cast<UPlayerNaturalDialogComponent>(GetOuter()))
	{
		AActor* ActorOwner = Owner->GetOwner();

		FWorldContext* const Context = GEngine->GetWorldContextFromWorld(GetWorld());
		if (Context != nullptr)
		{
			for (const FNamedNetDriver& Driver : Context->ActiveNetDrivers)
			{
				if (Driver.NetDriver != nullptr && Driver.NetDriver->ShouldReplicateFunction(ActorOwner, Function))
				{
					Driver.NetDriver->ProcessRemoteFunction(ActorOwner, Function, Parameters, OutParams, Stack, this);
					bProcessed = true;
				}
			}
		}
	}

	return bProcessed;
}

void UNaturalDialogTask::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	UObject::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UNaturalDialogTask, CachedPlayerData);
	DOREPLIFETIME(UNaturalDialogTask, CachedNPCData);
	DOREPLIFETIME(UNaturalDialogTask, ReplicatedOuter);
}

void UNaturalDialogTask::ExecuteTask(APlayerController* AskingPlayer, AActor* OwningActor)
{
	InitializeTaskData_Internal(AskingPlayer, OwningActor);

	if (IsTaskValid())
	{
		UE_LOG(LogNaturalDialogTask, Log, TEXT("Task references are valid, starting task execution"))
	}
	else
	{
		UE_LOG(LogNaturalDialogTask, Log, TEXT("Not all task references are valid, starting task execution without valid external values"));
	}

	StartTaskExecution_Internal();
}

void UNaturalDialogTask::StartTaskExecution_Internal()
{
	if (!bHasStarted)
	{
		bHasStarted = true;
		ReceiveOnTaskExecuted(CachedPlayerData, CachedNPCData);
	}
	else
	{
		UE_LOG(LogNaturalDialogTask, Warning, TEXT("Dialog task already executed"));
		// Has to be here return, because if we create and override child function, it has to stop here and do not execute other functionality
		return;
	}
}

void UNaturalDialogTask::FinishTaskExecution_Internal()
{
	// We do one more check if task is pending finish or not, because we dont want to send unnecessarily request on server 
	if (!bPendingFinish)
	{
		bPendingFinish = true;
		ReceiveOnTaskFinished();
	}
}

void UNaturalDialogTask::InitializeTaskData_Internal(APlayerController* AskingPlayer, AActor* OwningActor)
{
	CachedPlayerData = FDialogTaskPlayerData(AskingPlayer);
	CachedNPCData = FDialogTaskNPCData(OwningActor);

	if (!CachedPlayerData.IsValid())
	{
		UE_LOG(LogNaturalDialogTask, Error, TEXT("Player data are not valid, plase check if you are using valid player controller when asking for reply in NaturalDialogComponent class"));
	}

	if (!CachedNPCData.IsValid())
	{
		UE_LOG(LogNaturalDialogTask, Error, TEXT("NPC data are not valid, plase check if component owner actorcontains NaturalDialogComponent class"));
	}
}

void UNaturalDialogTask::OnRep_ReplicatedOuter()
{
	if(bIsWaitingToFinishOnClient)
	{
		FinishTaskExecution();
	}
}

void UNaturalDialogTask::Client_FinishTask_Implementation()
{
	FinishTaskExecution_Internal();
}

bool UNaturalDialogTask::Client_FinishTask_Validate()
{
	return true;
}
