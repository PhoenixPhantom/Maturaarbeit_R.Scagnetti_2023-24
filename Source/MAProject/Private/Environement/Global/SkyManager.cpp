// Fill out your copyright notice in the Description page of Project Settings.


#include "Environement/Global/SkyManager.h"

#include "Components/ExponentialHeightFogComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/VolumetricCloudComponent.h"


ASkyManager::ASkyManager()
{
	//Object creation
	Scene = CreateDefaultSubobject<USceneComponent>(TEXT("Scene"));
	Fog = CreateDefaultSubobject<UExponentialHeightFogComponent>(TEXT("StandardFog"));
	SkyLight = CreateDefaultSubobject<USkyLightComponent>(TEXT("SkyLight"));
	Atmosphere = CreateDefaultSubobject<USkyAtmosphereComponent>(TEXT("Atmosphere"));
	VolumetricClouds = CreateDefaultSubobject<UVolumetricCloudComponent>(TEXT("VolumetricVolumetricClouds"));

	//Setup scene
	RootComponent = Scene;
	Scene->SetMobility(EComponentMobility::Movable);
	Scene->SetCanEverAffectNavigation(false);

	//Setup fog
	Fog->SetupAttachment(Scene);
	Fog->SetMobility(EComponentMobility::Movable);
	Fog->SetCanEverAffectNavigation(false);
	Fog->SetFogInscatteringColor(FLinearColor(0.f, 0.f, 0.f, 1.f));
	Fog->SetDirectionalInscatteringColor(FLinearColor(0.f, 0.f, 0.f, 1.f));
	
	//Setup SkyLight
	SkyLight->AttachToComponent(Scene, FAttachmentTransformRules::KeepRelativeTransform);
	SkyLight->SetCanEverAffectNavigation(false);
	SkyLight->SetRelativeLocation(FVector(0.f, 0.f, 150.f));
	SkyLight->SetMobility(EComponentMobility::Movable);
	SkyLight->bLowerHemisphereIsBlack = false;
	SkyLight->bTransmission = true;
	SkyLight->SetCastRaytracedShadows(ECastRayTracedShadow::UseProjectSetting);
	SkyLight->bRealTimeCapture = true;
	SkyLight->CubemapResolution = 64; //should be reduced to 32 if performance hit is too high
	SkyLight->SetSamplesPerPixel(2);

	//Setup Atmosphere
	Atmosphere->AttachToComponent(Scene, FAttachmentTransformRules::KeepRelativeTransform);
	Atmosphere->SetCanEverAffectNavigation(false);

	//Setup VolumetricClouds
	VolumetricClouds->AttachToComponent(Scene, FAttachmentTransformRules::KeepRelativeTransform);
	VolumetricClouds->SetCanEverAffectNavigation(false);
	VolumetricClouds->SetRelativeLocation(FVector(0.f, 0.f, 0.f));
	
	UpdateOverlapsMethodDuringLevelStreaming = EActorUpdateOverlapsMethod::OnlyUpdateMovable;
	PrimaryActorTick.bCanEverTick = false;
}

