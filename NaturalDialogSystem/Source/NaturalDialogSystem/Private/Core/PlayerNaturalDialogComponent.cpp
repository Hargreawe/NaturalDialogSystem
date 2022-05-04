// Created by Michal Chamula. All rights reserved.


#include "Core/PlayerNaturalDialogComponent.h"
#include "Core/NpcNaturalDialogComponent.h"
#include "DefaultClasses/DefaultDialogReplyFunction.h"
#include "DefaultClasses/DefaultReplyHelperFunction.h"
#include "Engine/ActorChannel.h"
#include "Net/UnrealNetwork.h"
#include "Resources/NaturalDialogSystemLibrary.h"

#define DEFAULT_REPLY NSLOCTEXT("Natural Dialog System", "Default Reply", "I don't understand you")

FDialogTableHierarchy::FDialogTableHierarchy(const UNpcNaturalDialogComponent* NpcNaturalDialogComponent)
	: OwnerNpcNaturalDialogComponent(NpcNaturalDialogComponent)
{
	if (NpcNaturalDialogComponent)
	{
		DialogTables.Reserve(NpcNaturalDialogComponent->GetInitialDialogTables().Num());

		for (UDataTable* DataTable : NpcNaturalDialogComponent->GetInitialDialogTables())
		{
			if (DataTable)
			{
				DialogTables.Add(DataTable);
			}
		}
	}
}


DEFINE_LOG_CATEGORY(LogPlayerNaturalDialogComponent);

UPlayerNaturalDialogComponent::UPlayerNaturalDialogComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	SetIsReplicatedByDefault(true);
	ReplyFunctionClass = UDefaultDialogReplyFunction::StaticClass();
	ReplyHelperFunctionClass = UDefaultReplyHelperFunction::StaticClass();
}

void UPlayerNaturalDialogComponent::BeginPlay()
{
	Super::BeginPlay();

	// Cache dictionary subsystem
	if (GetOwner() && GetOwner()->GetGameInstance())
	{
		DictSubsystem = GetOwner()->GetGameInstance()->GetSubsystem<UDictionarySubsystem>();
	}

	// Dialog reply function object instance construction
	CreateReplyObjectInstance(ReplyFunctionClass);
}

bool UPlayerNaturalDialogComponent::ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags)
{
	bool Result = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);
	Result |= Channel->ReplicateSubobjectList(ActiveExecutionTasks, *Bunch, *RepFlags);

	return Result;
}

void UPlayerNaturalDialogComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// Other clients doesn't need know about data of another player
	DOREPLIFETIME_CONDITION(UPlayerNaturalDialogComponent, DialogData, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(UPlayerNaturalDialogComponent, ActiveExecutionTasks, COND_OwnerOnly);
}

void UPlayerNaturalDialogComponent::CreateReplyObjectInstance(const TSubclassOf<UDialogReplyFunction> InReplyFunctionClass)
{
	const TSubclassOf<UDialogReplyFunction> UsedReplyFunctionClass = InReplyFunctionClass ? InReplyFunctionClass : UDefaultDialogReplyFunction::StaticClass();
	const TSubclassOf<UReplyHelperFunction> HelperFunctionClass = ReplyHelperFunctionClass ? ReplyHelperFunctionClass : UDefaultReplyHelperFunction::StaticClass();

#if !UE_BUILD_SHIPPING

	if (InReplyFunctionClass)
	{
		UE_LOG(LogPlayerNaturalDialogComponent, Log, TEXT("%s used for reply function"), *InReplyFunctionClass->GetClass()->GetName());
		UE_LOG(LogPlayerNaturalDialogComponent, Log, TEXT("%s used for dialog help function"), *HelperFunctionClass->GetClass()->GetName());
	}
	else
	{
		UE_LOG(LogPlayerNaturalDialogComponent, Warning, TEXT("%s used for reply function as default"), *UDefaultDialogReplyFunction::StaticClass()->GetClass()->GetName());
		UE_LOG(LogPlayerNaturalDialogComponent, Warning, TEXT("%s used for dialog help function as default"), *UDefaultReplyHelperFunction::StaticClass()->GetClass()->GetName());
	}

#endif

	// Create reply function
	ReplyFunctionInstance = NewObject<UDialogReplyFunction>(this, UsedReplyFunctionClass);
	ReplyFunctionInstance->InitializeDialogReplyPicker();

	// Check override of abstract method
	FNaturalDialogResult AnswerResult;
	ReplyFunctionInstance->GenerateReply(FString(), nullptr, AnswerResult);

	// Create helper function
	DialogHelperFunction = NewObject<UReplyHelperFunction>(this, HelperFunctionClass);
	DialogHelperFunction->InitializeDialogReplyHelper();

	// Check override of abstract methods
	TSet<FString> OutOptions;
	FText OutOption;
	DialogHelperFunction->FindAskOptions(FText(), nullptr, OutOptions);
	DialogHelperFunction->FindBestOption(FText(), nullptr, OutOption);
}

void UPlayerNaturalDialogComponent::RegisterInitialTables(const UNpcNaturalDialogComponent* NpcNaturalDialogComponent)
{
	// Validate if we already met the NPC character, if not, then initialize startup tables
	if (NpcNaturalDialogComponent && NpcNaturalDialogComponent->HasInitialDialogTables() && !DialogData.Contains(NpcNaturalDialogComponent))
	{
		for (UDataTable* InitTable : NpcNaturalDialogComponent->GetInitialDialogTables())
		{
			if (InitTable)
			{
				RegisterDialogData(NpcNaturalDialogComponent, InitTable);
				break;
			}
		}

		OnNewNpcInteract.Broadcast(FDialogTaskNPCData(NpcNaturalDialogComponent->GetOwner()));
	}
}

TArray<FNaturalDialogAnswer> UPlayerNaturalDialogComponent::GenerateDialogReply(const FText& Input, const UNpcNaturalDialogComponent* NpcNaturalDialogComponent)
{
	TArray<FNaturalDialogAnswer> Result;
	Result.Add(DEFAULT_REPLY);

	if (DictSubsystem.IsValid() && ensure(GetOwnerPlayerController()))
	{
		// Validate function instance, if is not valid yet, we create new one
		if (!ReplyFunctionInstance)
		{
			UE_LOG(LogPlayerNaturalDialogComponent, Warning, TEXT("Reply instance is not valid yet, creating new one"));
			CreateReplyObjectInstance(ReplyFunctionClass);
		}

		// Validate if we already met the NPC character, if not, then initialize startup tables
		RegisterInitialTables(NpcNaturalDialogComponent);

		const TArray<FString> SplitInput = UNaturalDialogSystemLibrary::SplitToSentences(Input.ToString());
		Result.Reset(SplitInput.Num());

		for (const FString& Sentence : SplitInput)
		{
			// If the correct answer is not found, then override the result and end generating reply
			FNaturalDialogResult AnswerResult;
			const bool bWasReplyFound = ReplyFunctionInstance->GenerateReply(Sentence, NpcNaturalDialogComponent, AnswerResult);

			if (bWasReplyFound && ensureMsgf(AnswerResult.Table && !AnswerResult.RowName.IsNone(), TEXT("Result table is none or row_name was not found")))
			{
				FNaturalDialogRow* Row = nullptr;
				FNaturalDialogRow_Base* RowBase = nullptr;

				if(AnswerResult.Table->GetRowStruct()->IsChildOf(FNaturalDialogRow_Base::StaticStruct()))
				{
					RowBase = AnswerResult.Table->FindRow<FNaturalDialogRow_Base>(AnswerResult.RowName, nullptr);
				}
				
				if (AnswerResult.Table->GetRowStruct()->IsChildOf(FNaturalDialogRow::StaticStruct()))
				{
					Row = AnswerResult.Table->FindRow<FNaturalDialogRow>(AnswerResult.RowName, nullptr);
				}

				if (Row)
				{
					// If the answer has execution tasks we fire them
					for (TSoftClassPtr<UNaturalDialogTask> Task : Row->DialogTasks)
					{
						if(Task)
						{
							const TSubclassOf<UNaturalDialogTask> LoadedClass(Task.LoadSynchronous()); // #todo .. maybe not loaded on server yet
							Server_ExecuteDialogTask(NpcNaturalDialogComponent, LoadedClass);
						}
					}

					// Register row new dialog data
					for(const FNaturalDialogTableAction& DataTable : Row->DialogTables)
					{
						if(DataTable.DialogTable)
						{
							if(DataTable.Action == EDialogTableAction::Add)
							{
								RegisterDialogData(NpcNaturalDialogComponent, DataTable.DialogTable);
							}
							else
							{
								UnregisterDialogData(NpcNaturalDialogComponent, DataTable.DialogTable);
							}
						}
					}
				}
				
				if (RowBase && ensureMsgf(RowBase->Answer.IsValidIndex(AnswerResult.AnswerIndex), TEXT("Row index %d not found for table %s with row %s"), AnswerResult.AnswerIndex, *AnswerResult.Table->GetName(), *AnswerResult.RowName.ToString()))
				{
					Result.Add(RowBase->Answer[AnswerResult.AnswerIndex]);
				}
			}
			else if(AnswerResult.Table && !AnswerResult.RowName.IsNone())
			{
				FNaturalDialogRow_Base* RowBase = nullptr;

				if(AnswerResult.Table->GetRowStruct()->IsChildOf(FNaturalDialogRow_Base::StaticStruct()))
				{
					RowBase = AnswerResult.Table->FindRow<FNaturalDialogRow_Base>(AnswerResult.RowName, nullptr);
					Result.Add(RowBase->Answer[AnswerResult.AnswerIndex]);
				}
			}
		}

		// Check the result size, if is empty, we have to set invalid response as result
		if (!Result.IsValidIndex(0))
		{
			Result.Add(DEFAULT_REPLY);
		}
	}

	return Result;
}

bool UPlayerNaturalDialogComponent::FindBestAskOptions(const FText& Input, const UNpcNaturalDialogComponent* NpcNaturalDialogComponent, TSet<FString>& Options) const
{
	bool Result = false;

	if (DialogHelperFunction)
	{
		Result = DialogHelperFunction->FindAskOptions(Input, NpcNaturalDialogComponent, Options);
	}

	return Result;
}

bool UPlayerNaturalDialogComponent::FindBestAskOption(const FText& Input, const UNpcNaturalDialogComponent* NpcNaturalDialogComponent, FText& Option) const
{
	bool Result = false;

	if (DialogHelperFunction)
	{
		Result = DialogHelperFunction->FindBestOption(Input, NpcNaturalDialogComponent, Option);
	}

	return Result;
}

void UPlayerNaturalDialogComponent::RegisterDialogData(const UNpcNaturalDialogComponent* NpcNaturalDialogComponent, UDataTable* DialogTable)
{
	if (DialogTable && DialogTable->GetRowStruct()->IsChildOf(FNaturalDialogRow::StaticStruct()))
	{
		// Do not invert these steps, if we set first server then client, we can override potential replication from server step

		// Step 1. -> Set dialog data for client
		if (!GetOwner()->HasAuthority())
		{
			RegisterDialogTable_Internal(NpcNaturalDialogComponent, DialogTable);
		}

		// Step 2. -> Set dialog data on server, then we can wait for data replication.
		Server_RegisterDialogData(NpcNaturalDialogComponent, DialogTable);
	}
	else
	{
		UE_LOG(LogPlayerNaturalDialogComponent, Error, TEXT("Not valid dialog table i"));
	}
}

void UPlayerNaturalDialogComponent::UnregisterDialogData(const UNpcNaturalDialogComponent* NpcNaturalDialogComponent, UDataTable* DialogTable)
{
	// Step 1. -> Set dialog data for client
	if (!GetOwner()->HasAuthority())
	{
		UnregisterDialogTable_Internal(NpcNaturalDialogComponent, DialogTable);
	}

	// Step 2. -> Set dialog data on server, then we can wait for data replication.
	Server_UnregisterDialogData(NpcNaturalDialogComponent, DialogTable);
}

TSet<UDataTable*> UPlayerNaturalDialogComponent::GetDialogTables(const UNpcNaturalDialogComponent* NpcNaturalDialogComponent) const
{
	TSet<UDataTable*> Result;

	if (NpcNaturalDialogComponent)
	{
		const int32 Index = DialogData.Find(NpcNaturalDialogComponent);
		if (DialogData.IsValidIndex(Index))
		{
			Result = TSet<UDataTable*>(DialogData[Index].DialogTables);
		}
	}

	return Result;
}

bool UPlayerNaturalDialogComponent::HasDialogDataRegistered(const UNpcNaturalDialogComponent* NpcNaturalDialogComponent, UDataTable* DialogTable) const
{
	bool Result = false;

	if (NpcNaturalDialogComponent && DialogTable)
	{
		const int32 ElementIndex = DialogData.Find(NpcNaturalDialogComponent);
		if (ElementIndex != INDEX_NONE)
		{
			Result = DialogData[ElementIndex].DialogTables.Contains(DialogTable);
		}
	}

	return Result;
}

TSet<const UDataTable*> UPlayerNaturalDialogComponent::GetDialogTables_Const(const UNpcNaturalDialogComponent* NpcNaturalDialogComponent) const
{
	TSet<const UDataTable*> Result;

	if (NpcNaturalDialogComponent)
	{
		const int32 Index = DialogData.Find(NpcNaturalDialogComponent);
		if (DialogData.IsValidIndex(Index))
		{
			Result.Reserve(DialogData[Index].DialogTables.Num());

			for (const UDataTable* Table : DialogData[Index].DialogTables)
			{
				Result.Add(Table);
			}
		}
	}

	return Result;
}

void UPlayerNaturalDialogComponent::Server_RegisterDialogData_Implementation(const UNpcNaturalDialogComponent* NpcNaturalDialogComponent, UDataTable* DialogTable)
{
	if (NpcNaturalDialogComponent && DialogTable && DialogTable->GetRowStruct()->IsChildOf(FNaturalDialogRow::StaticStruct()))
	{
		RegisterDialogTable_Internal(NpcNaturalDialogComponent, DialogTable);
	}
	else
	{
		UE_LOG(LogPlayerNaturalDialogComponent, Warning, TEXT("Trying to register invalid data asset or data asset with invalid row structure"));
	}
}

bool UPlayerNaturalDialogComponent::Server_RegisterDialogData_Validate(const UNpcNaturalDialogComponent* NpcNaturalDialogComponent, UDataTable* DialogTable)
{
	return true;
}

void UPlayerNaturalDialogComponent::Server_UnregisterDialogData_Implementation(const UNpcNaturalDialogComponent* NpcNaturalDialogComponent, UDataTable* DialogTable)
{
	UnregisterDialogTable_Internal(NpcNaturalDialogComponent, DialogTable);
}

bool UPlayerNaturalDialogComponent::Server_UnregisterDialogData_Validate(const UNpcNaturalDialogComponent* NpcNaturalDialogComponent, UDataTable* DialogTable)
{
	return true;
}


void UPlayerNaturalDialogComponent::Server_ExecuteDialogTask_Implementation(const UNpcNaturalDialogComponent* NpcNaturalDialogComponent, TSubclassOf<UNaturalDialogTask> Task)
{
	if (!NpcNaturalDialogComponent)
	{
		UE_LOG(LogPlayerNaturalDialogComponent, Error, TEXT("Input NPC component object is invalid"));
		return;
	}

	if (!Task)
	{
		UE_LOG(LogPlayerNaturalDialogComponent, Error, TEXT("Task object is invalid"));
		return;
	}
	
	APlayerController* Controller = GetOwnerPlayerController();
	if (!Controller)
	{
		UE_LOG(LogPlayerNaturalDialogComponent, Error, TEXT("Invalid player controller for task execution"));
		return;
	}
	
	if(!HasValidDialogTask(NpcNaturalDialogComponent, TSoftClassPtr<UNaturalDialogTask>(Task)))
	{
		UE_LOG(LogPlayerNaturalDialogComponent, Error, TEXT("Server dialog data doesn't contain any dialog task of class %s"), *Task->GetName());
		return;
	}

#if !UE_BUILD_SHIPPING

	const TCHAR* TaskName = *Task->GetClass()->GetName();
	if (GetOwner())
	{
		UE_LOG(LogPlayerNaturalDialogComponent, Log, TEXT("New task %s start execution for owner actor %s"), TaskName, *GetOwner()->GetClass()->GetName());
	}
	else
	{
		UE_LOG(LogPlayerNaturalDialogComponent, Log, TEXT("New task object %s start execution"), TaskName);
	}

#endif
	
	UNaturalDialogTask* TaskInstance = NewObject<UNaturalDialogTask>(this, Task);
	TaskInstance->bHasAuthority = true;
	TaskInstance->ReplicatedOuter = this;

	ActiveExecutionTasks.Add(TaskInstance);
	TaskInstance->ExecuteTask(Controller, NpcNaturalDialogComponent->GetOwner());
}

bool UPlayerNaturalDialogComponent::Server_ExecuteDialogTask_Validate(const UNpcNaturalDialogComponent* NpcNaturalDialogComponent, TSubclassOf<UNaturalDialogTask> Task)
{
	return true;
}

void UPlayerNaturalDialogComponent::Server_FinishDialogTaskExecution_Implementation(UNaturalDialogTask* Task)
{
	if (!Task)
	{
		UE_LOG(LogPlayerNaturalDialogComponent, Error, TEXT("Input param Task is invalid for finalize task object"));
		return;
	}

#if !UE_BUILD_SHIPPING

	const TCHAR* TaskName = *Task->GetClass()->GetName();
	if (GetOwner())
	{
		UE_LOG(LogPlayerNaturalDialogComponent, Log, TEXT("Task object %s was sucessfully executed for owner actor %s"), TaskName, *GetOwner()->GetClass()->GetName());
	}
	else
	{
		UE_LOG(LogPlayerNaturalDialogComponent, Log, TEXT("Task object %s was sucessfully executed"), TaskName);
	}

#endif

	// Finish task
	Task->FinishTaskExecution_Internal(); // Step 1. -> Finish task on server
	Task->Client_FinishTask(); // Step 2. -> Finish task on client

	// Step 3. -> Task finalization, we are trying to shift deleting the object, because we want be sure the client task finish was called
	if(GetWorld())
	{
		FTimerHandle InHandle;
		GetWorld()->GetTimerManager().SetTimer(InHandle, FTimerDelegate::CreateWeakLambda(this, [=]()
		{
			ActiveExecutionTasks.Remove(Task);
		}), 3.f, false);
	}
	else
	{
		ActiveExecutionTasks.Remove(Task);
	}
}

bool UPlayerNaturalDialogComponent::Server_FinishDialogTaskExecution_Validate(UNaturalDialogTask* Task)
{
	return true;
}

void UPlayerNaturalDialogComponent::OnRep_ActiveExecutionTasks()
{
	for (UNaturalDialogTask* Task : ActiveExecutionTasks)
	{
		// Validate if the task has not already started
		if (!Task->bHasStarted)
		{
			// Player and NPC data should be set from server execution
			// We need only execute the task on client site
			Task->StartTaskExecution_Internal();
		}
	}
}

void UPlayerNaturalDialogComponent::RegisterDialogTable_Internal(const UNpcNaturalDialogComponent* NpcNaturalDialogComponent, UDataTable* DialogTable)
{
	if (NpcNaturalDialogComponent && DialogTable)
	{
		// First we initialize array element, with initial tables of NpcNaturalDialogComponent
		if (!DialogData.Contains(NpcNaturalDialogComponent))
		{
			DialogData.Add(NpcNaturalDialogComponent);
			for (const UDataTable* InitTable : NpcNaturalDialogComponent->GetInitialDialogTables())
			{
				if (InitTable)
				{
					OnDataTableAdded.Broadcast(InitTable);
				}
			}
			UE_LOG(LogPlayerNaturalDialogComponent, Log, TEXT("New NPC %s registered in PlayerNaturalDialogComponent"), *NpcNaturalDialogComponent->GetOwner()->GetName());
		}

		// Now we check if table already exists in dialog data of NPC
		const int32 ElementIndex = DialogData.Find(NpcNaturalDialogComponent);
		TArray<UDataTable*>& Data = DialogData[ElementIndex].DialogTables;

		if (!Data.Contains(DialogTable))
		{
			UE_LOG(LogPlayerNaturalDialogComponent, Log, TEXT("New data asset registered (%s)"), *DialogTable->GetName());

			Data.Add(DialogTable);
			OnDataTableAdded.Broadcast(DialogTable);
		}
		else
		{
			UE_LOG(LogPlayerNaturalDialogComponent, Log, TEXT("Trying to add new data asset, but already exists in dialog component"));
		}
	}
	else
	{
		UE_LOG(LogPlayerNaturalDialogComponent, Error, TEXT("Invalid input params in function RegisterDialogTable_Internal()"));
	}
}

void UPlayerNaturalDialogComponent::UnregisterDialogTable_Internal(const UNpcNaturalDialogComponent* NpcNaturalDialogComponent, UDataTable* DialogTable)
{
	if (NpcNaturalDialogComponent && DialogTable)
	{
		const int32 Index = DialogData.Find(NpcNaturalDialogComponent);

		if (DialogData.IsValidIndex(Index))
		{
			UE_LOG(LogPlayerNaturalDialogComponent, Log, TEXT("Removing dialog table (%s)"), *DialogTable->GetName());

			DialogData[Index].DialogTables.Remove(DialogTable);
			OnDataTableRemoved.Broadcast(DialogTable);
		}
		else
		{
			UE_LOG(LogPlayerNaturalDialogComponent, Warning, TEXT("Component doesn't contain dialog table (%s) for Npc dialog component"), *DialogTable->GetName());
		}
	}
	else
	{
		UE_LOG(LogPlayerNaturalDialogComponent, Error, TEXT("Invalid input params in function UnregisterDialogTable_Internal()"));
	}
}

APlayerController* UPlayerNaturalDialogComponent::GetOwnerPlayerController() const
{
	APlayerController* Result = nullptr;

	const APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (OwnerPawn)
	{
		Result = Cast<APlayerController>(OwnerPawn->GetController());
	}

#if !UE_BUILD_SHIPPING

	// Validate player controller
	if (!Result)
	{
		UE_LOG(LogPlayerNaturalDialogComponent, Error, TEXT("Player natural dialog component is attached in player not controlled pawn (%s), all requests for replies will be drop"), *GetOwner()->GetClass()->GetName());
	}

#endif

	return Result;
}

bool UPlayerNaturalDialogComponent::HasValidDialogTask(const UNpcNaturalDialogComponent* NpcNaturalDialogComponent, TSoftClassPtr<UNaturalDialogTask> Task) const
{
	bool Result = false;
	for(const UDataTable* InDataTable : GetDialogTables_Const(NpcNaturalDialogComponent))
	{
		for(const FName RowName : InDataTable->GetRowNames())
		{
			const FNaturalDialogRow* Row = InDataTable->FindRow<FNaturalDialogRow>(RowName, "Searching for row data");
			if(Row && Row->DialogTasks.Contains(Task))
			{
				Result = true;
				break;
			}
		}

		if(Result)
		{
			break;
		}
	}

	return Result;
}
