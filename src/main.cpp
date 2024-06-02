#include <algorithm>
#include <iterator>
#include <math.h>
#include <stdio.h>
#include <string>
#include <unordered_map>
#include <vector>

/* Animation Core */
using AnimationKey = std::string;
struct AnimationID {
	AnimationKey key;
	int value = 0;
};
struct Animation {
	AnimationID id;
	float start;
	float length;
	bool repeats;

	float local_time(float global_time) const;
};

float Animation::local_time(float global_time) const {
	return fmodf((global_time - this->start), this->length) / this->length;
}

class AnimationSystem {
public:
	std::vector<Animation> playing_animations(AnimationKey key, float global_time);

	AnimationID start_animation(AnimationKey key, float length, float global_time);
	void stop_animation(AnimationID id);

private:
	static bool _animation_is_active(const Animation& animation, float global_time);

	std::unordered_map<AnimationKey, std::vector<Animation>> m_animations;
};

std::vector<Animation> AnimationSystem::playing_animations(AnimationKey key, float global_time) {
	std::vector<Animation> playing_animations;
	for (const Animation& animation : m_animations[key]) {
		if (_animation_is_active(animation, global_time)) {
			playing_animations.push_back(animation);
		}
	}
	return playing_animations;
}

AnimationID AnimationSystem::start_animation(AnimationKey key, float length, float global_time) {
	static int id_value;
	AnimationID id = {.key = key, .value = ++id_value};
	m_animations[key].push_back(
		Animation {
			.id = id,
			.start = global_time,
			.length = length,
			.repeats = true,
		}
	);
	return id;
}

void AnimationSystem::stop_animation(AnimationID id) {
	std::erase_if(m_animations[id.key], [id_value = id.value](const Animation& animation) {
		return animation.id.value == id_value;
	});
}

bool AnimationSystem::_animation_is_active(const Animation& animation, float global_time) {
	if (animation.repeats) {
		return animation.start <= global_time;
	}
	else {
		return animation.start <= global_time && global_time < (animation.start + animation.length);
	}
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
	AnimationSystem animations;
	AnimationID loading_text_id;

	for (float time = 0; time <= 25; time++) {
		/* Trigger animation */
		if (time == 5.0) {
			loading_text_id = animations.start_animation("loading_text", 3, time);
		}

		/* Stop animation */
		if (time == 15.0) {
			animations.stop_animation(loading_text_id);
		}

		/* Run animation */
		std::string str;
		for (const Animation& animation : animations.playing_animations("loading_text", time)) {
			str = loading_text(animation.local_time(time));
		}

		/* Render */
		printf("%.1f\t%s\n", time, str.c_str());
	}
}
