#pragma once

#include <mutex>
#include "Engine/Engine.hpp"
#include "Helpers/Delegate.hpp"
#include "Maths/Time.hpp"

namespace acid {
class ACID_EXPORT Timer {
	friend class Timers;
public:
	Timer(const Time &interval, const std::optional<uint32_t> &repeat) :
		m_interval(interval),
		m_next(Time::Now() + m_interval),
		m_repeat(repeat) {
	}

	const Time &GetInterval() const { return m_interval; }
	const std::optional<uint32_t> &GetRepeat() const { return m_repeat; }
	bool IsDestroyed() const { return m_destroyed; }
	void Destroy() { m_destroyed = true; }
	Delegate<void()> &OnTick() { return m_onTick; };

private:
	Time m_interval;
	Time m_next;
	std::optional<uint32_t> m_repeat;
	bool m_destroyed = false;
	Delegate<void()> m_onTick;
};

/**
 * @brief Module used for timed events.
 */
class ACID_EXPORT Timers : public Module::Registrar<Timers> {
public:
	Timers();

	~Timers();

	void Update() override;

	template<typename ...Args>
	Timer *Once(const Time &delay, std::function<void()> &&function, Args ...args) {
		std::unique_lock<std::mutex> lock(m_mutex);
		auto instance = std::make_unique<Timer>(delay, 1);
		instance->m_onTick.Add(std::move(function), args...);
		m_timers.emplace_back(std::move(instance));
		m_condition.notify_all();
		return instance.get();
	}

	template<typename ...Args>
	Timer *Every(const Time &interval, std::function<void()> &&function, Args ...args) {
		std::unique_lock<std::mutex> lock(m_mutex);
		auto instance = std::make_unique<Timer>(interval, std::nullopt);
		instance->m_onTick.Add(std::move(function), args...);
		m_timers.emplace_back(std::move(instance));
		m_condition.notify_all();
		return instance.get();
	}

	template<typename ...Args>
	Timer *Repeat(const Time &interval, uint32_t repeat, std::function<void()> &&function, Args ...args) {
		std::unique_lock<std::mutex> lock(m_mutex);
		auto instance = std::make_unique<Timer>(interval, repeat);
		instance->m_onTick.Add(std::move(function), args...);
		m_timers.emplace_back(std::move(instance));
		m_condition.notify_all();
		return instance.get();
	}

private:
	void ThreadRun();

	std::vector<std::unique_ptr<Timer>> m_timers;

	bool m_stop = false;
	std::thread m_worker;

	std::mutex m_mutex;
	std::condition_variable m_condition;
};
}
