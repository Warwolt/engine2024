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

	std::optional<Timeline> TimelineSystem::timeline(TimelineID id) const {
		return std::nullopt;
	}

	TimelineID TimelineSystem::add_repeating_timeline(uint64_t start_time, uint64_t length) {
		// return _start_timeline(key, length, start_time, true);
		return TimelineID { 0 };
	}

	TimelineID TimelineSystem::add_one_shot_timeline(uint64_t start_time, uint64_t length) {
		// return _start_timeline(key, length, start_time, false);
		return TimelineID { 0 };
	}

	void TimelineSystem::remove_expired_timelines(uint64_t global_time) {
		//for (auto& [key, timelines] : m_timelines_OLD) {
		//	std::erase_if(timelines, [global_time](const Timeline& timeline) {
		//		return !timeline.repeats && global_time > timeline.start + timeline.length;
		//	});
		//}
	}

	TimelineID TimelineSystem::_start_timeline(uint64_t length, uint64_t start_time, bool repeats) {
		//static int id_value;
		//TimelineID id = { .key = key, .value = ++id_value };
		//m_timelines_OLD[key].push_back(
		//	Timeline {
		//		.id = id,
		//		.start = start_time,
		//		.length = length,
		//		.repeats = repeats,
		//	}
		//);
		//return id;
		return TimelineID { 0 };
	}

	void TimelineSystem::remove_timeline(TimelineID id) {
		//std::erase_if(m_timelines_OLD[id.key], [id_value = id.value](const Timeline& timeline) {
		//	return timeline.id.value == id_value;
		//});
	}

} // namespace engine
