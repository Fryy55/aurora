#include <aurora/singletons/TargetManager.hpp>

#include <aurora/log.hpp>

#include <filesystem>
#include <fstream>
#include <cstring>
#include <algorithm>
#include <chrono>

using namespace aurora;


TargetManager* TargetManager::get() noexcept {
	static TargetManager instance;

	return &instance;
}


bool TargetManager::canOpenFile(std::string_view pathToAFile) const noexcept {
	namespace fs = std::filesystem;

	fs::path path(pathToAFile);

	if (
		auto dir = path.parent_path();
		!fs::exists(dir) && !fs::create_directory(dir)
	) {
		log::warn(
			"[AURORA] Failed to access directory '{}': {}.",
			dir.string(), std::strerror(errno)
		);
		return false;
	}

	std::ofstream F(path);
	if (!F.is_open()) {
		log::warn(
			"[AURORA] Failed to add log target '{}': {}.",
			pathToAFile, std::strerror(errno)
		);
		return false;
	}
	F.close();
	if (!fs::remove(path)) {
		log::error(
			"[AURORA] Failed to remove file '{}': {}. "
			"Unwanted traces may stay on your system.",
			pathToAFile, std::strerror(errno)
		);
	}

	return true;
}


bool TargetManager::addLogTarget(std::string_view pathToAFile) noexcept {
	if (!canOpenFile(pathToAFile))
		return false;

	auto [iter, inserted] = m_logTargets.emplace(pathToAFile);
	if (!inserted) {
		log::warn(
			"[AURORA] Failed to add log target '{}'; target already exists.",
			pathToAFile
		);
		return false;
	}

	log::info("[AURORA] Log target '{}' added.", pathToAFile);

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

	log::info(
		"[AURORA] Log target '{}' removed.",
		pathToAFile
	);

	return true;
}

void TargetManager::clearLogTargets() noexcept {
	m_logTargets.clear();

	log::info("[AURORA] Log targets reset.");

	return;
}


void TargetManager::setMaxFilesInADir(std::uint16_t fileCount) noexcept {
	if (fileCount == 0u) {
		log::warn("[AURORA] Can't set maximum file count to 0.");
		return;
	}

	m_maxFilesInADir = fileCount;

	return;
}

std::optional<std::string> TargetManager::logToDir(
	std::string_view directory,
	std::string_view filename
) noexcept {
	namespace fs = std::filesystem;

	fs::path dir(directory);

	if (!canOpenFile((dir/".aurora-dir-test-file").string()))
		return std::nullopt;

	std::vector<fs::directory_entry> dirVec{};
	for (auto const& file : fs::directory_iterator(dir))
		dirVec.emplace_back(file);

	if (dirVec.size() >= m_maxFilesInADir) {
		std::ranges::sort(
			dirVec,
			[](fs::directory_entry const& lhs, fs::directory_entry const& rhs) {
				return fs::last_write_time(lhs) < fs::last_write_time(rhs);
			}
		);

		std::uint64_t removeFilesCount = dirVec.size() - m_maxFilesInADir + 1;
		log::info(
			"[AURORA] Removing {} file(s) from '{}' (max. files: {}).",
			removeFilesCount, dir.string(), m_maxFilesInADir
		);

		for (std::uint64_t i = 0u; i < removeFilesCount; ++i) {
			if (!fs::remove(dirVec[i])) {
				log::error(
					"[AURORA] Failed to remove file '{}': {}. "
					"Unwanted traces may stay on your system.",
					dirVec[i].path().filename().string(), std::strerror(errno)
				);
			}
		}
	}

	auto target = (dir/std::format(
		"{} {:%F %H.%M.%OS}.log",
		filename,
		std::chrono::zoned_time(
			std::chrono::current_zone(),
			std::chrono::system_clock::now()
		)
	)).string();

	addLogTarget(target);

	return std::move(target);
}