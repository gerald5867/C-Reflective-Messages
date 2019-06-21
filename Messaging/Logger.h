#pragma once
#if defined(_WIN32) || defined(_MSC_VER)
#include <windows.h>
#endif
#include <iostream>
#include <iomanip>
#include <string>
#include <chrono>
#include <vector>
#include "Debugbreak.h"
#include "ExtendedEnum.h"


namespace logging_utils {
	
	DECLENUMEX(LogLevel, int, eDEBUG, eTRACE, eWARNING, eERROR, eFATAL);
	DECLENUMEX(LogDestination, int, eCONSOLE, eFILE, eIDEOUTPUT);
	DECLENUMEX(LogConsoleColor, int, eDEFAULT, eRED, eGREEN, eLIGHT_RED, eYELLOW, eBLUE)
	
	class Logger final {
	public:
		Logger() = default;
		~Logger() {
			if (m_pLogFile != nullptr) {
				::fflush(m_pLogFile);
				::fclose(m_pLogFile);
				m_pLogFile = nullptr;
			}
		}

		void SetLogFile(const char* fileName, const bool ClearAlways = false) { 
			m_logFileName = fileName;
			if (m_pLogFile != nullptr) {
				::fclose(m_pLogFile);
			}
			if (ClearAlways) {
				m_pLogFile = ::fopen(fileName, "w+");
			} else {
				m_pLogFile = ::fopen(fileName, "r");
				if (m_pLogFile == nullptr) {
					m_pLogFile = ::fopen(fileName, "w+");
				}
				else {
					m_pLogFile = ::fopen(fileName, "a");
				}
			}
		}

		void ClearLogFile() {
			if (m_pLogFile != nullptr) {
				::freopen(m_logFileName.c_str(), "w+", m_pLogFile);
			} else {
				m_pLogFile = ::fopen(m_logFileName.c_str(), "w+");
			}
		}

		void AddLogDestination(LogDestination dest) { m_logDests.emplace_back(dest); }
		void EnableDisplayErrorAsMessageBox() { m_msgBoxEnabled = true;  }

		bool HasDestination(LogDestination dest) const {
			return std::find(m_logDests.cbegin(), m_logDests.cend(), dest) != m_logDests.cend();
		}

		template<typename... LogArguments>
		void LogLine(const char* format, const char* file, const int line, LogLevel level, LogArguments... arguments) {
			if (level < m_logLevel) {
				return;
			}
			static constexpr std::size_t FormatBufferSize = 1024 * 100;
			m_formatBuffer.resize(FormatBufferSize);
			std::time_t now = static_cast<std::time_t>(std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()));
			std::size_t currentOffset = std::strftime(&m_formatBuffer[0], m_formatBuffer.size(), "%Y-%m-%d %H:%M:%S", std::localtime(&now));
			currentOffset += ::snprintf(m_formatBuffer.data() + currentOffset, m_formatBuffer.size() - currentOffset,
				" %s : [%s on Line : %d] ", level.ToString().data() + 1, file, line);
			currentOffset += ::snprintf(m_formatBuffer.data() + currentOffset, m_formatBuffer.size() - currentOffset, format, arguments...);
			if (currentOffset - 1 < m_formatBuffer.size()) {
				m_formatBuffer[currentOffset++] = '\n';
			}
			m_formatBuffer.resize(currentOffset);
			FlushLine(level); // for now flush every line
		}

		LogLevel GetLogLevel() const noexcept { return m_logLevel; }
		void SetLogLevel(LogLevel level) { m_logLevel = level; }

		template<std::size_t N>
		constexpr static decltype(auto) GetFileNameFromFilePath(const char(&file)[N]) noexcept {
			std::size_t startIndex = 0;
			for (std::int64_t i = N - 2; i >= 0; --i) {
				if (file[i] == '\\' || file[i] == '/') {
					startIndex = static_cast<std::size_t>(i + 1);
					break;
				}
			}
			messaging::ConstexprArray<char, N> result;
			for (std::size_t i = startIndex; i < N - 1; ++i) {
				result[i - startIndex] = file[i];
			}
			return result;
		}
	private:
		void FlushLine(LogLevel level) {
			if (HasDestination(LogDestination::eCONSOLE)) {
				SwitchCoutColor(LogLevelToCoutColor(level));
				std::cout << std::fixed << std::setprecision(4) << m_formatBuffer << std::endl;
				SwitchCoutColor(LogConsoleColor::eDEFAULT);
			}
			if (m_pLogFile && HasDestination(LogDestination::eFILE)) {
				//TODO CHECK FOR MAX LOG FILE SIZE!!!!
				::fwrite(m_formatBuffer.data(), m_formatBuffer.size(), 1, m_pLogFile);
				::fflush(m_pLogFile);
			}
#if defined(_WIN32) || defined(_MSC_VER)
			if (level >= LogLevel::eERROR && m_msgBoxEnabled) {
				MessageBoxA(NULL, m_formatBuffer.data(), NULL, MB_OK);
			}
			if (HasDestination(LogDestination::eIDEOUTPUT)) {
				OutputDebugStringA(m_formatBuffer.c_str());
			}
#endif
		}

		static LogConsoleColor LogLevelToCoutColor(LogLevel level) {
			switch (level) {
			case LogLevel::eDEBUG: return LogConsoleColor::eGREEN;
			case LogLevel::eTRACE: return LogConsoleColor::eGREEN;
			case LogLevel::eERROR: return LogConsoleColor::eLIGHT_RED;
			case LogLevel::eWARNING: return LogConsoleColor::eYELLOW;
			case LogLevel::eFATAL: return LogConsoleColor::eRED;
			default: return LogConsoleColor::eDEFAULT;
			}
		}

		static void SwitchCoutColor(LogConsoleColor color) {
#if defined(_WIN32) || defined(_MSC_VER)
			std::uint16_t newColor = 0x0007;
			switch (color) {
			case LogConsoleColor::eBLUE:
				newColor = 0x0001;
				break;
			case LogConsoleColor::eRED:
				newColor = 0x0004;
				break;
			case LogConsoleColor::eYELLOW:
				newColor = 0x0001 | 0x0004;
				break;
			case LogConsoleColor::eGREEN:
				newColor = 0x0002;
				break;
			case LogConsoleColor::eLIGHT_RED:
				newColor = 0x0004 | 0x0002;
				break;
			default:
				break;
			}
			HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
			SetConsoleTextAttribute(hConsole, newColor);
#else
			static constexpr const char* const clearColor = "\033[2J";
			std::cout << clearColor;
			switch (color) {
			case LogConsoleColor::eBLUE:
				std::cout << "\033[34m";
				break;
			case LogConsoleColor::eLIGHT_RED:
			case LogConsoleColor::eRED:
				std::cout << "\033[31m";
				break;
			case LogConsoleColor::eYELLOW:
				std::cout << "\033[33m";
				break;
			case LogConsoleColor::eGREEN:
				std::cout << "\033[32m";
				break;
			default:
				break;
			}
#endif
		}

	private:
		LogLevel m_logLevel;
		bool m_msgBoxEnabled = false;
		FILE* m_pLogFile = nullptr;
		std::vector<LogDestination> m_logDests;
		std::string m_formatBuffer;
		std::string m_logFileName;
	};
}

#if defined(_DEBUG)
#define DEBUGBREAKONERROR(level) \
	if(level >= logging_utils::LogLevel::eERROR) { \
		GPG_DEBUGBREAK(); \
	} 
#else
#define DEBUGBREAKONERROR(level)
#endif

#define LOG_IF(logger, format, level, ...) \
	if(logger && level >= (logger)->GetLogLevel()) { \
		(logger)->LogLine(format, logging_utils::Logger::GetFileNameFromFilePath(__FILE__).data(), __LINE__, level, __VA_ARGS__); \
	} \
	DEBUGBREAKONERROR(level) 

#define LOGDEBUG(logger, format, ...)   LOG_IF(logger, format, logging_utils::LogLevel::eDEBUG,   __VA_ARGS__)
#define LOGTRACE(logger, format, ...)   LOG_IF(logger, format, logging_utils::LogLevel::eTRACE,   __VA_ARGS__) 
#define LOGWARNING(logger, format, ...) LOG_IF(logger, format, logging_utils::LogLevel::eWARNING, __VA_ARGS__) 
#define LOGERROR(logger, format, ...)   LOG_IF(logger, format, logging_utils::LogLevel::eERROR,   __VA_ARGS__) 
#define LOGFATAL(logger, format, ...)   LOG_IF(logger, format, logging_utils::LogLevel::eFATAL,   __VA_ARGS__)

#ifndef _DEBUG
	#define ASSERTEX(condition, logger)
#else 
	#define ASSERTEX(condition, logger) \
		if(!(condition)) { \
			LOGFATAL(logger, \
			"################################################\n" \
			"################# ASSERTION FAILED##############\n" \
			"################################################\n" \
			#condition##"\n"\
			GPG_DEBUGBREAK(); \
		}
#endif