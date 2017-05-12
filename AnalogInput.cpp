#include "Arduino.h"

// Combined CV + Pot

// 2 Pins

// 13 bit + 13 bit => LOW_VAL -> HIGH_VAL

// Separate Analogue In

// 13 bit => LOW_VAL -> HIGH_VAL

// Mapping function isn't uniform, i.e. output may have a half step at top and bottom of range
#include "AnalogInput.h"

AnalogInput::AnalogInput(uint pinIndex) {

	pin = pinIndex;
}

void AnalogInput::setRange(float outLow, float outHigh, boolean quantiseOutput) {
	quantise = quantiseOutput;

	// Check the range is the right way around
	if(outLow > outHigh) {
		outputLow = outHigh;
		outputHigh = outLow;
	} else {
		outputLow = outLow;
		outputHigh = outHigh;
	}

	// Use range + 1 so that the top value has equal coverage across the input range
	float range = (outputHigh - outputLow) + 1;

	inToOutRatio = (float) ADC_MAX_VALUE / range;

	if(range < 200 && quantise) {
		hysteresis = true;
		borderThreshold = inToOutRatio / 10;
	} else {
		borderThreshold = 12;
	}

	// Keep the inverse ratio so we only multiply during update
	inverseRatio = 1.0 / inToOutRatio;

	currentValue = outLow;
}

void AnalogInput::setAverage(boolean avg) {
	average = avg;
}

void AnalogInput::setSmoothSteps(int steps) {
	smoothSteps = steps;
}

float AnalogInput::getRatio() {
	return inToOutRatio;
}

boolean AnalogInput::update() {

	if(average) {
		inputValue = 0;
		for(int i=0;i<smoothSteps;i++) {
			inputValue += analogRead(pin);
		}
		inputValue /= smoothSteps;
		if(inputValue < borderThreshold) {
			inputValue = 0;
		}
		if(abs(inputValue - valueAtLastChange) > borderThreshold) {
			valueAtLastChange = inputValue;
			currentValue = (inputValue * inverseRatio) + outputLow;
			return true;
		}

	} else {
		inputValue = constrain(analogRead(pin),0, ADC_MAX_VALUE - 1);

		if(quantise) {
			int newValue = (int) (inputValue * inverseRatio) + outputLow;

			if(newValue != currentValue) {
				// Check for hysteresis threshold
				if(abs(inputValue - valueAtLastChange) > borderThreshold) {
					valueAtLastChange = inputValue;
					currentValue = newValue;
					return true;
				}
			}
		} else if(inputValue != oldInputValue) {
			currentValue = (inputValue * inverseRatio) + outputLow;
			return true;
		}

	}

	return false;
}

void AnalogInput::printDebug() {
	Serial.print("I ");
	Serial.print(pin);
	Serial.print("\t");
	Serial.println(inputValue);
}

