#include <aurora/singletons/TargetManager.hpp>

#include <aurora/log.hpp>

#include <fstream>

using namespace aurora;


TargetManager* TargetManager::get() noexcept {
	static TargetManager instance;

	return &instance;
}


bool TargetManager::addLogTarget(std::string_view pathToAFile) noexcept {
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

	auto [iter, inserted] = m_logTargets.emplace(pathToAFile);
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

bool TargetManager::removeLogTarget(std::string_view pathToAFile) noexcept {
	std::string pathToAFileStr(pathToAFile);

	if (!m_logTargets.contains(pathToAFileStr)) {
		log::warn(
			"[AURORA] Failed to remove log target '{}'; target doesn't exist.",
			pathToAFile
		);
		return false;
	}

	m_logTargets.erase(pathToAFileStr);

	log::debug(
		"[AURORA] Log target '{}' removed.",
		pathToAFile
	);

	return true;
}

void TargetManager::clearLogTargets() noexcept {
	m_logTargets.clear();

	log::debug("[AURORA] Log targets reset.");

	return;
}