#pragma once

#include "IO.h"

InputInt bottomButton = InputInt(D1, FALLING, INPUT);
InputInt portionDetector = InputInt(D2, FALLING, INPUT_PULLUP);
InputInt jamDetector = InputInt(D7, FALLING, INPUT);

Output motor = {D5, OUTPUT, Output::ELogic_ActiveHigh};
Output led = {D6, OUTPUT, Output::ELogic_ActiveHigh};