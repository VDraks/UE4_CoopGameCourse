// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SWeapon.h"
#include "SGrenadeLauncher.generated.h"

class ASProjectile;

UCLASS()
class UE4_COOPGAMECOURSE_API ASGrenadeLauncher : public ASWeapon
{
	GENERATED_BODY()

public:
	virtual void Fire() override;
	
	UPROPERTY(EditDefaultsOnly, Category="Projectile")
	TSubclassOf<ASProjectile> ProjectileClass;
};
