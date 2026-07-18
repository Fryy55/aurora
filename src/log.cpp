#include <aurora/log.hpp>

#include <aurora/singletons/ThreadManager.hpp>

#include <chrono>

using namespace aurora;


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
		"\e[0m\e[90m |" // separator
		" \e[1;30m[{}\e[0m\e[1;30m]\e[0m" // thread
		" {}" // h tag
		"{}" // log level
		"\e[0m\e[90m |" // separator
		"{}" // optional source specifier
		" \e[0m{}" // b tag
		"{}" // body
		"\e[0m\n", // newline

		hTag, // h tag
		[]() { // time
			namespace ch = std::chrono;

			auto tt = ch::system_clock::to_time_t(ch::system_clock::now());

			std::tm localTime;

			#if defined(_MSC_VER)
				localtime_s(&localTime, &tt);
			#else
				localtime_r(&tt, &localTime);
			#endif

			std::stringstream stream;
			stream << (s_use12hTime ?
						std::put_time(&localTime, "%r")
						:
						std::put_time(&localTime, "%H:%M:%OS"));
			
			return stream.str();
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
			std::format(" \e[36m[{}\e[0m\e[36m]\e[90m |", limitStr(matches[1].str()))
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
						return "";
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