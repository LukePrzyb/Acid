#include "Time.hpp"

#include "Files/Node.hpp"

namespace acid {
const std::chrono::time_point<std::chrono::high_resolution_clock> Time::Start(std::chrono::high_resolution_clock::now());

Time Time::Now() {
	return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - Start);
}

std::string Time::GetDateTime(const std::string &format) {
	auto now = std::chrono::system_clock::now();
	auto timeT = std::chrono::system_clock::to_time_t(now);

	std::stringstream ss;
	ss << std::put_time(std::localtime(&timeT), format.c_str());
	return ss.str();
}

bool Time::operator==(const Time &other) const {
	return m_microseconds == other.m_microseconds;
}

bool Time::operator!=(const Time &other) const {
	return m_microseconds != other.m_microseconds;
}

bool Time::operator<(const Time &other) const {
	return m_microseconds < other.m_microseconds;
}

bool Time::operator<=(const Time &other) const {
	return m_microseconds <= other.m_microseconds;
}

bool Time::operator>(const Time &other) const {
	return m_microseconds > other.m_microseconds;
}

bool Time::operator>=(const Time &other) const {
	return m_microseconds >= other.m_microseconds;
}

Time Time::operator-() const {
	return Time(-m_microseconds);
}

Time operator+(const Time &left, const Time &right) {
	return left.m_microseconds + right.m_microseconds;
}

Time operator-(const Time &left, const Time &right) {
	return left.m_microseconds - right.m_microseconds;
}

Time operator*(const Time &left, float right) {
	return left.m_microseconds * right;
}

Time operator*(const Time &left, int64_t right) {
	return left.m_microseconds * right;
}

Time operator*(float left, const Time &right) {
	return right * left;
}

Time operator*(int64_t left, const Time &right) {
	return right * left;
}

Time operator/(const Time &left, float right) {
	return left.m_microseconds / right;
}

Time operator/(const Time &left, int64_t right) {
	return left.m_microseconds / right;
}

double operator/(const Time &left, const Time &right) {
	return static_cast<double>(left.m_microseconds.count()) / static_cast<double>(right.m_microseconds.count());
}

Time &Time::operator+=(const Time &other) {
	return *this = *this + other;
}

Time &Time::operator-=(const Time &other) {
	return *this = *this - other;
}

Time &Time::operator*=(float other) {
	return *this = *this * other;
}

Time &Time::operator*=(int64_t other) {
	return *this = *this * other;
}

Time &Time::operator/=(float other) {
	return *this = *this / other;
}

Time &Time::operator/=(int64_t other) {
	return *this = *this / other;
}

const Node &operator>>(const Node &node, Time &time) {
	time.m_microseconds = std::chrono::microseconds(node.Get<int64_t>());
	return node;
}

Node &operator<<(Node &node, const Time &time) {
	node.Set(time.m_microseconds.count());
	return node;
}
}
