#include <engine/animation.h>

namespace engine {

	bool Animation::is_playing(float global_time) const {
		if (this->repeats) {
			return this->start <= global_time;
		}
		else {
			return this->start <= global_time && global_time < (this->start + this->length);
		}
	}

	float Animation::local_time(float global_time) const {
		return fmodf((global_time - this->start), this->length) / this->length;
	}

	std::vector<Animation> AnimationSystem::animations(AnimationKey key) {
		return m_animations[key];
	}

	AnimationID AnimationSystem::start_animation(AnimationKey key, float length, float global_time) {
		static int id_value;
		AnimationID id = { .key = key, .value = ++id_value };
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

} // namespace engine
