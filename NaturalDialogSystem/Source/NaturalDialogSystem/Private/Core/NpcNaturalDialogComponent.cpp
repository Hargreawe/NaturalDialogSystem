// Created by Michal Chamula. All rights reserved.


#include "Core/NpcNaturalDialogComponent.h"
#include "Net/UnrealNetwork.h"


DEFINE_LOG_CATEGORY(LogNpcNaturalDialogComponent);

UNpcNaturalDialogComponent::UNpcNaturalDialogComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}
