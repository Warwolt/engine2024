#include <algorithm>
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
};

class AnimationSystem {
public:
	const std::vector<Animation>& animations(AnimationKey key);

	AnimationID start_animation(AnimationKey key, float length, float global_time);
	void stop_animation(AnimationID id);

private:
	std::unordered_map<AnimationKey, std::vector<Animation>> m_animations;
};

const std::vector<Animation>& AnimationSystem::animations(AnimationKey key) {
	return m_animations[key];
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
	AnimationSystem animation_system;
	AnimationID loading_text_id;

	for (float global_time = 0; global_time <= 25; global_time++) {
		/* Trigger animation */
		if (global_time == 5.0) {
			loading_text_id = animation_system.start_animation("loading_text", 3, global_time);
		}

		/* Stop animation */
		if (global_time == 15.0) {
			animation_system.stop_animation(loading_text_id);
		}

		/* Run animation */
		std::string str;
		for (const Animation& animation : animation_system.animations("loading_text")) {
			if (animation_is_active(animation, global_time)) {
				float local_time = local_animation_time(animation, global_time);
				str = loading_text(local_time);
			}
		}

		/* Render */
		printf("%.1f\t%s\n", global_time, str.c_str());
	}
}
