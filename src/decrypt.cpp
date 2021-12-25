#include "decrypt.hpp"

#include <climits>	// for CHAR_BIT
#include <cstdint>	// for uint8_t

#define SLLC(V) sllC(V, 3)

static uint8_t sllC(uint8_t value, uint8_t count) {
	const uint8_t mask = (CHAR_BIT * sizeof(value)) - 1;
	count &= mask;
	return (uint8_t) ((value << count) | (value >> (-count & mask)));
}

void decryptShuffled(const uint8_t *__restrict raw, size_t idxPos, char *__restrict buf) {
	static_assert(CHAR_BIT == 8, "This function only works with 8-bit characters!");
	const size_t offset = SLLC(raw[idxPos + 1]);
	const size_t length = (size_t) SLLC(raw[offset]);

	const uint8_t *chars = raw + ((size_t) SLLC(raw[0]));
	const uint8_t *head = raw + offset + 1;

	for (size_t i = 0; i < length; i++) {
		size_t idx = SLLC(head[i]);
		buf[i] = SLLC(chars[(size_t) idx]);
	}
	buf[length] = '\0';
}
