#pragma once

#include <aurora/singletons/TargetManager.hpp>

#include <iostream>
#include <print>
#include <regex>
#include <fstream>


namespace aurora {

/**
 * @brief Main logging class.
 * Includes logging functions and configs
 * 
 */
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
	/**
	 * @brief An enum, containing all available log levels
	 * 
	 */
	enum class LogLevel : std::uint8_t {
		Debug,
		Info,
		Warn,
		Error
	};

	/**
	 * @brief Gets logging level for console output. `LogLevel::Debug` by default
	 * 
	 * @return Logging level
	 */
	[[nodiscard]] static LogLevel getLogLevel() noexcept { return s_logLevel; }
	/**
	 * @brief Sets logging level for console output. `LogLevel::Debug` by default
	 * 
	 * @param logLevel Logging level
	 */
	static void setLogLevel(LogLevel logLevel) noexcept { s_logLevel = logLevel; }
	/**
	 * @brief Gets logging level for file output. `LogLevel::Info` by default
	 * 
	 * @return Logging level
	 */
	[[nodiscard]] static LogLevel getFileLogLevel() noexcept { return s_fileLogLevel; }
	/**
	 * @brief Sets logging level for file output. `LogLevel::Info` by default
	 * 
	 * @param logLevel Logging level
	 */
	static void setFileLogLevel(LogLevel logLevel) noexcept { s_fileLogLevel = logLevel; }

private:
	static LogLevel s_logLevel;
	static LogLevel s_fileLogLevel;

public:
	// Time locale
	/**
	 * @brief Gets 12h time formatting setting. `false` by default
	 * 
	 * @return Current value
	 */
	[[nodiscard]] static bool get12hTimeEnabled() noexcept { return s_use12hTime; }
	/**
	 * @brief Sets 12h time formatting setting. `false` by default
	 * 
	 * @param on Value to set
	 */
	static void set12hTimeEnabled(bool on) noexcept { s_use12hTime = on; }

private:
	static bool s_use12hTime;

public:
	// Max source length
	/**
	 * @brief Gets maximum source specifier/thread name length. `12` by default
	 * 
	 * @return Current value
	 */
	[[nodiscard]] static std::uint8_t getMaxSourceLength() noexcept { return s_maxSourceLength; }
	/**
	 * @brief Sets maximum source specifier/thread name length. `12` by default
	 * 
	 * @param maxSourceLength Value to set
	 */
	static void setMaxSourceLength(std::uint8_t maxSourceLength) noexcept {
		s_maxSourceLength = maxSourceLength;
	}

private:
	static std::uint8_t s_maxSourceLength;

public:
	// Log to stderr
	/**
	 * @brief Gets the current console log target stream. `true` by default
	 * 
	 * @return true Aurora will log to `stderr`
	 * @return false Aurora will log to `stdout`
	 */
	[[nodiscard]] static bool getLogToStderrEnabled() noexcept { return s_logToStderr; }
	/**
	 * @brief Sets the current console log target stream. `true` by default
	 * 
	 * @param on Value to set. `true` will log to `stderr`; `false` will log to `stdout`
	 */
	static void setLogToStderrEnabled(bool on) noexcept { s_logToStderr = on; }

private:
	static bool s_logToStderr;


// Logging functions
private:
	using LogStates = std::pair<bool, bool>;

public:
	/**
	 * @brief Logs at the debug level
	 * 
	 * @param formatString String to format against (e.g. `"Hello, my name is {}"`)
	 * @param args Args to format with. Should match the number of fields in @p formatString
	 */
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
	/**
	 * @brief Logs at the info level
	 * 
	 * @param formatString String to format against (e.g. `"Hello, my name is {}"`)
	 * @param args Args to format with. Should match the number of fields in @p formatString
	 */
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
	/**
	 * @brief Logs at the warn level
	 * 
	 * @param formatString String to format against (e.g. `"Hello, my name is {}"`)
	 * @param args Args to format with. Should match the number of fields in @p formatString
	 */
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
	/**
	 * @brief Logs at the error level
	 * 
	 * @param formatString String to format against (e.g. `"Hello, my name is {}"`)
	 * @param args Args to format with. Should match the number of fields in @p formatString
	 */
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