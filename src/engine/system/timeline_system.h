#pragma once

#include <optional>
#include <stdint.h>
#include <string>
#include <unordered_map>
#include <vector>

namespace engine {

	using TimelineKey = std::string;

	struct TimelineID {
		TimelineKey key;
		int value = 0;
	};

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
		const std::vector<Timeline>& timelines(TimelineKey key) const;
		std::optional<Timeline> most_recent_timeline(TimelineKey key) const;

		TimelineID add_repeating_timeline(TimelineKey key, uint64_t start_time, uint64_t length);
		TimelineID add_one_shot_timeline(TimelineKey key, uint64_t start_time, uint64_t length);
		void remove_timeline(TimelineID id);
		void remove_expired_timelines(uint64_t global_time);

	private:
		TimelineID _start_timeline(TimelineKey key, uint64_t length, uint64_t start_time, bool repeats);

		const std::vector<Timeline> m_empty_timeline;
		std::unordered_map<TimelineKey, std::vector<Timeline>> m_timelines;
	};

} // namespace engine
