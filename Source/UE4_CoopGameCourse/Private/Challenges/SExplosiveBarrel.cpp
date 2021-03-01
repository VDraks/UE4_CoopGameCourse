// Fill out your copyright notice in the Description page of Project Settings.


#include "Challenges/SExplosiveBarrel.h"


#include "GeneratedCodeHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "PhysicsEngine/RadialForceComponent.h"

#include "Components/SHealthComponent.h"

// Sets default values
ASExplosiveBarrel::ASExplosiveBarrel()
{
	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	RootComponent = MeshComp;

	HealthComp = CreateDefaultSubobject<USHealthComponent>(TEXT("HealthComp"));

	ForceComp = CreateDefaultSubobject<URadialForceComponent>(TEXT("Force"));
	ForceComp->SetupAttachment(MeshComp);
	ForceComp->bImpulseVelChange = true;
	ForceComp->bAutoActivate = false;
	ForceComp->bIgnoreOwningActor = true;

	SetReplicates(true);
}

// Called when the game starts or when spawned
void ASExplosiveBarrel::BeginPlay()
{
	Super::BeginPlay();

	HealthComp->OnHealthChanged.AddDynamic(this, &ASExplosiveBarrel::OnHealthChanged);
}

void ASExplosiveBarrel::OnRep_Exploded()
{
	PlayExplodeEffects();	
}

void ASExplosiveBarrel::OnHealthChanged(USHealthComponent* HealthComponent, float Health, float HealthDelta,
                                        const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser)
{
	if (bExploded)
	{
		return;
	}
	
	if (Health <= 0.0f)
	{
		bExploded = true;

		PlayExplodeEffects();

		if (GetLocalRole() == ROLE_Authority)
		{
			ForceComp->FireImpulse();

			MeshComp->AddImpulse(FVector::UpVector * 400, NAME_None, true);
		}
	}	
}

void ASExplosiveBarrel::PlayExplodeEffects()
{
	MeshComp->SetMaterial(0, ExplodeMaterial);
		
	if (ExplosionEffect)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionEffect, GetActorLocation());	
	}
}

void ASExplosiveBarrel::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ASExplosiveBarrel, bExploded, COND_SkipOwner);
}
