// Created by Michal Chamula. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FunctionalClasses/ReplyHelperFunction.h"
#include "Resources/Resources.h"
#include "PlayerNaturalDialogComponent.generated.h"

class UReplyHelperFunction;
class UDictionarySubsystem;
class UDialogReplyFunction;
class UNaturalDialogTask;
class UNpcNaturalDialogComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDataTableChanged, const UDataTable*, DataTable);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNpcStateChanged, const FDialogTaskNPCData&, NpcData);

/**
 *
 */
USTRUCT(BlueprintType)
struct FDialogTableHierarchy
{
	GENERATED_BODY()

	FDialogTableHierarchy()
		: OwnerNpcNaturalDialogComponent(nullptr) {}

	FDialogTableHierarchy(const UNpcNaturalDialogComponent* NpcNaturalDialogComponent);

	bool operator==(const FDialogTableHierarchy& Other) const { return OwnerNpcNaturalDialogComponent == Other.OwnerNpcNaturalDialogComponent; }
	bool operator==(const UNpcNaturalDialogComponent* Other) const { return OwnerNpcNaturalDialogComponent == Other; };

	/**
	 * Actor that owns dialog tables from variable DialogTables
	 */
	UPROPERTY(BlueprintReadOnly)
	const UNpcNaturalDialogComponent* OwnerNpcNaturalDialogComponent;

	/**
	 * Held tables of the actor, which contains NpcNaturalDialogComponent
	 */
	UPROPERTY(BlueprintReadOnly)
	TArray<UDataTable*> DialogTables;
};


DECLARE_LOG_CATEGORY_EXTERN(LogPlayerNaturalDialogComponent, Log, All);

/**
 * Player natural dialog component handle all replies asked from player to NPC
 * Is closely related to UNpcNaturalDialogComponent, where are store initial tables for player
 * When player talks to new NPC, all initial tables are copied to this component into DialogData, which are replicated over network
 * Component generate reply using function GenerateDialogReply() and spawn potential dialog task, which are associated with dialog reply
 * These tasks are replicates over network and are executed on client and server site
 */
UCLASS(Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class NATURALDIALOGSYSTEM_API UPlayerNaturalDialogComponent : public UActorComponent
{
	GENERATED_BODY()

	friend class UNaturalDialogTask;

public:
	UPlayerNaturalDialogComponent();

protected:
	/**
	* Function used for reply for input from function GenerateDialogReply()
	* Constructed on BeginPlay()
	*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Natural Dialog Component")
	TSubclassOf<UDialogReplyFunction> ReplyFunctionClass;

	/**
	* Function used for help player to ask  for replies
	* Constructed on BeginPlay()
	*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Natural Dialog Component")
	TSubclassOf<UReplyHelperFunction> ReplyHelperFunctionClass;

public:
	/**
	* Fired, when new dialog data table is added to component
	* Fires all initial data tables, set in BP child details panel
	* Fired on server and client side
	*/
	UPROPERTY(BlueprintAssignable)
	FOnDataTableChanged OnDataTableAdded;

	/**
	* Fired, when existing dialog data table is removed from component
	* Fired on server and client side
	*/
	UPROPERTY(BlueprintAssignable)
	FOnDataTableChanged OnDataTableRemoved;

	/**
	 * Fired, when player interact with npc first time
	 * Fired on server and client side with prediction
	 */
	UPROPERTY(BlueprintAssignable)
	FOnNpcStateChanged OnNewNpcInteract;
	
protected:
	/** Initialize component properties (DictSubsystem and ReplyFunctionInstance) */
	virtual void BeginPlay() override;

	/** Allows a component to replicate other sub-object on the actor  */
	virtual bool ReplicateSubobjects(class UActorChannel* Channel, class FOutBunch* Bunch, FReplicationFlags* RepFlags) override;

	/** Replication props of component */
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** Constructs reply object instance */
	virtual void CreateReplyObjectInstance(const TSubclassOf<UDialogReplyFunction> InReplyFunctionClass);

public:
	/**
	 * Function registers all initial tables from given NpcNaturalDialogComponent
	 * All tables are registered automatically when player ask for first reply
	 * but tables are registered on server site and because of replication delay,
	 * there is a chance to get invalid response, because data are not prepared yet on client site
	 * We recommend to use this function before start asking a reply for component
	 * No checks are needed for this
	 * @param NpcNaturalDialogComponent - Initial data tables are taken from this NPC component
	 */
	UFUNCTION(BlueprintCallable, Category="Natural Dialog Component")
	void RegisterInitialTables(const UNpcNaturalDialogComponent* NpcNaturalDialogComponent);

	/**
	* Main function for generating reply on player dialog input
	* Used to generate dialog with the controller owner (NPC) and player
	* @param Input - Player text input (e.g. from UI input text block)
	* @param NpcNaturalDialogComponent - Defines component of npc which we asking for the reply
	* @return - NPC dialog responses to player, we can show it in UI
	*/
	UFUNCTION(BlueprintCallable, Category="Natural Dialog Component")
	TArray<FNaturalDialogAnswer> GenerateDialogReply(const FText& Input, const UNpcNaturalDialogComponent* NpcNaturalDialogComponent);

	/**
	 * Function helps player to ask question from NPC
	 * Uses DialogHelperFunction to generate possible options with using of input
	 * @param Input - Text, which player has already typed
	 * @param NpcNaturalDialogComponent - Component which we are asking for
	 * @param Options - Found options, which can be used
	 * @return - True, if we have found any possible option
	 */
	UFUNCTION(BlueprintCallable, Category= "Natural Dialog Component")
	bool FindBestAskOptions(const FText& Input, const UNpcNaturalDialogComponent* NpcNaturalDialogComponent, TSet<FString>& Options) const;

	/**
	 * Function helps player to ask question from NPC
	 * Uses DialogHelperFunction to generate possible option with using of input
	 * @param Input - Text, which player has already typed
	 * @param NpcNaturalDialogComponent - Component which we are asking for
	 * @param Option - Found option, which can be used
	 * @return - True, if we have found any possible option
	 */
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Find Best Ask Option (Single)"), Category= "Natural Dialog Component")
	bool FindBestAskOption(const FText& Input, const UNpcNaturalDialogComponent* NpcNaturalDialogComponent, FText& Option) const;

	// UFUNCTION(BlueprintCallable, Category= "Natural Dialog Component")
	// void CommitLastUsedOption(const FText&Option);

	/**
	* Register new data table asset for NPC
	* Allowed only data tables with row FNaturalDialogRow
	* We can do this if we need register more information during the game
	* Function does server request and client data adjustment, @see Server_RegisterDialogData() function for more info
	* e.g. when we complete som quest and we need to add new quest info to NPC
	* @param NpcNaturalDialogComponent - Npc which we are registering dialog data for
	* @param DialogTable - Data for NPC registration
	*/
	UFUNCTION(BlueprintCallable, Category="Natural Dialog Component")
	void RegisterDialogData(const UNpcNaturalDialogComponent* NpcNaturalDialogComponent, UDataTable* DialogTable);

	/**
	* Remove registered data for the owner NPC if exists
	* The data could be used for completed quest, when we dont need them anymore
	* @param NpcNaturalDialogComponent - Npc which we are unregistering dialog data for
	* @param DialogTable - Data which we want to remove from owner NPC dialog data tables 
	*/
	UFUNCTION(BlueprintCallable, Category="Natural Dialog Component")
	void UnregisterDialogData(const UNpcNaturalDialogComponent* NpcNaturalDialogComponent, UDataTable* DialogTable);

	/** Returns all available dialog tables for npc communication */
	UFUNCTION(BlueprintCallable, Category="Natural Dialog Component")
	TSet<UDataTable*> GetDialogTables(const UNpcNaturalDialogComponent* NpcNaturalDialogComponent) const;

	/** Returns True, if player has already registered dialog table */
	UFUNCTION(BlueprintCallable, Category="Natural Dialog Component")
	bool HasDialogDataRegistered(const UNpcNaturalDialogComponent* NpcNaturalDialogComponent, UDataTable* DialogTable) const;

	/** Returns all available const dialog tables for npc communication */
	TSet<const UDataTable*> GetDialogTables_Const(const UNpcNaturalDialogComponent* NpcNaturalDialogComponent) const;

protected:
	/**
	 * We have to divided request for dialog table registration
	 * When we asking for reply, UPlayerNaturalDialogComponent has not yet replicated DialogData, so it cannot reply dialog
	 * Because of this, we have server function Server_RegisterDialogData(), which sends request for table registration
	 * But with this registration, we add temp dialog data into the component
	 */
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_RegisterDialogData(const UNpcNaturalDialogComponent* NpcNaturalDialogComponent, UDataTable* DialogTable);

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_UnregisterDialogData(const UNpcNaturalDialogComponent* NpcNaturalDialogComponent, UDataTable* DialogTable);

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_ExecuteDialogTask(const UNpcNaturalDialogComponent* NpcNaturalDialogComponent, TSubclassOf<UNaturalDialogTask> Task);

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_FinishDialogTaskExecution(UNaturalDialogTask* Task);

	UFUNCTION()
	void OnRep_ActiveExecutionTasks();

private:
	void RegisterDialogTable_Internal(const UNpcNaturalDialogComponent* NpcNaturalDialogComponent, UDataTable* DialogTable);
	void UnregisterDialogTable_Internal(const UNpcNaturalDialogComponent* NpcNaturalDialogComponent, UDataTable* DialogTable);
	APlayerController* GetOwnerPlayerController() const;
	bool HasValidDialogTask(const UNpcNaturalDialogComponent* NpcNaturalDialogComponent, TSoftClassPtr<UNaturalDialogTask> Task) const;

	/**
	 * Cached dialog data for dialog replies
	 * When player interact with UNpcNaturalDialogComponent, all initial tables are cached into this component
	 * We can add or remove dialog tables data using server functions Server_RegisterDialogData and Server_RemoveRegisteredDialogData
	 * Struct holds reference to the owner of the component and its tables
	 */
	UPROPERTY(Replicated)
	TArray<FDialogTableHierarchy> DialogData;

	/**
	* All active tasks, executed by interaction with cached dialog tables
	* After server replication to client, we execute these tasks on the owner client too in replication function OnRep_ActiveExecutionTasks
	*/
	UPROPERTY(ReplicatedUsing="OnRep_ActiveExecutionTasks")
	TArray<UNaturalDialogTask*> ActiveExecutionTasks;

	/**
	* Cached dictionary subsystem
	* Strong ref is in game instance
	* Is set in BeginPlay() function
	*/
	TWeakObjectPtr<UDictionarySubsystem> DictSubsystem;

	/**
	* Cached reply function instance
	* Created in BeginPlay() function
	*/
	UPROPERTY()
	UDialogReplyFunction* ReplyFunctionInstance;

	/**
	 * Cached helper reply function instance
	 * Created in BeginPlay() function
	 */
	UPROPERTY()
	UReplyHelperFunction* DialogHelperFunction;
};
