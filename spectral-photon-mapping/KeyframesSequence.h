#pragma once
#include <vector>
#include <assert.h>

#include "common.h"

template<class T>
class KeyframesSequence {
	template<class T>
	struct KeyFrame {
		T value;
		unsigned int time;
	};
	float timescale = 1.0f;
	std::vector<KeyFrame<T>> keyframes;
public:
	void insert(const T& value, unsigned int time) const {
		const auto it = keyframes.begin();
		for (; it != keyframes.end(); ++it) {
			if (it->time > time)
				break;
		}
		keyframes.insert(it, {value, time});
	}
	void normalizeTimescale() {
		int maxTime = 0;
		for (const auto& keyframe : keyframes) {
			maxTime = std::max(maxTime, keyframe.time);
		}
		timescale = 1.0f / maxTime;
	}
	T valueAt(float time) const {
		assert(!keyframes.empty());
		if (keyframes.front().time * timescale >= time)
			return keyframes.front().value;
		if (keyframes.back().time * timescale <= time)
			return keyframes.back().value;
		int l = 0;
		int r = keyframes.size() - 1;
		while (r - l > 1) {
			int i = (l + r) / 2;
			if (keyframes[i].time < time)
				l = i;
			else
				r = i;
		}
		const auto& left = keyframes[l];
		const auto& right = keyframes[r];
		float dt = timescale * (right.time - left.time);
		return glm::lerp(left.value, right.value, glm::clamp((time - timescale * left.time) / dt, 0.0f, 1.0f));
	}
};