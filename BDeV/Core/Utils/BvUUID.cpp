#include "BvUUID.h"


void BvUUID::New()
{
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<u64> dist(0, kU64Max);
	u64 blocks[] = { dist(gen), dist(gen) };
	// Set version (4 bits for version, which is 4)
	blocks[0] = (blocks[0] & 0xFFFFFFFFFFFF0FFFULL) | 0x0000000000004000ULL;
	// Set variant (2 bits for variant, most significant bits are 10)
	blocks[1] = (blocks[1] & 0x3FFFFFFFFFFFFFFFULL) | 0x8000000000000000ULL;
	memcpy(&m_Data1, blocks, sizeof(BvUUID));
}