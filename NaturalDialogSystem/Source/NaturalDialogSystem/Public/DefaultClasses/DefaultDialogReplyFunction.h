// Created by Michal Chamula. All rights reserved.

#pragma once

#include "CoreMinimal.h"

#include "Core/DictionarySubsystem.h"
#include "FunctionalClasses/DialogReplyFunction.h"
#include "FunctionalClasses/StringDistanceFunction.h"
#include "Resources/Resources.h"
#include "DefaultDialogReplyFunction.generated.h"

class UPlayerNaturalDialogComponent;
class UNpcNaturalDialogComponent;


DECLARE_LOG_CATEGORY_EXTERN(Log_DefaultDialogReplyFunction, Log, All);

/**
 * 
 */
UCLASS()
class NATURALDIALOGSYSTEM_API UDefaultDialogReplyFunction : public UDialogReplyFunction
{
	GENERATED_BODY()

public:
	UDefaultDialogReplyFunction();
	
protected:
	/**
	 * Defines interval of calling function HandleMatrixWeariness()
	 * If interval is 1, then every second will metric weariness value is increased
	 * Weariness value is not increased for every answer in data tables (it's random)
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category= "Properties")
	float MetricInterval;
	
	/**
	* Curve is used for calculate metric values for reply function
	* Time (horizontal) value tels current metric value and vertical the new target value
	* If is not set, then is used basic algorithm to calculate new metric value
	*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category= "Properties")
	UCurveFloat* MetricCurve;

	UPROPERTY()
	TSubclassOf<UStringDistanceFunction> StringDistanceFunctionClass;
	
	/**
	* All invalid responses, when NPC doesnt recognize the answer
	* If not set, is used default reply text, without answer wave
	*/
	UPROPERTY(EditDefaultsOnly, meta=(RequiredAssetDataTags = "RowStructure=NaturalDialogRow_Base"), Category="Natural Dialog Component")
	UDataTable* DefaultResponses;

public:
	virtual void InitializeDialogReplyPicker() override;
	virtual bool GenerateReply(const FString& Sentence, const UNpcNaturalDialogComponent* NpcNaturalDialogComponent, FNaturalDialogResult& ResultAnswer) override;

	virtual TSet<const UDataTable*> FindBestTableSet(const UNpcNaturalDialogComponent* NpcNaturalDialogComponent, const TArray<FString>& Keywords) const;

protected:
	/** Handle case when input table is registered as new data table in owner component */
	UFUNCTION()
	void HandleNewTableRegistration(const UDataTable* NewTable);

	/** Handle case when input table is removed from owner component */
	UFUNCTION()
	void HandleRegisteredTableRemoved(const UDataTable* NewTable);

	UFUNCTION()
	void HandleMatrixWeariness();
	
private:
	FReplyData FindBestReply(const TArray<FReplyData>& AllReplies) const;
	void ModifyMetricValue(const UDataTable* InTable, const FName InRow, const int32 AnswerIndex);
	void LogMetricValues();
	
	// template<typename T>
	// static TArray<TSet<T*>> FindCombination(const TSet<T*>& InArray, const int32 CombinationSize);
	
	TArray<TSet<const UDataTable*>> CombinationUtil(const TArray<FString>& InArray, TArray<FString> Data, const int32 Start, const int32 End, const int32 Index, const int32 CombinationSize) const;

private:
	/**
	 * Initialized in function InitializeDialogReplyPicker()
	 * Strong ref in game instance
	 */
	TWeakObjectPtr<UDictionarySubsystem> DictionarySubsystem;

	/**
	 * Initialized in function InitializeDialogReplyPicker()
	 * Strong ref in game instance
	 */
	UPROPERTY()
	UStringDistanceFunction* StringDistanceFunction;
	
	/**
	 * Initialized in function InitializeDialogReplyPicker()
	 * Strong ref is in UDictionarySubsystem
	 */
	TWeakObjectPtr<const UDictionaryRepresentation> DictionaryRepresentation;

	TWeakObjectPtr<UPlayerNaturalDialogComponent> OwnerComponent;

	/**
	 * Metric is used to compute variations for NPC reply
	 * Every data table and row is stored metric value which is decreased when the answer of the row is used
	 * When we find correct reply of two answers, we use the one with greatest metric value
	 */
	FDialogMetric Metric;
	
	FTimerHandle MatrixWearinessHandler;
};
