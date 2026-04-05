#pragma once

#include <Engine/Types/CommonTypes.h>
#include <string>

namespace WEngine
{
	/**
	 * Represents different kinds of versions for the game engine.
	 */
	enum class VersionKind
	{
		Unknown,

		Dev,				///< Used for Dev builds
		Tests,				///< Just for Test mode and QA testing
		Modding,			///< Released for Modders
		Demo,				///< Cuts out much of the content
		Promo,				///< Similar to Demo, just different for Promo
		Alpha,				///< Used for Alpha releases
		Beta,				///< Used for Beta releases
		ReleaseCandidate,	///< Used for Release Candidates
		Release,			///< Used for Release builds
		Abandonware,		///< Used for last versions
	};

	/**
	 * Represents different kinds of versions for the game engine.
	 */
	struct Version
	{
	public:
		constexpr Version() : major(0), minor(0), patch(0), kind(VersionKind::Unknown) {}
		/**
    	 * Constructs a Version object with the given major, minor, patch numbers and kind set to Unknown.
    	 * @param major Major version number.
    	 * @param minor Minor version number.
    	 * @param patch Patch version number.
    	 */
		constexpr Version(uint16 major, uint16 minor, uint16 patch)
			: major(major), minor(minor), patch(patch), kind(VersionKind::Unknown) {
		}
		/**
		 * Constructs a Version object with the given major, minor, patch numbers and kind.
		 * @param major Major version number.
		 * @param minor Minor version number.
		 * @param patch Patch version number.
		 * @param kind Kind of the version.
		 */
		constexpr Version(uint16 major, uint16 minor, uint16 patch, VersionKind kind)
			: major(major), minor(minor), patch(patch), kind(kind) {
		}

	private:
		uint16 major;
		uint16 minor;
		uint16 patch;
		VersionKind kind;

	public:

		bool operator==(const Version& other) const
		{
			return major == other.major && minor == other.minor && patch == other.patch;
		}

		bool operator!=(const Version& other) const
		{
			return !(*this == other);
		}

		bool operator<(const Version& other) const
		{
			if (*this == other)
				return false;
			if (major != other.major)
				return major < other.major;
			if (minor != other.minor)
				return minor < other.minor;
			return patch < other.patch;
		}
		bool operator>(const Version& other) const
		{
			return !(*this < other) && (*this != other);
		}

		bool operator<=(const Version& other) const
		{
			return (*this < other) || (*this == other);
		}
		bool operator>=(const Version& other) const
		{
			return (*this > other) || (*this == other);
		}

		/**
		 * Returns the major version number.
		 * @return Major version number.
		 */
		int16 GetMajor() const { return major; }
		/**
		 * Returns the minor version number.
		 * @return Minor version number.
		 */
		int16 GetMinor() const { return minor; }
		/**
		 * Returns the patch version number.
		 * @return Patch version number.
		 */
		int16 GetPatch() const { return patch; }
		/**
		 * Returns the kind of the version.
		 * @return Kind of the version.
		 */
		VersionKind GetKind() const { return kind; }

		/**
		 * Returns the major version number in a constant expression context.
		 * @return Major version number.
		 */
		constexpr int16 GetMajorCex() const { return major; }
		/**
		 * Returns the minor version number in a constant expression context.
		 * @return Minor version number.
		 */
		constexpr int16 GetMinorCex() const { return minor; }
		/**
		 * Returns the patch version number in a constant expression context.
		 * @return Patch version number.
		 */
		constexpr int16 GetPatchCex() const { return patch; }
		/**
		 * Returns the kind of the version in a constant expression context.
		 * @return Kind of the version.
		 */
		constexpr VersionKind GetKindCex() const { return kind; }
		/**
		 * Converts a VersionKind enum value to its string representation.
		 * @param v The VersionKind enum value to convert.
		 * @return The string representation of the given VersionKind enum value.
		 */
		std::string VersionKind_ToString(VersionKind v) const;

		/**
		 * Converts a Version object to its string representation in the format "major.minor.patch (kind)".
		 * @return The string representation of this version object.
		 */
		std::string ToString() const;
	};
}
