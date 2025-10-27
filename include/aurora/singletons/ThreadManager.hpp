#pragma once

#include <thread>
#include <unordered_map>
#include <unordered_set>


namespace aurora {

/**
 * @brief Manages thread names for cleaner logging.
 * A singleton
 * 
 */
class ThreadManager final {
public:
	/**
	 * @brief Instance getting method
	 * 
	 * @return Instance of a singleton
	 */
	static ThreadManager* get() noexcept;

	ThreadManager(ThreadManager const&) = delete;
	ThreadManager& operator=(ThreadManager const&) = delete;
	ThreadManager(ThreadManager&&) = delete;
	ThreadManager& operator=(ThreadManager&&) = delete;

private:
	ThreadManager() noexcept = default;
	~ThreadManager() = default;

public:
	/**
	 * @brief Adds the @em calling @em thread to the database with a provided name
	 * 
	 * @param threadName Name for the thread
	 * @return Boolean, indicating successful addition
	 */
	bool addThread(std::string_view threadName) noexcept;
	/**
	 * @brief Removes a thread from the database by its ID
	 * 
	 * @param id ID of the thread to remove
	 * @return Boolean, indicating successful removal
	 */
	bool removeThread(std::thread::id id) noexcept;
	/**
	 * @brief Removes a thread from the database by its name
	 * 
	 * @param threadName Name of the thread to remove
	 * @return Boolean, indicating successful removal
	 */
	bool removeThread(std::string_view threadName) noexcept;
	/**
	 * @brief Resets the database of thread names
	 * 
	 */
	void clearDB() noexcept;
	/**
	 * @brief Gets the thread's name by its ID
	 * 
	 * @param id ID of the thread
	 * @return Name of the thread or `std::nullopt` if the thread doesn't exist
	 */
	[[nodiscard]] std::optional<std::string_view> getThreadNameByID(std::thread::id id) const noexcept;
	/**
	 * @brief Gets the thread's ID by its name
	 * 
	 * @param threadName Name of the thread
	 * @return ID of the thread or `std::nullopt` if the thread doesn't exist
	 */
	[[nodiscard]] std::optional<std::thread::id> getThreadIDByName(std::string_view threadName) const noexcept;

private:
	// Fields
	std::unordered_map<std::thread::id, std::string_view> m_dbID_S{};
	std::unordered_map<std::string_view, std::thread::id> m_dbS_ID{};
	std::unordered_set<std::string> m_strDB{};
};

} // namespace aurora