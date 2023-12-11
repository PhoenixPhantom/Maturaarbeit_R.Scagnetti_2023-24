// Fill out your copyright notice in the Description page of Project Settings.

//NOTE: Many of the ideas (as well as most of the numbers)
//are copied directly (or with small changes) from the native UE5 "Sun Position Calculator" plugin
//but extent it's functionality to better fit the use in this game
//this class should therefore not be viewed as a fully custom created class

#include "Environement/Global/CelestialBodyManager.h"

#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Kismet/KismetMathLibrary.h"

void FRotationProgress::SetNewTargetTime(double TimeToComplete, const FRotator& Target, const FRotator& Current)
{
	TickTimeRemaining = TimeToComplete;
	TargetRotation = Target;
	Increments = (Target-Current).GetNormalized() * (1.0/TimeToComplete);
}

void FRotationProgress::SetNewTargetSpeed(double DegPerSecond, const FRotator& Target, const FRotator& Current)
{
	TargetRotation = Target;
	const FRotator& DeltaRotation = (Target-Current).GetNormalized();
	const double Pitch = abs(DeltaRotation.Pitch);
	const double Yaw = abs(DeltaRotation.Yaw);
	const double Roll = abs(DeltaRotation.Roll);
	if(Pitch > Yaw && Pitch > Roll)
	{
		TickTimeRemaining = Pitch / DegPerSecond;
	}
	else if(Yaw > Pitch && Yaw > Roll)
	{
		TickTimeRemaining = Yaw / DegPerSecond;
	}
	else if(Roll > Yaw && Roll > Pitch)
	{
		TickTimeRemaining = Roll / DegPerSecond;
	}

	Increments = DeltaRotation * (1.0/TickTimeRemaining);
}

FRotator FRotationProgress::Process(double DeltaTime)
{
	TickTimeRemaining -= DeltaTime;
	if(TickTimeRemaining <= 0.0) return TargetRotation;
	return TargetRotation - Increments * TickTimeRemaining;
}

// Sets default values
ACelestialBodyManager::ACelestialBodyManager() : bUpdateWithOnConstruction(true), EquatorTilt(47.3),
	EarthRadius(636000000.0), WorldTimeSpeedMultiplier(1.f), NormalSunBrightness(7.5f), FullMoonBrightness(0.5f),
	NewMoonBrightness(0.f), MoonLightColor(0.2f, 0.45f, 1.f), SolarEclipseSunBrightness(3.f),
	LunarEclipseMoonLightColor(1.f, 0.f, 0.f), bIsEclipse(false)
{
	EquatorScene = CreateDefaultSubobject<USceneComponent>(TEXT("EquatorRotationScene"));
	RootComponent = EquatorScene;
	EquatorScene->SetMobility(EComponentMobility::Movable);

	
	EquatorRelativeScene = CreateDefaultSubobject<USceneComponent>(TEXT("EquatorRelativeScene"));
	EquatorRelativeScene->AttachToComponent(EquatorScene, FAttachmentTransformRules::KeepRelativeTransform);
	EquatorRelativeScene->SetMobility(EComponentMobility::Movable);
	
	
	Sun = CreateDefaultSubobject<UDirectionalLightComponent>(TEXT("Sun"));
	Sun->AttachToComponent(EquatorRelativeScene, FAttachmentTransformRules::KeepRelativeTransform);
	Sun->DynamicShadowCascades = 5;
	Sun->CascadeDistributionExponent = 1.4f;
	Sun->SetAtmosphereSunLight(true);
	Sun->bCastShadowsOnClouds = true;
	Sun->bCastShadowsOnAtmosphere = true;
	Sun->bCastCloudShadows = true;
	Sun->bPerPixelAtmosphereTransmittance = true;
	Sun->Intensity = NormalSunBrightness;
	Sun->ForwardShadingPriority = 1;


	/*//setup the moon's base (to have a common holder for both light and mesh)
	Moon = CreateDefaultSubobject<USceneComponent>(TEXT("Moon"));
	Moon->AttachToComponent(EquatorRelativeScene, FAttachmentTransformRules::KeepRelativeTransform);
	Moon->SetMobility(EComponentMobility::Movable);

	
	MoonMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MoonMesh"));
	MoonMesh->AttachToComponent(Moon, FAttachmentTransformRules::KeepRelativeTransform);
	MoonMesh->SetMobility(EComponentMobility::Movable);
	MoonMesh->SetEnableGravity(false);
	MoonMesh->SetCollisionProfileName("IgnoreAll");
	MoonMesh->SetCanEverAffectNavigation(false);
	MoonMesh->SetRelativeLocation({10'000'000'000.f, 0.f, 0.f});
	MoonMesh->bApplyImpulseOnDamage = false;
	MoonMesh->SetEnableGravity(false);
	MoonMesh->CanCharacterStepUpOn = ECanBeCharacterBase::ECB_No;
	MoonMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MoonMesh->SetGenerateOverlapEvents(false);
	MoonMesh->SetCastShadow(false);

	
	MoonLight = CreateDefaultSubobject<UDirectionalLightComponent>(TEXT("MoonLight"));
	MoonLight->AttachToComponent(MoonMesh, FAttachmentTransformRules::KeepRelativeTransform);
	MoonLight->SetMobility(EComponentMobility::Movable);
	MoonLight->SetRelativeRotation({0.f, 180.f, 0.f});
	MoonLight->Intensity = FullMoonBrightness; //the moon isn't that bright
	MoonLight->SetLightColor(MoonLightColor);
	MoonLight->SetShadowAmount(0.5);
	MoonLight->SetAtmosphereSunLight(true);
	MoonLight->bCastShadowsOnAtmosphere = true;
	MoonLight->bCastCloudShadows = true;
	MoonLight->AtmosphereSunLightIndex = 1;*/
	
	
	SkyDome = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StarrySky"));
	SkyDome->AttachToComponent(EquatorRelativeScene, FAttachmentTransformRules::KeepRelativeTransform);
	SkyDome->SetMobility(EComponentMobility::Movable);
	SkyDome->SetEnableGravity(false);
	SkyDome->SetCollisionProfileName("IgnoreAll");
	SkyDome->SetWorldScale3D({100000000000000.0, 100000000000000.0, 100000000000000.0});
	SkyDome->SetGenerateOverlapEvents(false);
	SkyDome->CanCharacterStepUpOn = ECanBeCharacterBase::ECB_No;
	SkyDome->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SkyDome->SetCastShadow(false);
	
#if WITH_EDITORONLY_DATA
	CompassMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CompassMesh"));
	CompassMesh->AttachToComponent(EquatorRelativeScene, FAttachmentTransformRules::KeepRelativeTransform);
	CompassMesh->SetCanEverAffectNavigation(false);
	CompassMesh->SetRelativeRotation({90.f, 0.f, 0.f});
	CompassMesh->bApplyImpulseOnDamage = false;
	CompassMesh->SetEnableGravity(false);
	CompassMesh->SetUsingAbsoluteRotation(true);
	CompassMesh->bVisibleInReflectionCaptures = false;
	CompassMesh->bVisibleInRayTracing = false;
	CompassMesh->CastShadow = false;
	CompassMesh->bHiddenInGame = true;
	CompassMesh->bIsEditorOnly = true;
	CompassMesh->SetEnableGravity(false);
	CompassMesh->SetGenerateOverlapEvents(false);
	CompassMesh->CanCharacterStepUpOn = ECanBeCharacterBase::ECB_No;
	CompassMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CompassMesh->SetCollisionProfileName(FName("NoCollision"));
#endif
	

	PrimaryActorTick.bCanEverTick = true;
}
/*
// Called every frame
void ACelestialBodyManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	BlendToTargetRotationTick(DeltaTime);
	InGameDateTime += FTimespan(ETimespan::TicksPerSecond * DeltaTime * WorldTimeSpeedMultiplier);
	UpdateDay();
	UpdateDaytime();
}
*/
void ACelestialBodyManager::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	if(bUpdateWithOnConstruction)
	{
		SetActorRotation({0.f, 0.f, -EquatorTilt});
		/*UpdateDay();
		UpdateDaytime();
		Sun->SetRelativeRotationExact(SunRotationProgress.TargetRotation);
		Moon->SetRelativeRotationExact(MoonRotationProgress.TargetRotation);
		if(!CalculateEclipseEffects())
		{
			//no eclipse

			//reset values if it was an eclipse before
			if(bIsEclipse == true)
			{
				MoonMesh->SetCustomPrimitiveDataFloat(0, 0.f);
				MoonLight->SetLightColor(MoonLightColor);
				Sun->SetIntensity(NormalSunBrightness);
				bIsEclipse = false;
			}
			CalculateMoonPhaseEffects();
		}*/
	}
}

void ACelestialBodyManager::BeginPlay()
{
	Super::BeginPlay();
	SetActorRotation({0.f, 0.f, -EquatorTilt});
	//UpdateDay();
	//UpdateDaytime();
}
/*
void ACelestialBodyManager::UpdateDaytime()
{
	if(InGameDateTime - LastUpdatedAtTimeOfDay < DayTimeTolerance) return;
	constexpr double HoursPerDay = 24.0;
	EquatorRelativeScene->SetRelativeRotationExact(FRotator(GetInGameTime().GetTotalHours()/HoursPerDay * 360.0 + 90.0,
		0.0, 0.0));
	LastUpdatedAtTimeOfDay = InGameDateTime;
}

void ACelestialBodyManager::UpdateDay()
{
	if(InGameDateTime - LastUpdatedAtTimeOfDay < DayTimeTolerance) return; 
	double NumOfRotations; //useless placeholder variable (because modf() needs it)
	const double DaysSinceDayZero = FTimespan(InGameDateTime.GetTicks()).GetTotalDays();

	
	///Sun advancement
	
	const double TotalSunSeasonCyclesSinceDayZero = DaysSinceDayZero / DaysPerYear;
	const double AbsoluteSunSeason = modf(TotalSunSeasonCyclesSinceDayZero, &NumOfRotations);
	//calculate the sun's relative tilt according to the season of the year
	const double SunHeight = cos(DOUBLE_TWO_PI*AbsoluteSunSeason) * EarthTilt;
	SunRotationProgress.SetNewTargetSpeed(5, {0.f, SunHeight, 0.f},
		Sun->GetRelativeRotation());

	
	///Moon advancement
	
	const double AbsoluteMoonRotation = modf(DaysSinceDayZero / DaysPerMonth, &NumOfRotations);
	const double AbsoluteMoonOrbitRotation = modf(DaysSinceDayZero / DaysPerLunarOrbitRotation, &NumOfRotations);
	const double CurrentMoonOrbitTilt = sin(DOUBLE_TWO_PI*AbsoluteMoonOrbitRotation) * MoonOrbitTilt;
	//calculate the moon's relative tilt according to the lunar phase
	MoonRotationProgress.SetNewTargetSpeed(5,
		{AbsoluteMoonRotation * 360.0, EarthTilt + CurrentMoonOrbitTilt, 0.f},
		Moon->GetRelativeRotation());
	LastUpdatedAtDateOfYear = InGameDateTime;
}


void ACelestialBodyManager::BlendToTargetRotationTick(const float DeltaSeconds)
{
	bool Changed = false;
	if(SunRotationProgress.IsProcessing())
	{
		Changed = true;
		Sun->SetRelativeRotationExact(SunRotationProgress.Process(DeltaSeconds));
	}
	if(MoonRotationProgress.IsProcessing())
	{
		Changed = true;
		Moon->SetRelativeRotationExact(MoonRotationProgress.Process(DeltaSeconds));
	}
	
	if(Changed)
	{
		if(!CalculateEclipseEffects())
		{
			//no eclipse

			//reset values if it was an eclipse before
			if(bIsEclipse == true)
			{
				MoonMesh->SetCustomPrimitiveDataFloat(0, 0.f);
				MoonLight->SetLightColor(MoonLightColor);
				Sun->SetIntensity(NormalSunBrightness);
				bIsEclipse = false;
			}
			CalculateMoonPhaseEffects();
		}
	}
}

bool ACelestialBodyManager::CalculateEclipseEffects()
{ ///!!!! WE ASSUME THAT THE ROOT OF THIS ACTOR IS PLACED AT THE CENTER OF THE EARTH (SkyAtmosphere)!!!!
	const FVector Scale = MoonMesh->GetComponentScale();
	check(Scale.X == Scale.Y && Scale.X == Scale.Z);
	const float MoonRadius = Scale.X * 50.f; //in-editor-tests deduced that the radius of the unscaled moon sphere is 50 (=> Scale*50 = MoonRadius)
		
	//project the earth shadow along the straight line through the sun and earth to see if it meets the moon (to cause a lunar eclipse) 
	const FVector RelevantEarthShadowLocation = GetActorLocation() +
		Sun->GetComponentRotation().Vector() * MoonMesh->GetRelativeLocation().X;
		
	//project the moon shadow along the straight line through the sun and moon to see if it meets the earth (to cause a solar eclipse)
	const FVector RelevantMoonShadowLocation = MoonMesh->GetComponentLocation() -
		Sun->GetComponentRotation().Vector() * MoonMesh->GetRelativeLocation().X;
	const float MoonCovering = FVector::Distance(RelevantEarthShadowLocation, MoonMesh->GetComponentLocation()) / (EarthRadius+MoonRadius);
	const float EarthCovering =  FVector::Distance(RelevantMoonShadowLocation, GetActorLocation()) / (EarthRadius+MoonRadius);
	if(MoonCovering < 1.f)
	{	//lunar eclipse (red moon)
		float Alpha;
		if(MoonCovering < 0.5f)
		{
			Alpha = 0.5f+pow(0.01f, MoonCovering*2.f)/2.f;
		}
		else
		{
			Alpha = 0.5-pow(0.001f, 1.f-(MoonCovering-0.5f)*2.f);
		}
		
		MoonMesh->SetCustomPrimitiveDataFloat(0, Alpha);
		MoonLight->SetLightColor(UKismetMathLibrary::LinearColorLerp(MoonLightColor, LunarEclipseMoonLightColor,
			Alpha));
		bIsEclipse = true;
		return true;
	}
	//else
	if(EarthCovering < 1.f)
	{
		//solar eclipse (darker sun)
		const float Alpha = pow(0.001f, 1.f-EarthCovering);
		Sun->SetIntensity(UKismetMathLibrary::Lerp(SolarEclipseSunBrightness, NormalSunBrightness, Alpha));
		bIsEclipse = true;
		return true;
	}
	//else
	return false;
}

void ACelestialBodyManager::CalculateMoonPhaseEffects() const
{ ///!!!! WE ASSUME THAT THE ROOT OF THIS ACTOR IS PLACED AT THE CENTER OF THE EARTH (SkyAtmosphere)!!!!
	
	//Set brightness according to the moon phase
	const float Score = FVector::DotProduct(Sun->GetComponentRotation().Vector(),
		UKismetMathLibrary::GetDirectionUnitVector(MoonMesh->GetComponentLocation(),GetActorLocation()));
	//the Dot product's max value is +1(full moon) and min is -1(new moon)
	MoonLight->SetIntensity(UKismetMathLibrary::Lerp(NewMoonBrightness, FullMoonBrightness, (1.f-Score)/2.f));
}*/
