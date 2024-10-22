#include <engine/system/timeline_system.h>

namespace engine {

	bool Timeline::is_active(uint64_t global_time) const {
		if (this->repeats) {
			return this->start <= global_time;
		}
		return this->start <= global_time && global_time < (this->start + this->length);
	}

	bool Timeline::is_expired(uint64_t global_time) const {
		if (this->repeats) {
			return false;
		}
		return global_time >= (this->start + this->length);
	}

	float Timeline::local_time(uint64_t global_time) const {
		if (this->is_active(global_time)) {
			// shift global time to local timeline origin, then normalize
			return (float)((global_time - this->start) % this->length) / (float)this->length;
		}
		if (this->is_expired(global_time)) {
			return 1.0f;
		}
		return 0.0f;
	}

	std::optional<Timeline> TimelineSystem::timeline(TimelineID id) const {
		if (auto it = m_timelines.find(id); it != m_timelines.end()) {
			return it->second;
		}
		return std::nullopt;
	}

	TimelineID TimelineSystem::add_repeating_timeline(uint64_t start_time, uint64_t length) {
		return _start_timeline(length, start_time, true);
	}

	TimelineID TimelineSystem::add_one_shot_timeline(uint64_t start_time, uint64_t length) {
		return _start_timeline(length, start_time, false);
	}

	void TimelineSystem::remove_expired_timelines(uint64_t global_time) {
		for (auto it = m_timelines.begin(); it != m_timelines.end();) {
			auto& [id, timeline] = *it;
			if (timeline.is_expired(global_time)) {
				it = m_timelines.erase(it);
			}
			else {
				it++;
			}
		}
	}

	TimelineID TimelineSystem::_start_timeline(uint64_t length, uint64_t start_time, bool repeats) {
		static int id_value = 0;
		TimelineID id = TimelineID { ++id_value };
		m_timelines.insert({
			id,
			Timeline {
				.start = start_time,
				.length = length,
				.repeats = repeats,
			},
		});
		return id;
	}

	void TimelineSystem::remove_timeline(TimelineID id) {
		m_timelines.erase(id);
	}

} // namespace engine
