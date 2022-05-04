// Created by Michal Chamula. All rights reserved.

#include "DefaultClasses/LevenshteinDistanceFunction.h"
#include <vector>

uint32 ULevenshteinDistanceFunction::GetStringDistance(const FString& InputA, const FString& InputB) const
{
	const uint32 MinSize = InputA.Len();
	const uint32 MaxSize = InputB.Len();
	
	if (MinSize > MaxSize)
	{
		return GetStringDistance(InputB, InputA);
	}
	
	std::vector<uint32> Lev_Dist(MinSize + 1);
	
	for (uint32 i = 0; i <= MinSize; ++i)
	{
		Lev_Dist[i] = i;
	}
	
	for (uint32 j = 1; j <= MaxSize; ++j)
	{
		uint32 PreviousDiagonal = Lev_Dist[0];
		++Lev_Dist[0];
	
		for (uint32 i = 1; i <= MinSize; ++i)
		{
			const uint32 PreviousDiagonalSave = Lev_Dist[i];
			if (InputA[i - 1] == InputB[j - 1])
			{
				Lev_Dist[i] = PreviousDiagonal;
			}
			else
			{
				Lev_Dist[i] = std::min(std::min(Lev_Dist[i - 1], Lev_Dist[i]), PreviousDiagonal) + 1;
			}
	
			PreviousDiagonal = PreviousDiagonalSave;
		}
	}
	
	return Lev_Dist[MinSize];
}
