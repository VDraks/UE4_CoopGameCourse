// Fill out your copyright notice in the Description page of Project Settings.


#include "Challenges/SExplosiveBarrel.h"

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
}

// Called when the game starts or when spawned
void ASExplosiveBarrel::BeginPlay()
{
	Super::BeginPlay();

	HealthComp->OnHealthChanged.AddDynamic(this, &ASExplosiveBarrel::OnHealthChanged);
}

void ASExplosiveBarrel::OnHealthChanged(USHealthComponent* HealthComponent, float Health, float HealthDelta,
    const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser)
{
	if (Health <= 0.0f && !bDied)
	{
		bDied = true;

		ForceComp->FireImpulse();

		MeshComp->AddImpulse(FVector::UpVector * 50000);		

		MeshComp->SetMaterial(0, ExplodeMaterial);
		
		if (ExplosionEffect)
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionEffect, GetActorLocation());	
		}
	}	
}