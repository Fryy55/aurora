#include <aurora/singletons/ThreadManager.hpp>

#include <aurora/log.hpp>

using namespace aurora;


ThreadManager* ThreadManager::get() noexcept {
	static auto instance = new ThreadManager(); // NOLINT(bugprone-unhandled-exception-at-new)

	return instance;
}


bool ThreadManager::addThread(std::string_view threadName) noexcept {
	auto id = std::this_thread::get_id();
	bool idAlreadySaved = false;

	if (m_dbID_S.contains(id))
		idAlreadySaved = true;

	auto [iter, inserted] = m_strDB.emplace(threadName);
	if (!inserted) {
		log::warn(
			"[AURORA] Failed to name thread {}; thread named '{}' already exists.",
			id, threadName
		);
		return false;
	}

	if (idAlreadySaved)
		this->removeThread(id);

	m_dbID_S.emplace(id, *iter);
	m_dbS_ID.emplace(*iter, id);

	if (idAlreadySaved)
		log::debug("[AURORA] Thread {} renamed to '{}'.", id, threadName);
	else
		log::debug("[AURORA] Thread {} saved as '{}'.", id, threadName);

	return true;
}

bool ThreadManager::removeThread(std::thread::id id) noexcept {
	if (!m_dbID_S.contains(id)) {
		log::warn(
			"[AURORA] Failed to remove thread {}; thread doesn't exist.",
			id
		);
		return false;
	}

	auto str = std::string(m_dbID_S[id]);

	m_dbS_ID.erase(str);
	m_dbID_S.erase(id);

	m_strDB.erase(str);

	log::debug(
		"[AURORA] Thread '{}' ({}) removed.",
		str, id
	);

	return true;
}

bool ThreadManager::removeThread(std::string_view str) noexcept {
	if (!m_dbS_ID.contains(str)) {
		log::warn(
			"[AURORA] Failed to remove thread '{}'; thread doesn't exist.",
			str
		);
		return false;
	}

	auto id = m_dbS_ID[str];

	m_dbID_S.erase(id);
	m_dbS_ID.erase(str);

	m_strDB.erase(std::string(str));

	log::debug(
		"[AURORA] Thread '{}' ({}) removed.",
		str, id
	);

	return true;
}

void ThreadManager::clearDB() noexcept {
	m_dbID_S.clear();
	m_dbS_ID.clear();

	m_strDB.clear();

	log::debug("[AURORA] Thread name databases reset.");

	return;
}

std::optional<std::string_view> ThreadManager::getThreadNameByID(
	std::thread::id id
) const noexcept {
	if (!m_dbID_S.contains(id))
		return std::nullopt;

	return m_dbID_S.at(id);
}

std::optional<std::thread::id> ThreadManager::getThreadIDByName(
	std::string_view str
) const noexcept {
	if (!m_dbS_ID.contains(str))
		return std::nullopt;

	return m_dbS_ID.at(str);
}