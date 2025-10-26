#include <aurora/log.hpp>

#include <aurora/ThreadManager.hpp>

#include <chrono>

using namespace aurora;


// Log level
auto log::s_logLevel = log::LogLevel::Info;
// Time locale
auto log::s_use12hTime = false;


std::string log::logString(log::LogLevel logLevel) noexcept {
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

	return std::format(
		"{}" // h tag
		"{}" // time
		"\e[90m |" // separator
		" \e[1;30m[{}]\e[0m" // thread
		" {}" // h tag
		"{}" // log level
		"\e[90m |" // separator
		"{}" // b tag
		" {{}}" // body
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
				std::format("Thread {}", thID))
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
		}()
	);
}