#include "Version.h"

using namespace WEngine;

std::string Version::VersionKind_ToString(VersionKind v) const
{
	switch (v)
	{
	case VersionKind::Unknown: return "Unknown";
	case VersionKind::Dev: return "Development";
	case VersionKind::Tests: return "Testing";
	case VersionKind::Modding: return "Modding";
	case VersionKind::Demo: return "Demo";
	case VersionKind::Promo: return "Promo";
	case VersionKind::Alpha: return "Alpha";
	case VersionKind::Beta: return "Beta";
	case VersionKind::ReleaseCandidate: return "Release Candidate";
	case VersionKind::Release: return "Release";
	case VersionKind::Abandonware: return "Abandonware";
	default: return "Unknown";
	}
}

std::string Version::ToString() const
{
	return std::to_string(major) + "." + std::to_string(minor) + "." + std::to_string(patch) + " (" + VersionKind_ToString(kind) + ")";
}
