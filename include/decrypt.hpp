#pragma once
#ifndef DECRYPT_HPP
#define DECRYPT_HPP

#include <cstddef>
#include <cstdint>

void decryptShuffled(const uint8_t *__restrict raw, size_t idxPos, char *__restrict buf);

#ifndef ENCRYPTED_FIELD
#define ENCRYPTED_FIELD encryptedField
#endif

#endif	// DECRYPT_HPP
