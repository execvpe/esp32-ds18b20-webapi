#pragma once
#ifndef ACTORS_HPP
#define ACTORS_HPP

#include <Arduino.h>

#include <cstdint>

#define ACTORS_MAX 1

#define ACTOR_ACTIVE   HIGH
#define ACTOR_INACTIVE LOW

#define ACTOR_INVERSE(L) ((L == ACTOR_ACTIVE) ? ACTOR_INACTIVE : ACTOR_ACTIVE)

using actor_t = uint8_t;

class Actors {
	private:
	// private variables
	actor_t actors[ACTORS_MAX];
	actor_t count = 0;

	public:
	// public functions
	actor_t add(uint8_t actorPin);
	actor_t add(uint8_t actorPin, uint8_t initialLevel);

	bool set(actor_t actor, uint8_t level);
	bool signal(actor_t actor, uint8_t level, uint32_t period);
};

#endif // ACTORS_HPP
