#include "BvUUID.h"


BvUUID GenerateUUIDv4()
{
	std::random_device rd;
	std::mt19937_64 gen(rd()); // Use 64-bit Mersenne Twister engine
	std::uniform_int_distribution<u64> dis;

	// Generate two 64-bit random numbers
	u64 part1 = dis(gen);
	u64 part2 = dis(gen);

	// Set version (4 bits for version, which is 4)
	part1 = (part1 & 0xFFFFFFFFFFFF0FFFULL) | 0x0000000000004000ULL;

	// Set variant (2 bits for variant, most significant bits are 10)
	part2 = (part2 & 0x3FFFFFFFFFFFFFFFULL) | 0x8000000000000000ULL;

	return { part1, part2 };
}