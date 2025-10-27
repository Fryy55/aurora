#include <aurora/log.hpp>

#include <aurora/ThreadManager.hpp>

#include <chrono>

using namespace aurora;


// Log level
auto log::s_logLevel = log::LogLevel::Debug;
auto log::s_fileLogLevel = log::LogLevel::Info;
// Time locale
auto log::s_use12hTime = false;
// Extra targets
log::Targets log::s_logTargets{};


bool log::addLogTarget(std::string_view pathToAFile) noexcept {
	std::string pathToAFileStr(pathToAFile);
	std::ofstream F(pathToAFileStr);
	if (!F.is_open()) {
		log::warn(
			"[AURORA] Failed to add log target '{}'; target isn't a file name.",
			pathToAFile
		);
		return false;
	}
	F.close();
	(void)std::remove(pathToAFileStr.c_str());

	auto [iter, inserted] = s_logTargets.emplace(pathToAFile);
	if (!inserted) {
		log::warn(
			"[AURORA] Failed to add log target '{}'; target already exists.",
			pathToAFile
		);
		return false;
	}

	log::debug("[AURORA] Log target '{}' added.", pathToAFile);

	return true;
}

bool log::removeLogTarget(std::string_view pathToAFile) noexcept {
	std::string pathToAFileStr(pathToAFile);

	if (!s_logTargets.contains(pathToAFileStr)) {
		log::warn(
			"[AURORA] Failed to remove log target '{}'; target doesn't exist.",
			pathToAFile
		);
		return false;
	}

	s_logTargets.erase(pathToAFileStr);

	log::debug(
		"[AURORA] Log target '{}' removed.",
		pathToAFile
	);

	return true;
}

void log::clearLogTargets() noexcept {
	s_logTargets.clear();

	log::debug("[AURORA] Log targets reset.");

	return;
}


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
	static std::regex const sourceRegex(R"(^(\[.*\]) (.*))");
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
		source ? // optional source specifier
			std::format("\e[0;36m {}\e[90m |", matches[1].str())
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