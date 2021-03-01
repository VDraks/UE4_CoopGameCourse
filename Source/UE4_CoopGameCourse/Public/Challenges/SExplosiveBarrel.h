// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SExplosiveBarrel.generated.h"

class URadialForceComponent;
class USHealthComponent;

UCLASS()
class UE4_COOPGAMECOURSE_API ASExplosiveBarrel : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASExplosiveBarrel();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* MeshComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USHealthComponent* HealthComp;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	URadialForceComponent* ForceComp;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Explosion")
	UMaterialInterface* ExplodeMaterial;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Explosion")
	UParticleSystem* ExplosionEffect;

	UPROPERTY(BlueprintReadOnly, Category = "Explosion", ReplicatedUsing = OnRep_Exploded)
	bool bExploded;

	UFUNCTION()
    void OnRep_Exploded();

	UFUNCTION()
	void OnHealthChanged(USHealthComponent* HealthComponent, float Health, float HealthDelta,
		const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser);

private:
	void PlayExplodeEffects();

	void SetExploded(bool Exploded);
};


