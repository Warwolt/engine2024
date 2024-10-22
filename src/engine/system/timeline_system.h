#pragma once

#include <core/newtype.h>

#include <optional>
#include <stdint.h>
#include <unordered_map>

namespace engine {
	DEFINE_NEWTYPE(TimelineID, int);
} // namespace engine
DEFINE_NEWTYPE_HASH_IMPL(engine::TimelineID, int);

namespace engine {

	struct Timeline {
		TimelineID id;
		uint64_t start;
		uint64_t length;
		bool repeats;

		bool is_active(uint64_t global_time) const; // global_time in range [start, start + length]
		float local_time(uint64_t global_time) const; // in range [0,1]
	};

	class TimelineSystem {
	public:
		std::optional<Timeline> timeline(TimelineID id) const;

		TimelineID add_repeating_timeline(uint64_t start_time, uint64_t length);
		TimelineID add_one_shot_timeline(uint64_t start_time, uint64_t length);
		void remove_timeline(TimelineID id);
		void remove_expired_timelines(uint64_t global_time);

	private:
		std::unordered_map<TimelineID, Timeline> m_timelines;

		TimelineID _start_timeline(uint64_t length, uint64_t start_time, bool repeats);
	};

} // namespace engine
