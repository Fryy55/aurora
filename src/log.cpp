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


std::string log::logString(
	log::LogLevel logLevel,
	std::string&& formattedBody
) noexcept {
	char const* hTag;
	switch (logLevel) {
		case LogLevel::Debug:
			hTag = "\e[30m";
			break;

		case LogLevel::Info:
			hTag = "\e[34m";
			break;

		case LogLevel::Warn:
			hTag = "\e[33m";
			break;

		case LogLevel::Error:
			hTag = "\e[91m";
			break;

		default:
			hTag = "\e[0m";
			break;
	}

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
		[logLevel]() { // log level
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
					return "BLANK";
			}
		}(),
		source ? // optional source specifier
			std::format("\e[0;36m [{}]\e[90m |", limitStr(matches[1].str()))
			:
			"",
		[logLevel]() { // b tag
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