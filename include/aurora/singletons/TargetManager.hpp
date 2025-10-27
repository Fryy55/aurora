#pragma once

#include <unordered_set>
#include <string>
#include <optional>
#include <cstdint>


namespace aurora {

/**
 * @brief Manages extra logging targets (files).
 * A singleton
 * 
 */
class TargetManager final {
public:
	/**
	 * @brief Instance getting method
	 * 
	 * @return Instance of a singleton
	 */
	static TargetManager* get() noexcept;

	TargetManager(TargetManager const&) = delete;
	TargetManager& operator=(TargetManager const&) = delete;
	TargetManager(TargetManager&&) = delete;
	TargetManager& operator=(TargetManager&&) = delete;

private:
	TargetManager() noexcept = default;
	~TargetManager() = default;

	[[nodiscard]] bool canOpenFile(std::string_view pathToAFile) const noexcept;

public:
	/**
	 * @brief An `std::unordered_set` of all currently active targets (files)
	 * 
	 */
	using Targets = std::unordered_set<std::string>;
	/**
	 * @brief Gets the current log targets (files) <em>by `const&`</em>
	 * 
	 * @return Currently active targets
	 */
	[[nodiscard]] Targets const& getLogTargets() const noexcept { return m_logTargets; }
	/**
	 * @brief Adds a new target (file) for logging
	 * 
	 * @param pathToAFile Absolute/relative to the executable path to a file
	 * @return Boolean, indicating successful creation
	 */
	bool addLogTarget(std::string_view pathToAFile) noexcept;
	/**
	 * @brief Removes a target (file) from current targets
	 * 
	 * @param pathToAFile Absolute/relative to the executable path to a file
	 * @return Boolean, indicating successful removal
	 */
	bool removeLogTarget(std::string_view pathToAFile) noexcept;
	/**
	 * @brief Removes all log targets (files)
	 * 
	 */
	void clearLogTargets() noexcept;

	/**
	 * @brief Gets the number of maximum files allowed in one auto-managed directory. `5` by default
	 * 
	 * @details Only matters on @ref TargetManger::logToDir calls
	 * 
	 * @return Current value
	 */
	[[nodiscard]] std::uint16_t getMaxFilesInADir() const noexcept { return m_maxFilesInADir; }
	/**
	 * @brief Sets the number of maximum files allowed in one auto-managed directory. `5` by default
	 * 
	 * @details Only matters on @ref TargetManger::logToDir calls
	 * 
	 * @param fileCount Value to set
	 */
	void setMaxFilesInADir(std::uint16_t fileCount) noexcept;
	/**
	 * @brief Adds the specified directory as an auto-managed directory for the current session
	 * 
	 * @details Depends on the maximum files in a directory setting within this class
	 *
	 * @warning Be @em EXTRA careful with this function. Setting @p directory to any non-blank directory <em>almost certainly will make you lose files</em>, as this function cleans them up to the limit, given it has permissions.
	 * As per the license, the developer(s) of Aurora cannot be held liable for all the damage caused by misuses of this function; no warranty is provided whatsoever
	 * 
	 * @param directory Directory path to use for log storage
	 * @param filename Identifier of log files (e.g. with @p filename being `Aurora` the file name will have the format `Aurora 2025-12-31 23.59.59.log`)
	 * @return Path to the current log or `std::nullopt` if the creation failed
	 */
	std::optional<std::string> logToDir(std::string_view directory, std::string_view filename) noexcept;

private:
	// Fields
	Targets m_logTargets{};
	std::uint16_t m_maxFilesInADir = 5;
};

} // namespace aurora