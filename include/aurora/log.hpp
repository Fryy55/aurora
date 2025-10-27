#pragma once

#include <aurora/singletons/TargetManager.hpp>

#include <iostream>
#include <print>
#include <regex>
#include <fstream>


namespace aurora {

class log final {
public:
	// ..............
	log() = delete;
	log(log const&) = delete;
	log& operator=(log const&) = delete;
	log(log&&) = delete;
	log& operator=(log&&) = delete;
	~log() = delete;

	// Log level
	enum class LogLevel : std::uint8_t {
		Debug,
		Info,
		Warn,
		Error
	};

	[[nodiscard]] static LogLevel getLogLevel() noexcept { return s_logLevel; }
	static void setLogLevel(LogLevel logLevel) noexcept { s_logLevel = logLevel; }
	[[nodiscard]] static LogLevel getFileLogLevel() noexcept { return s_fileLogLevel; }
	static void setFileLogLevel(LogLevel logLevel) noexcept { s_fileLogLevel = logLevel; }

private:
	static LogLevel s_logLevel;
	static LogLevel s_fileLogLevel;

public:
	// Time locale
	[[nodiscard]] static bool get12hTimeEnabled() noexcept { return s_use12hTime; }
	static void set12hTimeEnabled(bool on) noexcept { s_use12hTime = on; }

private:
	static bool s_use12hTime;

public:
	// Max source length
	[[nodiscard]] static std::uint8_t getMaxSourceLength() noexcept { return s_maxSourceLength; }
	static void setMaxSourceLength(std::uint8_t maxSourceLength) noexcept {
		s_maxSourceLength = maxSourceLength;
	}

private:
	static std::uint8_t s_maxSourceLength;

public:
	// Log to stderr
	[[nodiscard]] static bool getLogToStderrEnabled() noexcept { return s_logToStderr; }
	static void setLogToStderrEnabled(bool on) noexcept { s_logToStderr = on; }

private:
	static bool s_logToStderr;


// Logging functions
private:
	using LogStates = std::pair<bool, bool>;

public:
	template <typename ...Args>
	static void debug(std::format_string<Args...> formatString, Args&&... args) noexcept {
		LogStates states{};

		if (s_logLevel == LogLevel::Debug)
			states.first = true;
		if (s_fileLogLevel == LogLevel::Debug)
			states.second = true;

		if (states.first || states.second)
			log_impl(states, LogLevel::Debug, formatString, std::forward<Args>(args)...);

		return;
	}
	template <typename ...Args>
	static void info(std::format_string<Args...> formatString, Args&&... args) noexcept {
		LogStates states{};

		if (s_logLevel == LogLevel::Debug || s_logLevel == LogLevel::Info)
			states.first = true;
		if (s_fileLogLevel == LogLevel::Debug || s_fileLogLevel == LogLevel::Info)
			states.second = true;

		if (states.first || states.second)
			log_impl(states, LogLevel::Info, formatString, std::forward<Args>(args)...);

		return;
	}
	template <typename ...Args>
	static void warn(std::format_string<Args...> formatString, Args&&... args) noexcept {
		LogStates states{};

		if (s_logLevel != LogLevel::Error)
			states.first = true;
		if (s_fileLogLevel != LogLevel::Error)
			states.second = true;

		if (states.first || states.second)
			log_impl(states, LogLevel::Warn, formatString, std::forward<Args>(args)...);

		return;
	}
	template <typename ...Args>
	static void error(std::format_string<Args...> formatString, Args&&... args) noexcept {
		log_impl({ true, true }, LogLevel::Error, formatString, std::forward<Args>(args)...);

		return;
	}

private:
	template <typename ...Args>
	static void log_impl(
		LogStates const& states,
		LogLevel logLevel,
		std::format_string<Args...> formatString,
		Args&&... args
	) noexcept {
		auto string = logString(
			logLevel,
			std::format(formatString, std::forward<Args>(args)...)
		);

		if (states.first)
			std::print(s_logToStderr ? std::cerr : std::cout, "{}", string);
		if (states.second) {
			static std::regex const ansiRegex(R"(\x1B\[[\d;]*m)");

			auto fileString = std::regex_replace(string, ansiRegex, "");

			for (auto const& filename : TargetManager::get()->getLogTargets()) {
				std::ofstream F(filename, std::ios::app);
				F << fileString;
				F.close();
			}
		}

		return;
	}

	static std::string logString(
		LogLevel logLevel,
		std::string&& formattedBody
	) noexcept;
	static std::string&& limitStr(std::string&& str) noexcept;
};

} // namespace aurora