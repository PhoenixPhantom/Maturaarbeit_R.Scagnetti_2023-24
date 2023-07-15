#pragma once

#include "CoreMinimal.h"

template<typename F, typename S, typename T>
struct TTriple
{
	TTriple() : bIsValid(false){}
	TTriple(F NewFirst, S NewSecond, T NewThird) : bIsValid(true), First(NewFirst), Second(NewSecond), Third(NewThird){}
	bool bIsValid;
	F First;
	S Second;
	T Third;

	template<typename F2, typename S2, typename T2>
	TTriple operator=(const TTriple<F2, S2, T2>& Other){ bIsValid = Other.bIsValid; First = Other.First;
		Second = Other.Second; Third = Other.Third; return *this; }
};
