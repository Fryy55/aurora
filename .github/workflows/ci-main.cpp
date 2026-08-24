#include <aurora/aurora.hpp>


namespace foo {

template <typename ...Args>
void trace(std::format_string<Args...> const& formatString, Args&&... args) noexcept {
	using namespace aurora;

	log::custom(
		{
			.logLevel=log::LogLevel::Debug,
			.logLevelName="TRACE",
			.headTag="\e[35m",
			.bodyTag=log::LogLevel::Warn
		},
		formatString, std::forward<Args>(args)...
	);

	return;
}

} // namespace foo


int main() {
	using namespace aurora;

	log::set12hTimeEnabled(true);
	log::setFileLogLevel(log::LogLevel::Debug);

	ThreadManager::get()->addThread("Main");

	log::debug("[Cool Project] haii from Cool Project!");
	log::info("[Project with a very long name] haii from Project with a very long name!");
	log::warn("haii {} #{}!", "User", 43);

	ThreadManager::get()->addThread("Renamed Main");

	log::error("haii!");
	foo::trace("haii {} #{}!", "User", 44);

	return 0;
}