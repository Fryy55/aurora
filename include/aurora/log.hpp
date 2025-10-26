#pragma once

#include <cstdint>
#include <print>


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

private:
	static LogLevel s_logLevel;

public:
	// Time locale
	[[nodiscard]] static bool get12hTimeEnabled() noexcept { return s_use12hTime; }
	static void set12hTimeEnabled(bool on) noexcept { s_use12hTime = on; }

private:
	static bool s_use12hTime;

public:
	// Logging functions
	template <typename ...Args>
	static void debug(std::format_string<Args...> formatString, Args&&... args) noexcept {
		if (s_logLevel == LogLevel::Debug)
			printLog(LogLevel::Debug, formatString, std::forward<Args>(args)...);

		return;
	}
	template <typename ...Args>
	static void info(std::format_string<Args...> formatString, Args&&... args) noexcept {
		if (s_logLevel == LogLevel::Debug || s_logLevel == LogLevel::Info)
			printLog(LogLevel::Info, formatString, std::forward<Args>(args)...);

		return;
	}
	template <typename ...Args>
	static void warn(std::format_string<Args...> formatString, Args&&... args) noexcept {
		if (s_logLevel != LogLevel::Error)
			printLog(LogLevel::Warn, formatString, std::forward<Args>(args)...);

		return;
	}
	template <typename ...Args>
	static void error(std::format_string<Args...> formatString, Args&&... args) noexcept {
		printLog(LogLevel::Error, formatString, std::forward<Args>(args)...);

		return;
	}

private:
	template <typename ...Args>
	static void printLog(
		LogLevel logLevel,
		std::format_string<Args...> formatString,
		Args&&... args
	) noexcept {
		auto body = std::format(formatString, std::forward<Args>(args)...);

		std::vprint_unicode(
			logString(logLevel),
			std::make_format_args(body)
		);

		return;
	}
	static std::string logString(LogLevel logLevel) noexcept;
};

} // namespace aurora