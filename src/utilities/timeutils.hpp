#pragma once
#include <chrono>

class Clock {
private:
	std::chrono::steady_clock::time_point _prev;

public:
	Clock();
	
	// Calculates the elapsed time since the previous time this function was called.
	double getTimeDeltaSeconds();
};
