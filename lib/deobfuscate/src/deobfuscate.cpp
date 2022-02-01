#include "deobfuscate.hpp"

#include <climits> // for CHAR_BIT

/**
 * @brief Shift left logical circular. <br>
 * {@code sllC((uint8_t) 0b1010_1100, 2) == 0b1011_0010}
 *
 * @param value a {@code uint8_t} value
 * @param count amount of bits to be shifted left
 * @return the circular shifted {@code uint8_t}
 */
static uint8_t sllC(uint8_t value, uint8_t count) {
	const uint8_t mask = (CHAR_BIT * sizeof(value)) - 1;
	count &= mask;
	return (uint8_t) ((value << count) | (value >> (-count & mask)));
}

void ObfuscatedCredentials::deobfuscate(const uint8_t *__restrict obfuscatedData, size_t idxPos, char *__restrict buf) {
	static_assert(CHAR_BIT == 8, "Proper deobfuscation can only be guaranteed with 8-bit characters!");

	size_t offset = sllC(obfuscatedData[idxPos + 1], 3);
	size_t length = (size_t) sllC(obfuscatedData[offset], 3);

	const uint8_t *chars = obfuscatedData + ((size_t) sllC(*obfuscatedData, 3));
	const uint8_t *head  = obfuscatedData + offset + 1;

	for (size_t i = 0; i < length; i++) {
		size_t idx = sllC(head[i], 3);
		buf[i]     = sllC(chars[(size_t) idx], 3);
	}
	buf[length] = '\0';
}
