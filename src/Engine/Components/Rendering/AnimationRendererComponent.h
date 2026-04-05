#pragma once

// This file does not contain LLM generated documentation

#include <Engine/Components/Component.h>
#include <../../Types/Rendering/Animation.h>
#include <Engine/Types/SpawnArgs.h>
#include <../../Types/Rendering/Atlas.h>
#include <vector>

namespace WEngine
{
	/**
	 * This component is a basic animation renderer.
	 * It draws an animation from a spritesheet, and is generally meant to be controlled by an external component.
	 */
	class AnimationRendererComponent : public Component
	{
	public:
		/**
		 * @warning Auto invoked by the engine, do not call this manually!
		 */
		AnimationRendererComponent(Entity* e);

	private:
		Vector2 m_position = {};
		Vector2 m_offset = {};
		float32 m_scale = 1.0f;
		Atlas m_atlas;
		Animation m_currentAnim;

		float32 m_nextFrameCounter;
		float32 m_frameTimeBetween;
		bool m_isPlaying;

		uint16 m_currentFrame;

		bool m_flipX, m_flipY;

	public:
		/**
		 * @warning Auto invoked by the engine, do not call this manually!
		 */
		void Awake(ComponentArgs ca) override;
		/**
		 * @warning Auto invoked by the engine, do not call this manually!
		 */
		void Tick(float32 dt) override;
		/**
		 * @warning Auto invoked by the engine, do not call this manually!
		 */
		void Draw() override;

		/**
		 * Sets the flip state of the sprite.
		 * @param flipX If true, applies a flip to the sprite on the X axis, flipping it left and right on the horizontal axis.
		 * @param flipY If true, applies a flip to the sprite on the Y axis, flipping it up and down on the vertical axis.
		 */
		void SetFlip(bool flipX, bool flipY);

		/**
		 * Sets the offset of the sprite.
		 * @param offset The new offset of the sprite.
		 */
		void SetOffset(Vector2 offset);

		/**
		 * Finds the animation to be played and sets it as the current playing animation.
		 * @param archetype The archetype of the animation.
		 * @param subtype The subtype of the animation.
		 * @param name The name of the animation.
		 * @note If an invalid animation is passed, it will not be discarded. It will simply apply an empty animation.
		 */
		void SetAnimDirect(const std::string& archetype, const std::string& subtype, const std::string& name);

		/**
		 * Randomly selects a specific animation.
		 * @param archetype The archetype of the animation.
		 * @param subtype The subtype of the animation.
		 * @param name The name of the animation.
		 * @param variations The amount of animations available to choose from. This must be the count of animations
		 * instead of the highest number after an animation. If your last animation is called "Anim2", then variations is 3, not 2.
		 * @return The variation that was decided on.
		 * @note This only works if your animations are well-ordered starting with 0.
		 * @note Even if variations is 0, a zero will be appended to the name.
		 */
		uint8 SetAnimRandom(const std::string& archetype, const std::string& subtype, const std::string& name, uint8 variations);

		/**
		 * Sets the atlas to be used with the animation.
		 * @param atlasName Name of the atlas.
		 * @note The sprite atlas and animation yaml file must have the same case-sensitive name and be placed in the
		 * same directory within Data/Sprites/.
		 */
		void SetAtlas(const std::string& atlasName);

		/**
		 * Returns whether the animation is playing.
		 * @return True if the animation is playing, false if the animation is not playing.
		 * @note Depending on the order of the components within an entity, it could have a different implication. Assuming
		 * the animation is finished, if this component is ticked before being checked, then it means that the animation
		 * is done for this frame; if this component is checked before ticking, it means that the animation finished
		 * last frame. Both will work identically if a new animation is passed in when it returns false.
		 */
		[[nodiscard]] bool IsAnimationPlaying() const;

	private:
		void CreateAtlas(const std::string& name);
		void RenderAnim();

		void SetupAfterAnimSwitch();

		COMP_HASH(0xb39b0b842dc4734);
	};
}