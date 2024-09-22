#include "BvRandom.h"


BvSplitMix64::BvSplitMix64(u64 seed)
	: m_State(seed)
{
}


u64 BvSplitMix64::Next()
{
	u64 result = m_State;
	m_State = m_State + 0x9E3779B97F4A7C15ULL;
	result = (result ^ (result >> 30)) * 0xBF58476D1CE4E5B9ULL;
	result = (result ^ (result >> 27)) * 0x94D049BB133111EBULL;
	return result ^ (result >> 31);
}


u64 BvSplitMix64::operator()()
{
	return Next();
}