#include "Joysticks.hpp"

#include <GLFW/glfw3.h>

namespace acid {
void CallbackJoystick(int32_t id, int32_t event) {
	if (event == GLFW_CONNECTED) {
		Log::Out("Joystick connected: '", glfwGetJoystickName(id), "' to ", id, '\n');
		Joysticks::JoystickImpl joystick = {};
		joystick.m_name = glfwGetJoystickName(id);
		Joysticks::Get()->m_connected.emplace(id, joystick);
		Joysticks::Get()->m_onConnect(id, true);
	} else if (event == GLFW_DISCONNECTED) {
		Log::Out("Joystick disconnected from ", id, '\n');
		Joysticks::Get()->m_connected.erase(id);
		Joysticks::Get()->m_onConnect(id, false);
	}
}

Joysticks::Joysticks() {
	glfwSetJoystickCallback(CallbackJoystick);

	for (uint32_t i = 0; i < GLFW_JOYSTICK_LAST; i++) {
		if (glfwJoystickPresent(i)) {
			JoystickImpl joystick = {};
			joystick.m_name = glfwGetJoystickName(i);
			m_connected.emplace(i, joystick);
			m_onConnect(i, true);
		}
	}
}

void Joysticks::Update() {
	for (auto &[port, joystick] : m_connected) {
		int32_t axeCount = 0;
		auto axes = glfwGetJoystickAxes(port, &axeCount);
		joystick.m_axes.resize(static_cast<std::size_t>(axeCount));

		for (uint32_t i = 0; i < static_cast<uint32_t>(axeCount); i++) {
			if (joystick.m_axes[i] != axes[i]) {
				joystick.m_axes[i] = axes[i];
				m_onAxis(port, i, joystick.m_axes[i]);
			}
		}

		int32_t buttonCount = 0;
		auto buttons = glfwGetJoystickButtons(port, &buttonCount);
		joystick.m_buttons.resize(static_cast<std::size_t>(buttonCount));

		for (uint32_t i = 0; i < static_cast<uint32_t>(buttonCount); i++) {
			if (buttons[i] != GLFW_RELEASE && joystick.m_buttons[i] != InputAction::Release) {
				joystick.m_buttons[i] = InputAction::Repeat;
			} else if (joystick.m_buttons[i] != static_cast<InputAction>(buttons[i])) {
				joystick.m_buttons[i] = static_cast<InputAction>(buttons[i]);
				m_onButton(port, i, joystick.m_buttons[i]);
			}
		}

		int32_t hatCount = 0;
		auto hats = glfwGetJoystickHats(port, &hatCount);
		joystick.m_hats.resize(static_cast<std::size_t>(hatCount));

		for (uint32_t i = 0; i < static_cast<uint32_t>(hatCount); i++) {
			if (joystick.m_hats[i] != MakeBitMask<JoystickHatValue>(hats[i])) {
				joystick.m_hats[i] = MakeBitMask<JoystickHatValue>(hats[i]);
				m_onHat(port, i, joystick.m_hats[i]);
			}
		}
	}
}

bool Joysticks::IsConnected(JoystickPort port) const {
	return GetJoystick(port).has_value();
}

std::string Joysticks::GetName(JoystickPort port) const {
	auto joystick = GetJoystick(port);
	return joystick ? joystick->m_name : "";
}

std::size_t Joysticks::GetAxisCount(JoystickPort port) const {
	auto joystick = GetJoystick(port);
	return joystick ? static_cast<std::size_t>(joystick->m_axes.size()) : 0;
}

std::size_t Joysticks::GetButtonCount(JoystickPort port) const {
	auto joystick = GetJoystick(port);
	return joystick ? static_cast<std::size_t>(joystick->m_buttons.size()) : 0;
}

std::size_t Joysticks::GetHatCount(JoystickPort port) const {
	auto joystick = GetJoystick(port);
	return joystick ? static_cast<std::size_t>(joystick->m_hats.size()) : 0;
}

float Joysticks::GetAxis(JoystickPort port, JoystickAxis axis) const {
	auto joystick = GetJoystick(port);

	if (!joystick || axis > joystick->m_axes.size()) {
		return 0.0f;
	}

	return joystick->m_axes[axis];
}

InputAction Joysticks::GetButton(JoystickPort port, JoystickButton button) const {
	auto joystick = GetJoystick(port);

	if (!joystick || button > joystick->m_buttons.size()) {
		return InputAction::Release;
	}

	return joystick->m_buttons[button];
}

BitMask<JoystickHatValue> Joysticks::GetHat(JoystickPort port, JoystickHat hat) const {
	auto joystick = GetJoystick(port);

	if (!joystick || hat > joystick->m_hats.size()) {
		return JoystickHatValue::Centered;
	}

	return joystick->m_hats[hat];
}

std::optional<Joysticks::JoystickImpl> Joysticks::GetJoystick(JoystickPort port) const {
	auto it = m_connected.find(port);

	if (it == m_connected.end()) {
		return std::nullopt;
	}

	return it->second;
}
}
