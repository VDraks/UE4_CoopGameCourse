// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/STrackerBot.h"



#include "DrawDebugHelpers.h"
#include "NavigationPath.h"
#include "NavigationSystem.h"
#include "SCharacter.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SphereComponent.h"
#include "Sound/SoundCue.h"

#include "Components/SHealthComponent.h"

// Sets default values
ASTrackerBot::ASTrackerBot()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
    MeshComp->SetCanEverAffectNavigation(false);
	MeshComp->SetSimulatePhysics(true);
    RootComponent = MeshComp;

	HealthComp = CreateDefaultSubobject<USHealthComponent>(TEXT("HealthComp"));

	SphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	SphereComp->SetSphereRadius(200);
	SphereComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SphereComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	SphereComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	SphereComp->SetupAttachment(RootComponent);

	BotCheckSphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("BotCheckSphereComp"));
	BotCheckSphereComp->SetSphereRadius(500);
	BotCheckSphereComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	BotCheckSphereComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	BotCheckSphereComp->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
	BotCheckSphereComp->SetupAttachment(RootComponent);

	bUseVelocityChange = false;
	MovementForce = 1000;
	RequiredDistanceToTarget = 100;

    ExplosionDamage = 40;
    ExplosionRadius = 200;

	SelfDamageInterval = 0.25f;

	PowerLevel = 0;
	MaxPowerLevel = 3;
	LastPowerLevelCheckTime = 0.f;
}

// Called when the game starts or when spawned
void ASTrackerBot::BeginPlay()
{
	Super::BeginPlay();

	HealthComp->OnHealthChanged.AddDynamic(this, &ASTrackerBot::HandleTakeDamage);

	if (GetLocalRole() == ROLE_Authority)
	{
		NextPathPoint = GetNextPathPoint();	
	}	
}

void ASTrackerBot::HandleTakeDamage(USHealthComponent* HealthComponent, float Health, float HealthDelta,
	const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser)
{
	if (GetMatInst())
	{
		GetMatInst()->SetScalarParameterValue("LastTimeDamageTaken", GetWorld()->TimeSeconds);	
	}

	UE_LOG(LogTemp, Log, TEXT("Health %s of %s"), *FString::SanitizeFloat(Health), *GetName())

	if (Health <= 0.0f)
	{
		SelfDestruct();
	}
}

FVector ASTrackerBot::GetNextPathPoint()
{
	ACharacter* PlayerPawn = UGameplayStatics::GetPlayerCharacter(this, 0);
	
	UNavigationSystemV1* NavigationSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(this);
	UNavigationPath* NavPath = NavigationSystem->FindPathToActorSynchronously(this, GetActorLocation(), PlayerPawn);

	if (NavPath->PathPoints.Num() > 1)
	{
		return NavPath->PathPoints[1];
	}

	return GetActorLocation();
}

void ASTrackerBot::SelfDestruct()
{
	if (bExploded)
	{
		return;
	}

	bExploded = true;

	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionEffect, GetActorLocation());

	UGameplayStatics::PlaySoundAtLocation(this, ExplodedSound, GetActorLocation());

	MeshComp->SetVisibility(false, true);
	MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	if (GetLocalRole() == ROLE_Authority)
	{
		TArray<AActor*> IgnoredActors;
		IgnoredActors.Add(this);

		const float FinalDamage = ExplosionDamage +
			ExplosionDamage * FMath::Clamp(PowerLevel / static_cast<float>(MaxPowerLevel), 0.f, 1.f);

		UGameplayStatics::ApplyRadialDamage(this, FinalDamage, GetActorLocation(), ExplosionRadius, nullptr, IgnoredActors, this,
                                            GetInstigatorController(), true);

		DrawDebugSphere(GetWorld(), GetActorLocation(), ExplosionRadius, 12, FColor::Red, false, 2.0f, 0, 1.0f);

		SetLifeSpan(2.f);
	}
}

void ASTrackerBot::DamageSelf()
{
	UGameplayStatics::ApplyDamage(this, 20, GetInstigatorController(), this, nullptr);
}

void ASTrackerBot::NotifyActorBeginOverlap(AActor* OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);
	
	if (bStartedSelfDestruction ||  bExploded)
	{
		return;
	}
	
	ASCharacter* PlayerPawn = Cast<ASCharacter>(OtherActor);
	if (PlayerPawn)
	{
		if (GetLocalRole() == ROLE_Authority)
		{
			GetWorldTimerManager().SetTimer(TimerHandle_SelfDamage, this, &ASTrackerBot::DamageSelf, SelfDamageInterval, true, 0.0f);
		}

		bStartedSelfDestruction = true;

		UGameplayStatics::SpawnSoundAttached(SelfDestructSound, RootComponent);		
	}
	else
	{
		ASTrackerBot* OtherBot = Cast<ASTrackerBot>(OtherActor);
	}
}

void ASTrackerBot::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (GetLocalRole() == ROLE_Authority && !bExploded)
	{
		const float DistanceToTarget = (GetActorLocation() - NextPathPoint).Size();

		if (DistanceToTarget <= RequiredDistanceToTarget)
		{
			NextPathPoint = GetNextPathPoint();

			DrawDebugString(GetWorld(), GetActorLocation(), "Target Reached!");
		}
		else
		{
			FVector ForceDirection = NextPathPoint - GetActorLocation();
			ForceDirection.Normalize();

			ForceDirection *= MovementForce;
		
			MeshComp->AddForce(ForceDirection, NAME_None, bUseVelocityChange);

			DrawDebugDirectionalArrow(GetWorld(), GetActorLocation(), GetActorLocation() + ForceDirection,
                32, FColor::Yellow, false, 0.0f, 0, 1.0f);
		}

		DrawDebugSphere(GetWorld(), NextPathPoint, 20, 12, FColor::Yellow,
            false, 0.0f, 1.f);
	}

	CheckNearbyBots(DeltaTime);
}

UMaterialInstanceDynamic* ASTrackerBot::GetMatInst()
{
	if (MatInst == nullptr)
	{
		MatInst = MeshComp->CreateAndSetMaterialInstanceDynamicFromMaterial(0, MeshComp->GetMaterial(0));	
	}

	return MatInst;
}

void ASTrackerBot::CheckNearbyBots(float DeltaTime)
{	
	if (LastPowerLevelCheckTime < 1.f)
	{
		LastPowerLevelCheckTime += DeltaTime;
		return;
	}

	LastPowerLevelCheckTime = 0.f;
	
	const int OldPowerLevel = PowerLevel;
	
	TArray<AActor*> OverlappingActors;
	GetOverlappingActors(OverlappingActors, GetClass());

	PowerLevel = 0;
	for (const AActor* Actor : OverlappingActors)
	{
		PowerLevel++;
		UE_LOG(LogTemp, Log, TEXT("Bot overlaped %s"), *Actor->GetName());
	}

	if (OldPowerLevel != PowerLevel)
	{
		if (GetMatInst())
		{
			GetMatInst()->SetScalarParameterValue("PowerLevelAlpha", PowerLevel / static_cast<float>(MaxPowerLevel));	
		}
	}
}

