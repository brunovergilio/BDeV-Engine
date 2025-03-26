#include "BvUUID.h"
#include "BvRandom.h"


void BvUUID::New()
{
	BvSplitMix64 gen;
	u64 blocks[] = { gen.Next(), gen.Next() };
	// Set version (4 bits for version, which is 4)
	blocks[0] = (blocks[0] & 0xFFFFFFFFFFFF0FFFULL) | 0x0000000000004000ULL;
	// Set variant (2 bits for variant, most significant bits are 10)
	blocks[1] = (blocks[1] & 0x3FFFFFFFFFFFFFFFULL) | 0x8000000000000000ULL;
	
	m_ULL1 = blocks[0];
	m_ULL2 = blocks[1];
}