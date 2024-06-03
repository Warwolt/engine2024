#pragma once

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
		std::vector<Animation> animations(AnimationKey key);

		AnimationID start_animation(AnimationKey key, float length, float global_time);
		void stop_animation(AnimationID id);

	private:
		std::unordered_map<AnimationKey, std::vector<Animation>> m_animations;
	};

} // namespace engine
