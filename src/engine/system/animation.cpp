#include <engine/system/animation.h>

namespace engine {

	bool Animation::is_playing(uint64_t global_time) const {
		if (this->repeats) {
			return this->start <= global_time;
		}
		else {
			return this->start <= global_time && global_time < (this->start + this->length);
		}
	}

	float Animation::local_time(uint64_t global_time) const {
		return (float)((global_time - this->start) % this->length) / (float)this->length;
	}

	const std::vector<Animation>& AnimationSystem::animations(AnimationKey key) const {
		return m_animations.contains(key) ? m_animations.at(key) : m_empty_animation;
	}

	std::optional<Animation> AnimationSystem::most_recent_animation(AnimationKey key) const {
		const std::vector<Animation>& animations = this->animations(key);
		return animations.empty() ? std::nullopt : std::make_optional(animations.back());
	}

	AnimationID AnimationSystem::start_animation(AnimationKey key, uint64_t length, uint64_t start_time) {
		return _start_animation(key, length, start_time, true);
	}

	AnimationID AnimationSystem::start_single_shot_animation(AnimationKey key, uint64_t length, uint64_t start_time) {
		return _start_animation(key, length, start_time, false);
	}

	void AnimationSystem::clear_old_animations(uint64_t global_time) {
		for (auto& [key, animations] : m_animations) {
			std::erase_if(animations, [global_time](const Animation& animation) {
				return !animation.repeats && global_time > animation.start + animation.length;
			});
		}
	}

	AnimationID AnimationSystem::_start_animation(AnimationKey key, uint64_t length, uint64_t start_time, bool repeats) {
		static int id_value;
		AnimationID id = { .key = key, .value = ++id_value };
		m_animations[key].push_back(
			Animation {
				.id = id,
				.start = start_time,
				.length = length,
				.repeats = repeats,
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
