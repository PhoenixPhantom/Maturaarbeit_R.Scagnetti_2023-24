// Fill out your copyright notice in the Description page of Project Settings.


#include "Utility/Animation/AnimNotifyState_TimedNiagaraEffectParameters.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"


UAnimNotifyState_TimedNiagaraEffectParameters::UAnimNotifyState_TimedNiagaraEffectParameters() : NotifyTime(0.f)
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
	if (!ValidateParameters(MeshComp)) return nullptr;

	UNiagaraComponent* NiagaraSystem =  UNiagaraFunctionLibrary::SpawnSystemAttached(Template, MeshComp,
		SocketName, LocationOffset, RotationOffset, EAttachLocation::KeepRelativeOffset, !bDestroyAtEnd);

	if(!PlayLengthName.IsEmpty()) NiagaraSystem->SetNiagaraVariableFloat(PlayLengthName, NotifyTime);

	for(const FFloatParameter& Parameter : FloatParameters)
	{
		NiagaraSystem->SetNiagaraVariableFloat(Parameter.VariableName, Parameter.Value);	
	}
	
	for(const FObjectParameter& Parameter : ObjectParameters)
	{
		NiagaraSystem->SetNiagaraVariableObject(Parameter.VariableName, Parameter.Object);
	}
	
	return NiagaraSystem;
}
