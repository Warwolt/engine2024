#include <core/random.h>

#include <random>

namespace core {

	int random_int(int low, int high) {
		std::random_device dev;
		std::mt19937 rng(dev());
		std::uniform_int_distribution<std::mt19937::result_type> dist(low, high);
		return dist(rng);
	}

} // namespace core
