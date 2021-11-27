#pragma once
#ifndef DECRYPT_HPP
#define DECRYPT_HPP

#include <stddef.h>
#include <stdint.h>

void decryptShuffled(const uint8_t *__restrict raw, size_t idxPos, char *__restrict buf);

#ifndef ENCRYPTED_FIELD
#define ENCRYPTED_FIELD encryptedField
#endif

#endif	// DECRYPT_HPP
