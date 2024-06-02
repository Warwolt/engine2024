#include <algorithm>
#include <math.h>
#include <stdio.h>
#include <string>
#include <unordered_map>
#include <vector>

/* Animation Core */
using AnimationID = int;
struct Animation {
	AnimationID id;
	float start;
	float length;
	bool repeats;
};

float local_animation_time(const Animation& animation, float global_time) {
	return fmodf((global_time - animation.start), animation.length) / animation.length;
}

bool animation_is_active(const Animation& animation, float global_time) {
	if (animation.repeats) {
		return animation.start <= global_time;
	}
	else {
		return animation.start <= global_time && global_time < (animation.start + animation.length);
	}
}

AnimationID start_animation(std::vector<Animation>* animations, float length, float global_time) {
	static AnimationID id = 0;
	animations->push_back(
		Animation {
			.id = ++id,
			.start = global_time,
			.length = length,
			.repeats = true,
		}
	);
	return id;
}

void stop_animation(std::vector<Animation>* animations, AnimationID id) {
	std::erase_if(*animations, [id](const Animation& animation) {
		return animation.id == id;
	});
}

/* Application */
std::string loading_text(float t) {
	if (t < 1.0 / 3.0) {
		return "loading";
	}
	if (t < 2.0 / 3.0) {
		return "loading.";
	}
	return "loading..";
}

int main() {
	std::unordered_map<std::string, std::vector<Animation>> animations;

	AnimationID id = 0;
	for (float global_time = 0; global_time <= 25; global_time++) {
		/* Trigger animation */
		if (global_time == 5.0) {
			id = start_animation(&animations["loading_text"], 6, global_time);
		}

		/* Stop animation */
		if (global_time == 15.0) {
			stop_animation(&animations["loading_text"], id);
		}

		/* Run animation */
		std::string str;
		for (const Animation& animation : animations["loading_text"]) {
			if (animation_is_active(animation, global_time)) {
				float local_time = local_animation_time(animation, global_time);
				str = loading_text(local_time);
			}
		}

		/* Render */
		printf("%.1f\t%s\n", global_time, str.c_str());
	}
}
