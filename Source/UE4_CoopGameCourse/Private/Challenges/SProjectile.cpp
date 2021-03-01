// Fill out your copyright notice in the Description page of Project Settings.


#include "Challenges/SProjectile.h"

#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"

ASProjectile::ASProjectile()
{
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileComp"));
	ProjectileMovement->InitialSpeed = 3000.f;
	ProjectileMovement->MaxSpeed = 3000.f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = true;
}

void ASProjectile::BeginPlay()
{
	Super::BeginPlay();

	FTimerHandle TimerHandle_Explode;
	GetWorldTimerManager().SetTimer(TimerHandle_Explode, this, &ASProjectile::Explode, 1.0f);
}

inline void ASProjectile::Explode()
{
	if (ExplodeEffect)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplodeEffect, GetActorLocation(), GetActorRotation());	
	}
	
	UGameplayStatics::ApplyRadialDamage(GetWorld(), 100.f, GetActorLocation(), 200.f,
		DamageType, {}, this);
	
	Destroy();
}

