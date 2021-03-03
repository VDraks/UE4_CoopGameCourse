// Fill out your copyright notice in the Description page of Project Settings.


#include "SGameMode.h"


#include "SGameState.h"
#include "SPlayerState.h"
#include "Components/SHealthComponent.h"

ASGameMode::ASGameMode()
{
	TimeBetweenWaves = 2.f;

	GameStateClass = ASGameState::StaticClass();
	PlayerStateClass = ASPlayerState::StaticClass();

	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 1.f;
}

void ASGameMode::SpawnBotTimerElapsed()
{
	SpawnNewBot();

	NrOfBotsToSpawn--;

	if (NrOfBotsToSpawn <= 0)
	{
		EndWave();
	}
}

void ASGameMode::StartWave()
{
	WaveCount++;

	NrOfBotsToSpawn = 2 * WaveCount;

	GetWorldTimerManager().SetTimer(TimerHandle_BotSpawner, this, &ASGameMode::SpawnBotTimerElapsed, 1.f, true, 0.f);

	SetWaveState(EWaveState::WaveInProgress);
}

void ASGameMode::EndWave()
{
	GetWorldTimerManager().ClearTimer(TimerHandle_BotSpawner);

	SetWaveState(EWaveState::WaitingToComplete);
}

void ASGameMode::PrepareForNextWave()
{
	GetWorldTimerManager().SetTimer(TimerHandle_NextWaveStart, this, &ASGameMode::StartWave, TimeBetweenWaves, false);

	SetWaveState(EWaveState::WaitingToStart);
}

void ASGameMode::CheckWaveState()
{
	const bool bIsPreparingForWave = GetWorldTimerManager().IsTimerActive(TimerHandle_NextWaveStart);
	
	if (NrOfBotsToSpawn > 0 || bIsPreparingForWave)
	{
		return;
	}
	
	bool bIsAnyBotAlive = false;
	
	for (auto It = GetWorld()->GetPawnIterator(); It; ++It)
	{
		APawn* TestPawn = It->Get();
		if (TestPawn == nullptr || TestPawn->IsPlayerControlled())
		{
			continue;
		}

		USHealthComponent* HealthComp = TestPawn->FindComponentByClass<USHealthComponent>();
		if (HealthComp && HealthComp->GetHealth() > 0.f)
		{
			bIsAnyBotAlive = true;
			break;
		}		
	}

	if (!bIsAnyBotAlive)
	{
		SetWaveState(EWaveState::WaveComplete);
		
		PrepareForNextWave();
	}
}

void ASGameMode::CheckAnyPlayerAlive()
{
	for (auto It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PC = It->Get();
		if (PC && PC->GetPawn())
		{
			APawn* MyPawn = PC->GetPawn();
			auto HealthComp = MyPawn->FindComponentByClass<USHealthComponent>();
			if (ensure(HealthComp) && HealthComp->GetHealth() > 0.0f)
			{
				return;
			}
		}
	}

	GameOver();
}

void ASGameMode::GameOver()
{
	EndWave();

	// TODO Finish the match

	UE_LOG(LogTemp, Log, TEXT("Game over"));

	SetWaveState(EWaveState::GamOver);
}

void ASGameMode::SetWaveState(EWaveState NewState)
{
	ASGameState* GS = GetGameState<ASGameState>();
	if (ensure(GS))
	{
		GS->SetWaveState(NewState);
	}	
}

void ASGameMode::StartPlay()
{
	Super::StartPlay();

	PrepareForNextWave();
}

void ASGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	CheckWaveState();

	CheckAnyPlayerAlive();
}
