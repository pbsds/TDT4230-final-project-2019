#include <chrono>
#include "timeutils.h"


// Calculates the elapsed time since the previous time this function was called.
double getTimeDeltaSeconds() {
	static std::chrono::steady_clock::time_point _prev
		= std::chrono::steady_clock::now();
	
	std::chrono::steady_clock::time_point now
		= std::chrono::steady_clock::now();

	// nanoseconds delta
	long long td = std::chrono::duration_cast<std::chrono::nanoseconds>(now - _prev).count();
	
	_prev = now;

	return ((double)td) / 1000000000.0; // return as seconds
}
