#pragma once
#include <functional>


template <class _Fty>
struct TOwnedFunction
{
	std::function<_Fty> Function;
	UObject* Owner;	
};
