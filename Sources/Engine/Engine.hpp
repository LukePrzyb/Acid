#pragma once

#include "Helpers/NonCopyable.hpp"
#include "Maths/ElapsedTime.hpp"
#include "Maths/Time.hpp"
#include "Module.hpp"
#include "App.hpp"
#include "Log.hpp"

namespace acid {
class ACID_EXPORT Delta {
public:
	void Update() {
		m_currentFrameTime = Time::Now();
		m_change = m_currentFrameTime - m_lastFrameTime;
		m_lastFrameTime = m_currentFrameTime;
	}

	Time m_currentFrameTime;
	Time m_lastFrameTime;
	Time m_change;
};

class ACID_EXPORT ChangePerSecond {
public:
	void Update(const Time &time) {
		m_valueTemp++;

		if (std::floor(time.AsSeconds()) > std::floor(m_valueTime.AsSeconds())) {
			m_value = m_valueTemp;
			m_valueTemp = 0;
		}

		m_valueTime = time;
	}

	uint32_t m_valueTemp = 0, m_value = 0;
	Time m_valueTime;
};

/**
 * @brief Main class for Acid, manages modules and updates. After creating your Engine object call {@link Engine#Run} to start.
 */
class ACID_EXPORT Engine : public virtual NonCopyable {
public:
	/**
	 * Gets the engines instance.
	 * @return The current engine instance.
	 */
	static Engine *Get() { return Instance; }

	/**
	 * Carries out the setup for basic engine components and the engine. Call {@link Engine#Run} after creating a instance.
	 * @param argv0 The first argument passed to main.
	 * @param emptyRegister If the module register will start empty.
	 */
	explicit Engine(std::string argv0, bool emptyRegister = false);

	~Engine();

	/**
	 * The update function for the updater.
	 * @return {@code EXIT_SUCCESS} or {@code EXIT_FAILURE}
	 */
	int32_t Run();

	/**
	 * Gets the first argument passed to main.
	 * @return The first argument passed to main.
	 */
	const std::string &GetArgv0() const { return m_argv0; };

	/**
	 * Gets the engine's version.
	 * @return The engine's version.
	 */
	const Version &GetVersion() const { return m_version; }

	/**
	 * Gets the current application.
	 * @return The renderer manager.
	 */
	App *GetApp() const { return m_app.get(); }

	/**
	 * Sets the current application to a new application.
	 * @param app The new application.
	 */
	void SetApp(std::unique_ptr<App> &&app) { m_app = std::move(app); }

	/**
	 * Gets the fps limit.
	 * @return The frame per second limit.
	 */
	float GetFpsLimit() const { return m_fpsLimit; }

	/**
	 * Sets the fps limit. -1 disables limits.
	 * @param fpsLimit The new frame per second limit.
	 */
	void SetFpsLimit(float fpsLimit) { m_fpsLimit = fpsLimit; }

	/**
	 * Gets if the engine is running.
	 * @return If the engine is running.
	 */
	bool IsRunning() const { return m_running; }

	/**
	 * Gets the delta (seconds) between updates.
	 * @return The delta between updates.
	 */
	const Time &GetDelta() const { return m_deltaUpdate.m_change; }

	/**
	 * Gets the delta (seconds) between renders.
	 * @return The delta between renders.
	 */
	const Time &GetDeltaRender() const { return m_deltaRender.m_change; }

	/**
	 * Gets the average UPS over a short interval.
	 * @return The updates per second.
	 */
	uint32_t GetUps() const { return m_ups.m_value; }

	/**
	 * Gets the average FPS over a short interval.
	 * @return The frames per second.
	 */
	uint32_t GetFps() const { return m_fps.m_value; }

	/**
	 * Requests the engine to stop the game-loop.
	 */
	void RequestClose() { m_running = false; }

private:
	void UpdateStage(Module::Stage stage);
	
	static Engine *Instance;

	std::string m_argv0;
	Version m_version;

	std::unique_ptr<App> m_app;

	float m_fpsLimit;
	bool m_running;

	Delta m_deltaUpdate;
	Delta m_deltaRender;
	ElapsedTime m_elapsedUpdate;
	ElapsedTime m_elapsedRender;

	ChangePerSecond m_ups, m_fps;
};
}
