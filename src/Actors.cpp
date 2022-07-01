#include "Actors.hpp"

#include <cstdarg>

// public functions

actor_t Actors::add(uint8_t actorPin) {
	return add(actorPin, ACTOR_INACTIVE);
}

actor_t Actors::add(uint8_t actorPin, uint8_t initialLevel) {
	if (count >= ACTORS_MAX) {
		abort();
	}

	actors[count] = actorPin;

	pinMode(actorPin, OUTPUT);
	digitalWrite(actorPin, initialLevel);

	return count++;
}

bool Actors::set(actor_t actor, uint8_t level) {
	if (actor >= count) {
		return false;
	}

	digitalWrite(actors[actor], level);
	return true;
}

bool Actors::signal(actor_t actor, uint8_t level, uint32_t period) {
	if (actor >= count) {
		return false;
	}

	digitalWrite(actors[actor], level);
	delay(period);
	digitalWrite(actors[actor], ACTOR_INVERSE(level));
	return true;
}
