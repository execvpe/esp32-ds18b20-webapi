#include "Actors.hpp"

#include <cstdarg>

// public functions

actor_t Actors::add(uint8_t actorPin) {
	return add(actorPin, ACTOR_INACTIVE);
}

actor_t Actors::add(uint8_t actorPin, uint8_t initialLevel) {
	static size_t count = 0;

	actors[count] = actorPin;

	pinMode(actorPin, OUTPUT);
	digitalWrite(actorPin, initialLevel);

	return count++;
}

void Actors::signal(actor_t actor, uint8_t level, uint32_t period) {
	digitalWrite(actors[actor], level);
	delay(period);
	digitalWrite(actors[actor], ACTOR_INVERSE(level));
}

void Actors::set(actor_t actor, uint8_t level) {
	digitalWrite(actors[actor], level);
}
