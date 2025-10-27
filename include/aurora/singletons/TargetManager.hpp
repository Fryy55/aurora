#pragma once

#include <unordered_set>
#include <string>
#include <optional>
#include <cstdint>


namespace aurora {

class TargetManager final {
public:
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
	using Targets = std::unordered_set<std::string>;
	[[nodiscard]] Targets const& getLogTargets() const noexcept { return m_logTargets; }
	bool addLogTarget(std::string_view pathToAFile) noexcept;
	bool removeLogTarget(std::string_view pathToAFile) noexcept;
	void clearLogTargets() noexcept;

	[[nodiscard]] std::uint16_t getMaxFilesInADir() const noexcept { return m_maxFilesInADir; }
	void setMaxFilesInADir(std::uint16_t fileCount) noexcept;
	std::optional<std::string> logToDir(std::string_view directory, std::string_view filename) noexcept;

private:
	// Fields
	Targets m_logTargets{};
	std::uint16_t m_maxFilesInADir = 5;
};

} // namespace aurora