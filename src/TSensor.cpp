#include "TSensor.hpp"

#define INVALIDATE_AFTER_SEC 45
#define SENSOR_BUS_PIN 25

#define POWER_ON_RST_VALUE (85.0F)
#define INVALID_READ (-127.0F)

#define FLOAT_EQUALS(X, Y) (abs(X - Y) < std::numeric_limits<float>::epsilon())

static inline void updateHead(void *compare, void *value);

struct Reading {
   private:
	// private variables

	size_t index;

   public:
	// public variables

	float temperature;
	unsigned long timestamp;
	Reading *next;

	// public constructors

	Reading(size_t index) {
		this->index = index;
		Serial.printf("Reading-Constructor Pos. %i\n", index);

		updateHead(nullptr, this);
	}

	~Reading() {
		Serial.printf("Reading-Destructor Pos. %i\n", index);

		updateHead(this, nullptr);

		// "The value of the operand of delete may be a null pointer value."
		// Delete recursively
		delete next;
	}
};

static Reading *head = nullptr;

static inline void updateHead(void *compare, void *value) {
	if (head == compare)
		head = static_cast<Reading *>(value);
}

// private functions

float TSensor::readCelsius(uint8_t sensorIdx) {
	float value;

	for (size_t i = 0; i < 3; i++) {
		value = sensors.getTempCByIndex(sensorIdx);
		Serial.printf("Sensor %i - Value: %.2f C\n", sensorIdx, value);

		if (FLOAT_EQUALS(value, INVALID_READ))
			throw -127;
		if (!FLOAT_EQUALS(value, POWER_ON_RST_VALUE))
			break;
		Serial.printf("Sensor %i has RST_VALUE (%.2f C). Reading again...\n", sensorIdx, value);
		sensors.requestTemperatures();
	}

	return value;
}

// public constructors

TSensor::TSensor() : oneWire(SENSOR_BUS_PIN), sensors(&oneWire) {
	//sensors.setResolution(12);
	//sensors.setWaitForConversion(true);
	sensors.begin();
}

// public functions

unsigned long TSensor::elapsedSince(uint8_t sensorIdx) {
	if (head == nullptr)
		throw -127;

	Reading *current = head;
	for (size_t idx = 0; idx < sensorIdx; idx++) {
		if (current->next != nullptr) {
			current = current->next;
			continue;
		}
		throw -127;
	}

	return abs(millis() - current->timestamp);
}

float TSensor::getCelsius(uint8_t sensorIdx) {
	if (head == nullptr)
		throw -127;

	Reading *current = head;
	for (size_t idx = 0; idx < sensorIdx; idx++) {
		if (current->next != nullptr) {
			current = current->next;
			continue;
		}
		throw -127;
	}

	return current->temperature;
}

float TSensor::getFahrenheit(uint8_t sensorIdx) {
	return sensors.toFahrenheit(this->getCelsius(sensorIdx));
}

void TSensor::updateAll() {
	sensors.requestTemperatures();

	Reading *drag = nullptr;
	Reading *current = head;
	for (size_t idx = 0;; idx++) {
		float temp;
		try {
			temp = this->readCelsius(idx);
		} catch (int e) {
			if (current && (abs(millis() - current->timestamp) > (INVALIDATE_AFTER_SEC * 1000))) {
				Serial.printf("Value No. %i is no longer valid!\n", idx);
				delete current;
				if (drag != nullptr) {
					drag->next = nullptr;
				}
			}
			return;
		}

		if (current == nullptr) {
			current = new Reading(idx);
			current->next = nullptr;
		}

		if (drag != nullptr) {
			drag->next = current;
		}

		current->temperature = temp;
		current->timestamp = millis();

		drag = current;
		current = current->next;
	}
}
