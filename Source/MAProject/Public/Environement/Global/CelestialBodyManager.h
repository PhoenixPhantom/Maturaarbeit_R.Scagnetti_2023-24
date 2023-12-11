// Fill out your copyright notice in the Description page of Project Settings.



//NOTE: Many of the ideas (as well as most of the numbers)
//are taken directly from the native Sun Position Calculator plugin
//but extent it's functionality to better fit the use in this game
//this class should therefore not be viewed as a fully custom created class

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CelestialBodyManager.generated.h"

class UDirectionalLightComponent;

struct FRotationProgress
{
	FRotationProgress() : TickTimeRemaining(0.0){}
	void SetNewTargetTime(double TimeToComplete, const FRotator& Target, const FRotator& Current);
	void SetNewTargetSpeed(double DegPerSecond, const FRotator& Target, const FRotator& Current);
	bool IsProcessing() const { return TickTimeRemaining > 0.0; }
	FRotator Process(double DeltaTime);
	FRotator TargetRotation;
	FRotator Increments;
	double TickTimeRemaining;
};

UCLASS()
class MAPROJECT_API ACelestialBodyManager : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ACelestialBodyManager();

	//virtual void Tick(float DeltaTime) override;

	//Get the current time of day
	FTimespan GetInGameTime() const{ return InGameDateTime.GetTimeOfDay(); }

	virtual void OnConstruction(const FTransform& Transform) override;

protected:

	const double DaysPerYear = 365.256; //in days/full rotation
	const double EarthTilt = 23.43632; //in degrees
	const double MoonOrbitTilt = 5.14;
	const double DaysPerLunarOrbitRotation = 3232.6054; //in days/full rotation
	const double DaysPerMonth =  29.53; //in days/full rotations
	
	FDateTime LastUpdatedAtTimeOfDay;
	FDateTime LastUpdatedAtDateOfYear;
	
	FRotationProgress SunRotationProgress;
	FRotationProgress MoonRotationProgress;

	UPROPERTY(EditAnywhere)
	uint8 bUpdateWithOnConstruction:1;
	
	//The actual latitude of the location
	UPROPERTY(EditAnywhere, Category="WorldInfo|Earth", meta=(UIMin="-180", UIMax="180"))
	float EquatorTilt;

	UPROPERTY(EditAnywhere, Category="WorldInfo|Earth", meta=(Units="cm"))
	float EarthRadius;
		
	UPROPERTY(EditAnywhere, Category="WorldInfo|Time", meta=(ClampMin="1", UIMin="1", ForceUnits="x"))
	float WorldTimeSpeedMultiplier;

	//The in-game time that is allowed to pass until the sun adjusts its angle to account for changes in day time
	UPROPERTY(EditAnywhere, Category="WorldInfo|Time")
	FTimespan DayTimeTolerance;

	//The in-game time that is allowed to pass until the sun adjusts its angle to account for changes in day of the year
	UPROPERTY(EditAnywhere, Category="WorldInfo|Time")
	FTimespan DateTimeTolerance;

	UPROPERTY(EditAnywhere, Category="WorldInfo|Time")
	FDateTime InGameDateTime;

	
	UPROPERTY(EditAnywhere, Category="WorldInfo|Sun")
	float NormalSunBrightness;
	
	UPROPERTY(EditAnywhere, Category="WorldInfo|Moon")
	float FullMoonBrightness;
	UPROPERTY(EditAnywhere, Category="WorldInfo|Moon")
	float NewMoonBrightness;
	UPROPERTY(EditAnywhere, Category="WorldInfo|Moon")
	FLinearColor MoonLightColor;

	
	UPROPERTY(EditAnywhere, Category="WorldInfo|SolarEclipse")
	float SolarEclipseSunBrightness;
	UPROPERTY(EditAnywhere, Category="WorldInfo|SolarEclipse")
	FLinearColor LunarEclipseMoonLightColor;

	UPROPERTY(VisibleAnywhere, Category="WorldInfo|SolarEclipse")
	uint8 bIsEclipse:1;

	UPROPERTY(EditDefaultsOnly, Category=Components)
	USceneComponent* EquatorScene;
	UPROPERTY(EditDefaultsOnly, Category=Components)
	USceneComponent* EquatorRelativeScene;
	UPROPERTY(EditDefaultsOnly, Category=Components)
	UDirectionalLightComponent* Sun;
	/*UPROPERTY(EditDefaultsOnly, Category=Components)
	USceneComponent* Moon;
	UPROPERTY(EditAnywhere, Category=Components)
	UStaticMeshComponent* MoonMesh;
	UPROPERTY(EditAnywhere, Category=Components)
	UDirectionalLightComponent* MoonLight;*/
	UPROPERTY(EditDefaultsOnly, Category=Components)
	UStaticMeshComponent* SkyDome;
#if WITH_EDITORONLY_DATA
	UPROPERTY(EditDefaultsOnly, Category=Components)
	UStaticMeshComponent* CompassMesh;
#endif
	
	
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	/*void UpdateDaytime();
	void UpdateDay();
	void BlendToTargetRotationTick(const float DeltaSeconds);
	///@return if there is an eclipse
	bool CalculateEclipseEffects();
	void CalculateMoonPhaseEffects() const;*/
};
