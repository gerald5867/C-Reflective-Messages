#pragma once
#include <chrono>
namespace utils {
	class TimeOutClock {
	public:
		using ClockT = std::chrono::system_clock;
		explicit TimeOutClock(const std::chrono::milliseconds& timeOut) {
			m_start = ClockT::now();
			SetTimeOut(timeOut);
		}
		bool IsExpired() const noexcept {
			return std::chrono::duration_cast<std::chrono::milliseconds>(ClockT::now() - m_start) >= m_timeOut;
		}
		void Reset() {
			m_start = ClockT::now();
		}
		void SetTimeOut(const std::chrono::milliseconds& timeOut) {
			m_timeOut = timeOut;
		}
	private:
		std::chrono::milliseconds m_timeOut;
		ClockT::time_point m_start;
	};
}