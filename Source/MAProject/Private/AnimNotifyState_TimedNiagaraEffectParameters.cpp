// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimNotifyState_TimedNiagaraEffectParameters.h"
#include "NiagaraComponent.h"


UAnimNotifyState_TimedNiagaraEffectParameters::UAnimNotifyState_TimedNiagaraEffectParameters()
{
}

void UAnimNotifyState_TimedNiagaraEffectParameters::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);
	//TODO: Cast doesn't seem valid
	UNiagaraComponent* NiagaraSystem = CastChecked<UNiagaraComponent>(GetSpawnedEffect(MeshComp));

	for(const FObjectParameter& Parameter : ObjectParameters)
	{
		NiagaraSystem->SetNiagaraVariableObject(Parameter.VariableName, Parameter.Object);	
	}
}