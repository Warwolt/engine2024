#include <engine/system/timeline_system.h>

namespace engine {

	bool Timeline::is_active(uint64_t global_time) const {
		if (this->repeats) {
			return this->start <= global_time;
		}
		else {
			return this->start <= global_time && global_time < (this->start + this->length);
		}
	}

	float Timeline::local_time(uint64_t global_time) const {
		return (float)((global_time - this->start) % this->length) / (float)this->length;
	}

	const std::vector<Timeline>& TimelineSystem::timelines(TimelineKey key) const {
		return m_timelines.contains(key) ? m_timelines.at(key) : m_empty_timeline;
	}

	std::optional<Timeline> TimelineSystem::most_recent_timeline(TimelineKey key) const {
		const std::vector<Timeline>& timelines = this->timelines(key);
		return timelines.empty() ? std::nullopt : std::make_optional(timelines.back());
	}

	TimelineID TimelineSystem::start_timeline(TimelineKey key, uint64_t start_time, uint64_t length) {
		return _start_timeline(key, length, start_time, true);
	}

	TimelineID TimelineSystem::start_one_shot_timeline(TimelineKey key, uint64_t start_time, uint64_t length) {
		return _start_timeline(key, length, start_time, false);
	}

	void TimelineSystem::remove_expired_timelines(uint64_t global_time) {
		for (auto& [key, timelines] : m_timelines) {
			std::erase_if(timelines, [global_time](const Timeline& animation) {
				return !animation.repeats && global_time > animation.start + animation.length;
			});
		}
	}

	TimelineID TimelineSystem::_start_timeline(TimelineKey key, uint64_t length, uint64_t start_time, bool repeats) {
		static int id_value;
		TimelineID id = { .key = key, .value = ++id_value };
		m_timelines[key].push_back(
			Timeline {
				.id = id,
				.start = start_time,
				.length = length,
				.repeats = repeats,
			}
		);
		return id;
	}

	void TimelineSystem::stop_timeline(TimelineID id) {
		std::erase_if(m_timelines[id.key], [id_value = id.value](const Timeline& animation) {
			return animation.id.value == id_value;
		});
	}

} // namespace engine
