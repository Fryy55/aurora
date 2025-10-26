#include <aurora/ThreadManager.hpp>

#include <aurora/log.hpp>

using namespace aurora;


ThreadManager* ThreadManager::get() noexcept {
	static ThreadManager instance;

	return &instance;
}


void ThreadManager::addThread(std::string_view threadName) noexcept {
	auto [iter, inserted] = m_strDB.emplace(threadName);
	if (!inserted) {
		log::warn(
			"Failed to name thread {}; thread named '{}' already exists.",
			std::this_thread::get_id(), threadName
		);
		return;
	}

	auto id = std::this_thread::get_id();

	m_dbID_S.emplace(id, *iter);
	m_dbS_ID.emplace(*iter, id);

	log::debug("Thread {} saved as '{}'.", id, threadName);

	return;
}

void ThreadManager::removeThread(std::thread::id id) noexcept {
	if (!m_dbID_S.contains(id)) {
		log::warn(
			"Failed to remove thread {}; thread doesn't exist.",
			id
		);
		return;
	}

	auto str = std::string(m_dbID_S[id]);

	m_dbS_ID.erase(str);
	m_dbID_S.erase(id);

	m_strDB.erase(str);

	log::debug(
		"Thread '{}' ({}) removed.",
		str, id
	);

	return;
}

void ThreadManager::removeThread(std::string_view str) noexcept {
	if (!m_dbS_ID.contains(str)) {
		log::warn(
			"Failed to remove thread '{}'; thread doesn't exist.",
			str
		);
		return;
	}

	auto id = m_dbS_ID[str];

	m_dbID_S.erase(id);
	m_dbS_ID.erase(str);

	m_strDB.erase(std::string(str));

	log::debug(
		"Thread '{}' ({}) removed.",
		str, id
	);

	return;
}

void ThreadManager::resetDB() noexcept {
	m_dbID_S.clear();
	m_dbS_ID.clear();

	m_strDB.clear();

	log::debug("Thread name databases reset.");

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