#pragma once
#ifndef ACTORS_HPP
#define ACTORS_HPP

#include <Arduino.h>

#include <cstdint>

#define ACTORS_MAX 3

#define ACTOR_ACTIVE   HIGH
#define ACTOR_INACTIVE LOW

#define ACTOR_INVERSE(L) ((L == ACTOR_ACTIVE) ? ACTOR_INACTIVE : ACTOR_ACTIVE)

using actor_t = uint8_t;

class Actors {
	private:
	// private variables
	uint8_t actors[ACTORS_MAX];

	public:
	// public functions
	actor_t add(uint8_t actorPin);
	actor_t add(uint8_t actorPin, uint8_t initialLevel);

	void signal(actor_t actor, uint8_t level, uint32_t period);
	void set(actor_t actor, uint8_t level);
};

#endif // ACTORS_HPP
