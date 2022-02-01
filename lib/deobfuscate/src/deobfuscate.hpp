#pragma once
#ifndef DEOBFUSCATE_HPP
#define DEOBFUSCATE_HPP

#include <cstddef> // for size_t
#include <cstdint> // for uint8_t

namespace ObfuscatedCredentials {
	/**
	 * @brief Deobfuscate credentials. <b>There is and cannot be any kind of error checking in this function. Buffer
	 * overflows are possible, as is the access of invalid data. The behavior of the function with wrong parameters is
	 * not deterministic!</b>
	 *
	 * @param obfuscatedData the array from the header created with the Credentials Obfuscator
	 * @param idxPos position of the String specified on the command line (starting at 0)
	 * @param buf a buffer of sufficient size to store the obfuscated string (do not forget to add 1 for the NULL byte)
	 */
	void deobfuscate(const uint8_t *__restrict obfuscatedData, size_t idxPos, char *__restrict buf);
} // namespace ObfuscatedCredentials

#endif // DEOBFUSCATE_HPP
