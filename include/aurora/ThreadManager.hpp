#pragma once

#include <thread>
#include <unordered_map>
#include <unordered_set>


namespace aurora {

class ThreadManager final {
public:
	static ThreadManager* get() noexcept;

	ThreadManager(ThreadManager const&) = delete;
	ThreadManager& operator=(ThreadManager const&) = delete;
	ThreadManager(ThreadManager&&) = delete;
	ThreadManager& operator=(ThreadManager&&) = delete;

private:
	ThreadManager() noexcept = default;
	~ThreadManager() = default;

public:
	void addThread(std::string_view threadName) noexcept;
	void removeThread(std::thread::id id) noexcept;
	void removeThread(std::string_view threadName) noexcept;
	void resetDB() noexcept;
	[[nodiscard]] std::optional<std::string_view> getThreadNameByID(std::thread::id id) const noexcept;
	[[nodiscard]] std::optional<std::thread::id> getThreadIDByName(std::string_view threadName) const noexcept;

private:
	// Fields
	std::unordered_map<std::thread::id, std::string_view> m_dbID_S{};
	std::unordered_map<std::string_view, std::thread::id> m_dbS_ID{};
	std::unordered_set<std::string> m_strDB{};
};

} // namespace aurora