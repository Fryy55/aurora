#include <aurora/log.hpp>

#include <aurora/singletons/ThreadManager.hpp>

#include <chrono>

using namespace aurora;


// Log level
auto log::s_logLevel = log::LogLevel::Debug;
auto log::s_fileLogLevel = log::LogLevel::Info;
// Time locale
bool log::s_use12hTime = false;
// Max source length
std::uint8_t log::s_maxSourceLength = 12u;
// Log to stderr
bool log::s_logToStderr = true;


log::LogStates log::statesForLevel(LogLevel logLevel) noexcept {
	LogStates ret{};

	switch (logLevel) {
		case LogLevel::Debug:
			if (s_logLevel == LogLevel::Debug)
				ret.first = true;
			if (s_fileLogLevel == LogLevel::Debug)
				ret.second = true;
			break;

		case LogLevel::Info:
			if (s_logLevel == LogLevel::Debug || s_logLevel == LogLevel::Info)
				ret.first = true;
			if (s_fileLogLevel == LogLevel::Debug || s_fileLogLevel == LogLevel::Info)
				ret.second = true;
			break;

		case LogLevel::Warn:
			if (s_logLevel != LogLevel::Error)
				ret.first = true;
			if (s_fileLogLevel != LogLevel::Error)
				ret.second = true;
			break;

		case LogLevel::Error: [[fallthrough]];
		default:
			ret = { true, true };
	}

	return ret;
}

std::string log::logString(
	ConfigOpt const& customConfig,
	log::LogLevel logLevel,
	std::string&& formattedBody
) noexcept {
	constexpr auto hasLogLevel = [](CustomLogLevelConfig::ANSITag const& ansiTag) noexcept {
		return std::holds_alternative<LogLevel>(ansiTag);
	};
	constexpr auto getLogLevel = [](CustomLogLevelConfig::ANSITag const& ansiTag) noexcept {
		return std::get<LogLevel>(ansiTag);
	};
	constexpr auto getANSIString = [](CustomLogLevelConfig::ANSITag const& ansiTag) noexcept {
		return std::get<std::string_view>(ansiTag);
	};

	auto hTag = [&logLevel, &customConfig, &hasLogLevel, &getLogLevel, &getANSIString]() -> std::string_view {
		if (!customConfig || hasLogLevel(customConfig->get().headTag)) {
			if (customConfig && hasLogLevel(customConfig->get().headTag))
				logLevel = getLogLevel(customConfig->get().headTag);

			switch (logLevel) {
				case LogLevel::Debug:
					return "\e[30m";

				case LogLevel::Info:
					return "\e[34m";

				case LogLevel::Warn:
					return "\e[33m";

				case LogLevel::Error:
					return "\e[91m";

				default:
					return "\e[0m";
			}
		}

		// control gets here if a config is present and it's set to a custom tag
		return getANSIString(customConfig->get().headTag);
	}();

	// check for a source
	bool source = false;
	static std::regex const sourceRegex(R"(^\[(.*?)\] (.*))");
	std::smatch matches;

	if (std::regex_search(formattedBody, matches, sourceRegex))
		source = true;

	return std::format(
		"{}" // h tag
		"{}" // time
		"\e[90m |" // separator
		" \e[1;30m[{}]\e[0m" // thread
		" {}" // h tag
		"{}" // log level
		"\e[90m |" // separator
		"{}" // optional source specifier
		"{}" // b tag
		" {}" // body
		"\e[0m" // reset tag
		"\n", // newline

		hTag, // h tag
		[]() { // time
			namespace ch = std::chrono;

			ch::zoned_time time(ch::current_zone(), ch::system_clock::now());

			return s_use12hTime ?
				std::format("{:%r}", time)
				:
				std::format("{:%H:%M:%OS}", time);
		}(),
		[]() { // thread
			auto thID = std::this_thread::get_id();

			return std::string(ThreadManager::get()->getThreadNameByID(thID).value_or(
				limitStr(std::format("Thread {}", thID)))
			);
		}(),
		hTag, // h tag
		[logLevel, &customConfig]() -> std::string_view { // log level
			if (customConfig)
				return customConfig->get().logLevelName;

			switch (logLevel) {
				case LogLevel::Debug:
					return "DEBUG";

				case LogLevel::Info:
					return "INFO ";

				case LogLevel::Warn:
					return "WARN ";

				case LogLevel::Error:
					return "ERROR";

				default:
					return "_____";
			}
		}(),
		source ? // optional source specifier
			std::format("\e[0;36m [{}]\e[90m |", limitStr(matches[1].str()))
			:
			"",
		[&logLevel, &customConfig, &hasLogLevel, &getLogLevel, &getANSIString]() -> std::string_view { // b tag
			if (!customConfig || hasLogLevel(customConfig->get().bodyTag)) {
				if (customConfig && hasLogLevel(customConfig->get().bodyTag))
					logLevel = getLogLevel(customConfig->get().bodyTag);

				switch (logLevel) {
					case LogLevel::Warn:
						return "\e[93m";

					case LogLevel::Error:
						return "\e[31m";

					case LogLevel::Debug: [[fallthrough]];
					case LogLevel::Info: [[fallthrough]];
					default:
						return "\e[0m";
				}
			}

			// control gets here if a config is present and it's set to a custom tag
			return getANSIString(customConfig->get().bodyTag);
		}(),
		source ? // body
			matches[2].str()
			:
			formattedBody
	);
}

std::string&& log::limitStr(std::string&& str) noexcept {
	if (str.size() > s_maxSourceLength)
		str = str.substr(0, s_maxSourceLength).append(">");

	return str;
}