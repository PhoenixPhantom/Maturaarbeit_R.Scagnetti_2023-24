// Fill out your copyright notice in the Description page of Project Settings.


#include "ReadWriteHelpers.h"

#include "Serialization/ObjectAndNameAsStringProxyArchive.h"



namespace UReadWriteHelpers
{
	void WriteToTarget(AActor* Target, TArray<uint8>& Bytes)
	{
		FMemoryReader MemReader(Bytes);

		FObjectAndNameAsStringProxyArchive ProxyArchive(MemReader, true);
		ProxyArchive.ArIsSaveGame = true;
		//Convert binary array back into actor's variables
		Target->Serialize(ProxyArchive);
	}

	void ReadFromTarget(AActor* Target, TArray<uint8>& Bytes)
	{
		//Pass the array to fill with data from Actor
		FMemoryWriter MemoryWriter(Bytes);
		FObjectAndNameAsStringProxyArchive ProxyArchive(MemoryWriter, true);
		//Find only variables with UPROPERTY(SaveGame)
		ProxyArchive.ArIsSaveGame = true;
		//Converts Actor's SaveGame UPROPERTIES into binary array
		Target->Serialize(ProxyArchive);
	}
}