// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SkyManager.generated.h"

class UVolumetricCloudComponent;

UCLASS()
class MAPROJECT_API ASkyManager : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ASkyManager();

protected:
	UPROPERTY(EditDefaultsOnly, Category="Components")
	USceneComponent* Scene;
	UPROPERTY(EditDefaultsOnly, Category="Components")
	UExponentialHeightFogComponent* Fog;
	UPROPERTY(EditDefaultsOnly, Category="Components")
	USkyAtmosphereComponent* Atmosphere;
	UPROPERTY(EditDefaultsOnly, Category="Components")
	USkyLightComponent* SkyLight;
	UPROPERTY(EditDefaultsOnly, Category="Components")
	UVolumetricCloudComponent* VolumetricClouds;
};
