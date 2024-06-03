#include <algorithm>
#include <iterator>
#include <math.h>
#include <stdio.h>
#include <string>
#include <unordered_map>
#include <vector>

#include <platform/animation.h>

/* Application */
std::string loading_text(float t) {
	if (t < 1.0 / 3.0) {
		return "loading";
	}
	if (t < 2.0 / 3.0) {
		return "loading.";
	}
	else /* t < 3.0 / 3.0 */ {
		return "loading..";
	}
}

int main() {
	platform::AnimationSystem animations;
	platform::AnimationID loading_text_id;

	for (float time = 0; time <= 25; time++) {
		/* Trigger animation */
		if (time == 5.0) {
			float length = 6.0;
			loading_text_id = animations.start_animation("loading_text", length, time);
		}

		/* Stop animation */
		if (time == 18.0) {
			animations.stop_animation(loading_text_id);
		}

		/* Run animation */
		std::string str;
		for (const platform::Animation& animation : animations.animations("loading_text")) {
			if (animation.is_playing(time)) {
				str = loading_text(animation.local_time(time));
			}
		}

		/* Render */
		printf("%.1f\t%s\n", time, str.c_str());
	}
}
