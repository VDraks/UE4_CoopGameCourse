// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "GameFramework/Actor.h"
#include "SPickupActor.generated.h"

class USphereComponent;

UCLASS()
class UE4_COOPGAMECOURSE_API ASPickupActor : public AActor
{
	GENERATED_BODY()
	
public:	
	ASPickupActor();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	USphereComponent* SphereComp;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UDecalComponent* DecalComp;

public:
	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;	
};
