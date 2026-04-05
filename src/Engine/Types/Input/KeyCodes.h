#pragma once

#include <SDL3/SDL.h>
#include "../CommonTypes.h"

// Only realistically usable keys
enum class WKey : int
{
	// Letters
	A = SDL_SCANCODE_A,
	B = SDL_SCANCODE_B,
	C = SDL_SCANCODE_C,
	D = SDL_SCANCODE_D,
	E = SDL_SCANCODE_E,
	F = SDL_SCANCODE_F,
	G = SDL_SCANCODE_G,
	H = SDL_SCANCODE_H,
	I = SDL_SCANCODE_I,
	J = SDL_SCANCODE_J,
	K = SDL_SCANCODE_K,
	L = SDL_SCANCODE_L,
	M = SDL_SCANCODE_M,
	N = SDL_SCANCODE_N,
	O = SDL_SCANCODE_O,
	P = SDL_SCANCODE_P,
	Q = SDL_SCANCODE_Q,
	R = SDL_SCANCODE_R,
	S = SDL_SCANCODE_S,
	T = SDL_SCANCODE_T,
	U = SDL_SCANCODE_U,
	V = SDL_SCANCODE_V,
	W = SDL_SCANCODE_W,
	X = SDL_SCANCODE_X,
	Y = SDL_SCANCODE_Y,
	Z = SDL_SCANCODE_Z,

	// Numbers
	N1 = SDL_SCANCODE_1,
	N2 = SDL_SCANCODE_2,
	N3 = SDL_SCANCODE_3,
	N4 = SDL_SCANCODE_4,
	N5 = SDL_SCANCODE_5,
	N6 = SDL_SCANCODE_6,
	N7 = SDL_SCANCODE_7,
	N8 = SDL_SCANCODE_8,
	N9 = SDL_SCANCODE_9,
	N0 = SDL_SCANCODE_0,

	// Special keys
	ENTER = SDL_SCANCODE_RETURN,
	ESCAPE = SDL_SCANCODE_ESCAPE,
	SPACE = SDL_SCANCODE_SPACE,
	TAB = SDL_SCANCODE_TAB,
	BACKSPACE = SDL_SCANCODE_BACKSPACE,
	CAPSLOCK = SDL_SCANCODE_CAPSLOCK,
	ALT = SDL_SCANCODE_RALT,
	SHIFT = SDL_SCANCODE_LSHIFT,

	// Arrows
	LEFT = SDL_SCANCODE_LEFT,
	RIGHT = SDL_SCANCODE_RIGHT,
	UP = SDL_SCANCODE_UP,
	DOWN = SDL_SCANCODE_DOWN,

	// Debug Keys (function keys)
	// If anyone ever thinks they should use them in gameplay
	// then they should reconsider their choices. These are reserved
	// for debugging and tools, and should be used as such.
	DEBUG1 = SDL_SCANCODE_F1,
	DEBUG2 = SDL_SCANCODE_F2,
	DEBUG3 = SDL_SCANCODE_F3,
	DEBUG4 = SDL_SCANCODE_F4,
	DEBUG5 = SDL_SCANCODE_F5,
	DEBUG6 = SDL_SCANCODE_F6,
	DEBUG7 = SDL_SCANCODE_F7,
	DEBUG8 = SDL_SCANCODE_F8,
	DEBUG9 = SDL_SCANCODE_F9,
	DEBUG10 = SDL_SCANCODE_F10,
	DEBUG11 = SDL_SCANCODE_F11,
	DEBUG12 = SDL_SCANCODE_F12,
};

enum class WMouseBtn : int
{
	MOUSE_LEFT,
	MOUSE_MIDDLE,
	MOUSE_RIGHT,
	MOUSE_4,
	MOUSE_5,
};

enum class WPadBtn : uint8
{
	// Xbox , Valve , PlayStation , Nintendo (for Copy&Paste)
	South,		///< Xbox A, Valve A, PlayStation Cross, Nintendo B
	East,		///< Xbox B, Valve B, PlayStation Circle, Nintendo A
	West,		///< Xbox X, Valve X, PlayStation Square, Nintendo Y
	North,		///< Xbox Y, Valve Y, PlayStation Triangle, Nintendo X
	Select,		///< Xbox View, Valve View, PlayStation Share (None), Nintendo -
	Start,		///< Xbox Menu, Valve Menu, PlayStation Options, Nintendo +
	LS,			///< Xbox LS, Valve L3, PlayStation L3, Nintendo (idk)
	RS,			///< Xbox RS, Valve R3, PlayStation R3, Nintendo (idk)
	LB,			///< Xbox LB, Valve L1, PlayStation L1, Nintendo L Button
	RB,			///< Xbox RB, Valve R1, PlayStation R1, Nintendo R Button
	LT,			///< Xbox LT, Valve L2, PlayStation L2, Nintendo ZL
	RT,			///< Xbox RT, Valve R2, PlayStation R2, Nintendo ZR
	Up,			///< Xbox D-Pad Up, Valve D-Pad Up, PlayStation D-Pad Up, Nintendo D-Pad Up
	Down,		///< Xbox D-Pad Down, Valve D-Pad Down, PlayStation D-Pad Down, Nintendo D-Pad Down
	Left,		///< Xbox D-Pad Left, Valve D-Pad Left, PlayStation D-Pad Left, Nintendo D-Pad Left
	Right,		///< Xbox D-Pad Right,Valve D-Pad Right, PlayStation D-Pad Right, Nintendo D-Pad Right
	Touchpad,	///< Valve Trackpad, PlayStation Touchpad
	L4,			///< Xbox P1, Valve L4, Playstation R[B]
	R4,			///< Xbox P3, Valve R4, Playstation L[B]
	L5,			///< Xbox P2, Valve L5
	R5,			///< Xbox P4, Valve R5

	Pad_Button_Count
};

// Since apparently not every manufacturer can agree on what button
// gives what signal, we will have to basically make google translate

// The following masks are meant to relate the above enum to the
// buttons sent by the controllers.

extern uint8 WPadBtn_Generic[];
extern uint8 WPadBtn_Playstation[];
extern uint8 WPadBtn_Xbox[];
