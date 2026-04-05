#pragma once

#include <Engine/Types/CommonTypes.h>
#include <string>

#include "Engine/Types/Nullable.h"
#include "Engine/WTL/vector.h"
#include "yaml-cpp/node/convert.h"

namespace WEditor
{
	class AnimationEditorWidget;
}

namespace WEngine
{
	/**
	 *  This struct contains all the information that the Renderer needs to
	 *  render an animation. An animation relies on a sprite atlas to offer
	 *  the animation frames.
	 */
	struct Animation
	{
		/** Name of the Animation. */
		std::string name{};
		/** Starting frame of the Animation. */
		uint16 startFrame{};
		/** Final frame of the Animation. */
		uint16 endFrame{};
		/** Frame count */
		uint16 frameCount{};
	};

	class AnimationParser;
	class AnimationInformation
	{
		friend AnimationParser;
		friend WEditor::AnimationEditorWidget;
	public:
		struct AnimationSubtype
		{
			std::string name;
			wtl::vector<Animation> animations;
		};

		struct AnimationArchetype
		{
			std::string name;
			wtl::vector<AnimationSubtype> subtypes;
		};

		[[nodiscard]] Nullable<Animation> GetAnimation(const std::string& archetype, const std::string& subtype, const std::string& name) const;
		[[nodiscard]] uint16 GetHFramesCount() const { return m_hFramesCount; }
		[[nodiscard]] uint16 GetVFramesCount() const { return m_vFramesCount; }
		[[nodiscard]] uint8 GetFramerate() const { return m_framerate; }
		[[nodiscard]] const std::string& GetName() const { return m_animationName; }

	private:
		wtl::vector<AnimationArchetype> m_archetypes;
		uint16 m_hFramesCount;
		uint16 m_vFramesCount;
		uint8 m_framerate;
		std::string m_animationName;
	};

	class AnimationParser
	{
	public:
		static AnimationInformation ParseAnimation(const std::string& animName);

	private:
		static void ParseCuts(const YAML::Node& root, AnimationInformation& info);
		static void ParseAnimBase(const YAML::Node& root, wtl::vector<Animation>& animCache, uint32 expected);
		static void ParseAnimInfo(const YAML::Node& root, AnimationInformation& info, uint32& expected);
		static void LinkAnimations(AnimationInformation& info, const wtl::vector<Animation>& animCache);
		static void SearchAndPlaceAnimation(AnimationInformation& info, const Animation& animCached);
	};
}
