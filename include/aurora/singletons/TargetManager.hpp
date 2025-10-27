#pragma once

#include <unordered_set>
#include <string>


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

public:
	using Targets = std::unordered_set<std::string>;
	[[nodiscard]] Targets const& getLogTargets() noexcept { return m_logTargets; }
	bool addLogTarget(std::string_view pathToAFile) noexcept;
	bool removeLogTarget(std::string_view pathToAFile) noexcept;
	void clearLogTargets() noexcept;

private:
	// Fields
	Targets m_logTargets{};
};

} // namespace aurora