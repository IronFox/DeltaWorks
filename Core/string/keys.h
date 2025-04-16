#ifndef keysH
#define keysH

/******************************************************************

keycode-definitions.

******************************************************************/

#include "../global_root.h"


#if SYSTEM==WINDOWS
	#include <Winuser.h>
#elif SYSTEM==UNIX

	#define VK_SHIFT		50 //54
	#define VK_CAPITAL		21
	#define VK_CONTROL		37
	#define VK_BACK			22
	#define VK_RETURN		36
	#define VK_TAB			23
	#define VK_PAUSE		110
	#define VK_ESCAPE		9

	#define VK_LEFT			100
	#define VK_UP			98
	#define VK_RIGHT		102
	#define VK_DOWN			104
	

	#define VK_SNAPSHOT		111
	
	#define VK_PRINT		111

#else

	#define VK_SHIFT		0x10
	#define VK_CONTROL		0x11
	#define VK_BACK			0x8
	#define VK_RETURN		0xD
	#define VK_TAB			0x9
	#define VK_PAUSE		0x13
	#define VK_ESCAPE		0x1B
	#define VK_CAPITAL		0x14

	#define VK_LEFT			37
	#define VK_UP			38
	#define VK_RIGHT		39
	#define VK_DOWN			40

	#define VK_SNAPSHOT		44
	
	#define VK_PRINT		0x2A

#endif


#define SET_JOYSTICK(INDEX)	\
	Joystick##INDEX##Button0,\
	Joystick##INDEX##Button1,\
	Joystick##INDEX##Button2,\
	Joystick##INDEX##Button3,\
	Joystick##INDEX##Button4,\
	Joystick##INDEX##Button5,\
	Joystick##INDEX##Button6,\
	Joystick##INDEX##Button7,\
	Joystick##INDEX##Button8,\
	Joystick##INDEX##Button9,\
	Joystick##INDEX##Button10,\
	Joystick##INDEX##Button11,\
	Joystick##INDEX##Button12,\
	Joystick##INDEX##Button13,\
	Joystick##INDEX##Button14,\
	Joystick##INDEX##Button15,\
	Joystick##INDEX##Button16,\
	Joystick##INDEX##Button17,\
	Joystick##INDEX##Button18,\
	Joystick##INDEX##Button19,\
	Joystick##INDEX##Button20,\
	Joystick##INDEX##Button21,\
	Joystick##INDEX##Button22,\
	Joystick##INDEX##Button23,\
	Joystick##INDEX##Button24,\
	Joystick##INDEX##Button25,\
	Joystick##INDEX##Button26,\
	Joystick##INDEX##Button27,\
	Joystick##INDEX##Button28,\
	Joystick##INDEX##Button29,\
	Joystick##INDEX##Button30,\
	Joystick##INDEX##Button31,


namespace Key	//! Key code namespace
{
	enum Name	//! Key code name
	{
		Undefined	=	0,				//!< Undefined key
		Shift		=	VK_SHIFT,		//!< Left or right shift key
		Ctrl		=	VK_CONTROL,		//!< Left or right control key
		Control		=	VK_CONTROL,
		Back		=	VK_BACK,		//!< Backspace key
		BackSpace	=	VK_BACK,		//!< Backspace key
		Backspace	=	VK_BACK,		//!< Backspace key
		Return		=	VK_RETURN,		//!< Return key
		Tab			=	VK_TAB,			//!< Tabulator key
		Tabulator	=	VK_TAB,			//!< Tabulator key
		Pause		=	VK_PAUSE,		//!< PAUSE key
		Esc			=	VK_ESCAPE,		//!< Escape key
		Escape		=	VK_ESCAPE,		//!< Escape key
		Left		=	VK_LEFT,		//!< Left arrow key
		Up			=	VK_UP,			//!< Up arrow key
		Right		=	VK_RIGHT,		//!< Right arrow key
		Down		=	VK_DOWN,		//!< Down arrow key
		LeftArrow	=	VK_LEFT,		//!< Left arrow key
		UpArrow		=	VK_UP,			//!< Up arrow key
		RightArrow	=	VK_RIGHT,		//!< Right arrow key
		DownArrow	=	VK_DOWN,		//!< Down arrow key
		ArrowLeft	=	VK_LEFT,		//!< Left arrow key
		ArrowUp		=	VK_UP,			//!< Up arrow key
		ArrowRight	=	VK_RIGHT,		//!< Right arrow key
		ArrowDown	=	VK_DOWN,		//!< Down arrow key
		SnapShot	=	VK_SNAPSHOT,	//!< Snapshot (print) key
		Snapshot	=	VK_SNAPSHOT,	//!< Snapshot (print) key
		Print		=	VK_PRINT,
		
		Capital		=	VK_CAPITAL,		//!< Caps lock
		CapsLock	=	VK_CAPITAL,
		

		ColonSemicolon = VK_OEM_1,		//;:
		BracketOpen = VK_OEM_4,		//[{
		BracketClose = VK_OEM_6,		//]}
		


	#if SYSTEM==WINDOWS
		A		=	65,					//!< A character key
		B		=	66,					//!< B character key
		C		=	67,					//!< C character key
		D		=	68,					//!< D character key
		E		=	69,					//!< E character key
		F		=	70,					//!< F character key
		G		=	71,					//!< G character key
		H		=	72,					//!< H character key
		I		=	73,					//!< I character key
		J		=	74,					//!< J character key
		K		=	75,					//!< K character key
		L		=	76,					//!< L character key
		M		=	77,					//!< M character key
		N		=	78,					//!< N character key
		O		=	79,					//!< O character key
		P		=	80,					//!< P character key
		Q		=	81,					//!< Q character key
		R		=	82,					//!< R character key
		S		=	83,					//!< S character key
		T		=	84,					//!< T character key
		U		=	85,					//!< U character key
		V		=	86,					//!< V character key
		W		=	87,					//!< W character key
		X		=	88,					//!< X character key
		Y		=	89,					//!< Y character key
		Z		=	90,					//!< Z character key

		OE		=	VK_OEM_3,				//!< O umlaut character key
		AE		=	VK_OEM_7,				//!< A umlaut character key
		UE		=	VK_OEM_1,				//!< U umlaut character key
		SZ		=	VK_OEM_4,				//!< Sz umlaut character key

		Alt		=	18,					//!< Alt key
		PageUp	=	33,					//!< Page scroll up key
		PageDown=	34,					//!< Page scroll down key
		End		=	35,					//!< End key
		Home	=	36,					//!< Home key
		Insert	=	45,					//!< Insert key
		Del		=	0x2E,				//!< Delete key
		Delete	=	0x2E,				//!< Delete key
		PadPlus =	107,				//!< Plus key on the number pad
		PadMinus=	109,				//!< Minus key on the number pad
		Plus	=	187,				//!< Plus key on the main key block
		Minus	=	189,				//!< Minus key on the main key block
		Space	=	32,					//!< Space key
		Enter	=	0x6C,				//!< Enter key
		NumLock =	0x90,				//!< Num lock key
		Caret	=	220,				//!< Caret (^) symbol key
		Mesh	=	191,				//!< Mesh (#)  symbol key
		Comma	=	VK_OEM_COMMA,				//!< Comma key
		Period	=	VK_OEM_PERIOD,				//!< Period key

		N1		=	49,					//!< Digit 1 key
		N2		=	50,					//!< Digit 2 key
		N3		=	51,					//!< Digit 3 key
		N4		=	52,					//!< Digit 4 key
		N5		=	53,					//!< Digit 5 key
		N6		=	54,					//!< Digit 6 key
		N7		=	55,					//!< Digit 7 key
		N8		=	56,					//!< Digit 8 key
		N9		=	57,					//!< Digit 9 key
		N0		=	48,					//!< Digit 0 key

		F1		=	VK_F1,				//!< F1 control key
		F2		=	VK_F2,				//!< F2 control key
		F3		=	VK_F3,				//!< F3 control key
		F4		=	VK_F4,				//!< F4 control key
		F5		=	VK_F5,				//!< F5 control key
		F6		=	VK_F6,				//!< F6 control key
		F7		=	VK_F7,				//!< F7 control key
		F8		=	VK_F8,				//!< F8 control key
		F9		=	VK_F9,				//!< F9 control key
		F10		=	VK_F10,				//!< F10 control key
		F11		=	VK_F11,				//!< F11 control key
		F12		=	VK_F12,					//!< F12 control key
		F13		=	VK_F13,					//!< F12 control key
		F14		=	VK_F14,					//!< F12 control key
		F15		=	VK_F15,					//!< F12 control key
		F16		=	VK_F16,					//!< F12 control key
		F17		=	VK_F17,					//!< F12 control key
		F18		=	VK_F18,					//!< F12 control key
		F19		=	VK_F19,					//!< F12 control key
		F20		=	VK_F20,					//!< F12 control key


	#elif SYSTEM==UNIX

		Q		=	24,					//!< Q character key
		W		=	25,					//!< Wcharacter key
		E		=	26,					//!< E character key
		R		=	27,					//!< R character key
		T		=	28,					//!< T character key
		Z		=	29,					//!< Z character key
		U		=	30,					//!< U character key
		I		=	31,					//!< I character key
		O		=	32,					//!< O character key
		P		=	33,					//!< P character key
		A		=	38,					//!< A character key
		S		=	39,					//!< S character key
		D		=	40,					//!< D character key
		F		=	41,					//!< F character key
		G		=	42,					//!< G character key
		H		=	43,					//!< H character key
		J		=	44,					//!< J character key
		K		=	45,					//!< K character key
		L		=	46,					//!< L character key
		Y		=	52,					//!< Y character key
		X		=	53,					//!< X character key
		C		=	54,					//!< C character key
		V		=	55,					//!< V character key
		B		=	56,					//!< B character key
		N		=	57,					//!< N character key
		M		=	58,					//!< M character key

		OE		=	47,					//!< O umlaut character key
		AE		=	48,					//!< A umlaut character key
		UE		=	34,					//!< U umlaut character key
		SZ		=	20,					//!< Sz umlaut character key

		Alt		=	64,					//!< Alt key
		PageUp	=	99,					//!< Page scroll up key
		PageDown=	105,				//!< Page scroll down key
		End		=	103,				//!< End key
		Home	=	97,					//!< Home key
		Insert	=	106,				//!< Insert key
		Del		=	107,				//!< Delete key
		Delete	=	107,				//!< Delete key
		PadPlus =	86,					//!< Plus key on the number pad
		PadMinus=	82,					//!< Minus key on the number pad
		PadMult	=	63,
		PadDiv	=	109,
		Plus	=	35,					//!< Plus key on the main key block
		Minus	=	61,					//!< Minus key on the main key block
		Space	=	65,					//!< Space key
		Enter	=	108,				//!< Enter key
		NumLock =	77,					//!< Num lock key
		Caret	=	49,					//!< Caret (^) symbol key
		Mesh	=	51,					//!< Mesh (#) symbol key
		Comma	=	59,					//!< Comma key
		Period	=	60,					//!< Period key

		N1		=	10,					//!< Digit 1 key
		N2		=	11,					//!< Digit 2 key
		N3		=	12,					//!< Digit 3 key
		N4		=	13,					//!< Digit 4 key
		N5		=	14,					//!< Digit 5 key
		N6		=	15,					//!< Digit 6 key
		N7		=	16,					//!< Digit 7 key
		N8		=	17,					//!< Digit 8 key
		N9		=	18,					//!< Digit 9 key
		N0		=	19,					//!< Digit 0 key

		F1		=	67,					//!< F1 control key
		F2		=	68,					//!< F2 control key
		F3		=	69,					//!< F3 control key
		F4		=	70,					//!< F4 control key
		F5		=	71,					//!< F5 control key
		F6		=	72,					//!< F6 control key
		F7		=	73,					//!< F7 control key
		F8		=	74,					//!< F8 control key
		F9		=	75,					//!< F9 control key
		F10		=	76,					//!< F10 control key
		F11		=	95,					//!< F11 control key
		F12		=	96,					//!< F12 control key

		F13		=	112,				//I have no idea where these should start
		F14,
		F15,
		F16,
		F17,
		F18,
		F19,
		F20,

	#endif
		LeftMouseButton		=	0x100,
		MiddleMouseButton	=	0x101,
		RightMouseButton	=	0x102,
		MouseButton0		=	LeftMouseButton,
		MouseButton1		=	MiddleMouseButton,
		MouseButton2		=	RightMouseButton,
		MouseButton3		=	0x103,
		MouseButton4		=	0x104,
		MouseButton5		=	0x105,
		MouseButton6		=	0x106,
		MouseButton7		=	0x107,

		SET_JOYSTICK(0)
		SET_JOYSTICK(1)
		SET_JOYSTICK(2)
		SET_JOYSTICK(3)
		SET_JOYSTICK(4)
		SET_JOYSTICK(5)
		SET_JOYSTICK(6)
		SET_JOYSTICK(7)
		SET_JOYSTICK(8)
		SET_JOYSTICK(9)
		SET_JOYSTICK(10)
		SET_JOYSTICK(11)
		SET_JOYSTICK(12)
		SET_JOYSTICK(13)
		SET_JOYSTICK(14)
		SET_JOYSTICK(15)


		Count,
		Max		=	Count-1			//!< Maximum key index
	};
}


#endif
