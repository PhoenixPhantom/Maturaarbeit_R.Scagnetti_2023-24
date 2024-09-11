// Fill out your copyright notice in the Description page of Project Settings.


#include "Utility/Animation/AnimNotifyState_TimedNiagaraEffectParameters.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"


UAnimNotifyState_TimedNiagaraEffectParameters::UAnimNotifyState_TimedNiagaraEffectParameters() : NotifyTime(0.f),
	NonAttachedScale(1.f)
{
}

void UAnimNotifyState_TimedNiagaraEffectParameters::NotifyBegin(USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);
	NotifyTime = TotalDuration;
}

UFXSystemComponent* UAnimNotifyState_TimedNiagaraEffectParameters::SpawnEffect(USkeletalMeshComponent* MeshComp,
                                                                               UAnimSequenceBase* Animation) const
{
	// Only spawn if we've got valid params
	if (!SocketName.IsNone() && !ValidateParameters(MeshComp)) return nullptr;

	UNiagaraComponent* NiagaraSystem;
	if(!SocketName.IsNone())
	{
		 NiagaraSystem =  UNiagaraFunctionLibrary::SpawnSystemAttached(Template, MeshComp,
			SocketName, LocationOffset, RotationOffset, EAttachLocation::KeepRelativeOffset, bDestroyAtEnd);
	}
	else
	{
		const FVector Offset =
			MeshComp->GetComponentRotation().RotateVector(LocationOffset)*MeshComp->GetComponentScale().X;
		NiagaraSystem = UNiagaraFunctionLibrary::SpawnSystemAtLocation(MeshComp->GetWorld(), Template,
			 Offset + MeshComp->GetComponentLocation(), MeshComp->GetComponentRotation() + RotationOffset,
			 NonAttachedScale, bDestroyAtEnd);
	}

	if(!PlayLengthName.IsNone()) NiagaraSystem->SetVariableFloat(PlayLengthName, NotifyTime);

	for(const FFloatParameter& Parameter : FloatParameters)
	{
		if(Parameter.bUseMeshScaling)
		{
			NiagaraSystem->SetVariableFloat(Parameter.VariableName,
				Parameter.Value * MeshComp->GetComponentScale().X);
		}
		else
		{
			NiagaraSystem->SetVariableFloat(Parameter.VariableName, Parameter.Value);
		}
	}
	for(const FVectorParameter& Parameter : VectorParameters)
	{
		if(Parameter.bUseMeshScaling)
		{
			NiagaraSystem->SetVariableVec3(Parameter.VariableName,
				Parameter.Value * MeshComp->GetComponentScale());
		}
		else
		{
			NiagaraSystem->SetVariableVec3(Parameter.VariableName, Parameter.Value);
		}
	}
	
	for(const FMaterialParameter& Parameter : MaterialParameters)
	{
		NiagaraSystem->SetVariableMaterial(Parameter.VariableName, Parameter.Material);
	}
	for(const FLinearColorParameter& Parameter : LinearColorParameters)
	{
		NiagaraSystem->SetVariableLinearColor(Parameter.VariableName, Parameter.Value);
	}
	
	return NiagaraSystem;
}
