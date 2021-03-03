// Fill out your copyright notice in the Description page of Project Settings.


#include "SGameState.h"

#include "GeneratedCodeHelpers.h"

void ASGameState::OnRep_WaveState(EWaveState OldState)
{
	WaveStateChanged(WaveState, OldState);
}

void ASGameState::SetWaveState(EWaveState NewState)
{
	if (GetLocalRole() == ROLE_Authority)
	{
		const auto OldState = WaveState;

		WaveState = NewState;
		OnRep_WaveState(OldState);	
	}	
}

void ASGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASGameState, WaveState);
}
