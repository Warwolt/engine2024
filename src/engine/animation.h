#pragma once

#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace engine {

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

		bool is_playing(float global_time) const;
		float local_time(float global_time) const;
	};

	class AnimationSystem {
	public:
		const std::vector<Animation>& animations(AnimationKey key) const;
		std::optional<Animation> most_recent_animation(AnimationKey key) const;

		AnimationID start_animation(AnimationKey key, float length, float start_time);
		AnimationID start_single_shot_animation(AnimationKey key, float length, float start_time);
		void stop_animation(AnimationID id);
		void clear_old_animations(float global_time);

	private:
		AnimationID _start_animation(AnimationKey key, float length, float start_time, bool repeats);

		const std::vector<Animation> m_empty_animation;
		std::unordered_map<AnimationKey, std::vector<Animation>> m_animations;
	};

} // namespace engine
