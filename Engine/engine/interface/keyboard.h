#ifndef engine_interface_keyboardH
#define engine_interface_keyboardH

/******************************************************************

engine keyboard-interface.

******************************************************************/


#include "../timing.h"
#include <io/config.h>

#include <string/keys.h>

#if SYSTEM==UNIX
	#include <X11/Xlib.h>
#endif

#include "input.h"


namespace Engine
{

	using namespace Time::Literals;


	
	/*!
		\brief 	Keyboard interface class.
		
		The Keyboard class acts as an interface helping with the interaction between an application and the user keyboard.
		It is currently used by the Eve engine and the hive multi-window interface.
	*/

	class Keyboard
	{
	public:
		/**
		Overwrites the specified character section with zeros
		*/
		static void			WipeChars(char*c,size_t len)
		{
			volatile char*vp = c;
			for (index_t i = 0; i < len; i++)
				vp[i] = 0; 
		}

		/**
		Plugin to be used with SecureStrategy.
		Only a single method is redefined to make sure data is properly wiped
		*/
		class SecurePlugin : public PrimitiveStrategyPlugin
		{
		public:
			template <typename T>
				static	inline	void	destructRange(T*begin, T*end)
				{
					WipeChars((char*)begin,((char*)end)-((char*)begin));
				}
		};

		/**
		Strategy to be used with character buffers that makes sure characters are wipe before being freed.
		Without the use of this strategy it would be impossible to catch all cases where the Buffer class internally discards used memory
		*/
		typedef CommonStrategy<SecurePlugin>	SecureStrategy;

		class CharacterBuffer : private Ctr::Buffer0<char,SecureStrategy>
		{
			typedef Ctr::Buffer0<char,SecureStrategy>	Super;


		public:
			/**/				CharacterBuffer()
			{
				Super::Append('\0');
			}
			//virtual				~CharacterBuffer()
			//{
			//	Wipe();
			//}
			/**
			Appends a character to the end of the buffer. Makes sure the internal buffer is zero-terminated
			*/
			void				Append(char c)	{Super::Last() = c; if (c != 0) Super::Append('\0'); DBG_ASSERT__(IsSane());}
			void				Append(const char*, size_t);
			/**
			Wipes and drops the last stored character (if any)
			*/
			void				EraseLast()		{if (IsEmpty()) return; Super::EraseLast(); Super::Last() = 0; DBG_ASSERT__(IsSane());}

			void				SetTo(const String&str)		{SetTo(str.c_str(),str.GetLength());}
			void				SetTo(const char*str)		{SetTo(str,strlen(str));}
			void				SetTo(const char*, size_t);

			/**
			Replaces a section in the local buffer. Can be used to insert, delete, or replace
			@param from Index of the first character to replace (if len > 0), starting from 0 (=first char)
			@param len Length of the section to replace. Can be zero to allow insert operations, making @param from the first character after the inserted characters
			@param str Characters to insert into the section. Does not have to be of the same length as @a len. Can be empty making this a deletion operation
			*/
			void				ReplaceSection(index_t from, size_t len, const String&str)	{ReplaceSection(from,len,str.c_str(),str.GetLength());}
			void				ReplaceSection(index_t from, size_t len, const char*str)	{ReplaceSection(from,len,str,strlen(str));}
			void				ReplaceSection(index_t from, size_t len, const char*str, size_t strLen);

			bool				IsEmpty()	const {return Super::GetLength() <= 1;}
			bool				IsNotEmpty() const {return Super::GetLength() > 1;}
			/**
			Retrieves the number of stored characters, excluding the terminating zero.
			*/
			size_t				GetLength()		const {return Super::GetLength()-1;}
			/**
			Retrieves a pointer to the immutable stored data. Can never be null. Always zero-terminated
			*/
			const char*			GetCharacters()	const {DBG_ASSERT__(IsSane()); return Super::pointer();}
			/**
			Overwrites every stored character and clears the buffer
			*/
			void				Wipe()			{WipeChars(Super::pointer(),Super::GetLength()); Super::Clear();Super::Append('\0'); DBG_ASSERT__(IsSane());}

			void				Truncate(size_t toLength);

			void				swap(CharacterBuffer&other)	{Super::swap(other); DBG_ASSERT__(IsSane()); DBG_ASSERT__(other.IsSane());}
			friend void			swap(CharacterBuffer&a, CharacterBuffer&b)	{a.swap(b);}
			bool				IsSane()	const;
			
		};


	private:
			char				translation[0x100];
			bool				block;
			Time::Stamp			block_start;
			Time::Delta			block_len;
			BYTE				on_char_input;
			unsigned			delay_started,delay;
			CharacterBuffer		internalBuffer;
	public:
			void				keyDown(Key::Name key);		//!< Triggers key down event handling for the specified key. Usually invoked automatically by the surrounding event handling system. \param key Key-index
			void				keyUp(Key::Name key);		//!< Triggers key up event handling for the specified key. Usually invoked automatically by the surrounding event handling system. \param key Key-index
			void				input(char c);			//!< Triggers char event handling for the specified character.  Usually invoked automatically by the surrounding event handling system. \param c Character
			char				convertInput(char c);	//!< Converts a character using the internal conversion map. If the character is not valid or not mapped then 0 is returned

	#if SYSTEM==WINDOWS
	friend	LRESULT CALLBACK	WndProc(HWND hWnd, UINT Msg, WPARAM wParam,LPARAM lParam);
	#elif SYSTEM==UNIX
	friend	void				ExecuteEvent(XEvent&event);
	#endif

	public:
		typedef void	(*charReader)	(char);
		
			
		InputMap		&map;
		bool			read;					//!< Species whether or not character inputs should be handled.
		charReader		reader,					//!< Pointer to an external character handling function. Set NULL to internally handle character input (if \b read is set true)
						onCharHandled;			//!< Pointer to be notified when a char has been handled. Effective only if @a reader is NULL.


		/**/			Keyboard(InputMap&map);
		virtual			~Keyboard();
		const char*		GetInput() const;				//!< Returns the internal character buffer content.  \return Character array pointer pointing to the internal character buffer. The returned string is zero terminated.
		size_t			GetInputLen() const ;			//!< Queries buffer Fill state. \return Number of characters currently in the internal buffer (not counting any trailing zero character).
		void			FlushInput();					//!< Wipes and dumps the internal character buffer.
		void			FillInput(const char*strn);		//!< Fills the specified string into the internal character buffer overwriting any previous buffer content. \param strn String to Fill in.
		void			FillInput(const String&strn);	//!< Fills the specified string into the internal character buffer overwriting any previous buffer content. \param strn String to Fill in.
		void			FilterAppend(const char*strn);		//!< Appends the specified string to the internal character buffer. Only characters passing the character filter are considered. \param strn String to append
		void			FilterAppend(const String&strn);	//!< @copydoc FilterAppend(const char*)
		void			DropLastCharacter();			//!< Drops one character from the end of the internal character buffer reducing the buffer Fill state by one. The method has no effect if the internal buffer is empty.
		void			TruncateIfLonger(size_t len);										//!< Wipes and dumps a number of characters from the end of the internal character buffer so that its content is at most \b len characters long after execution. \param len Maximum number of characters, the internal buffer should contain after execution, not counting the terminating zero.
		void			ReplaceInputSection(index_t offset, size_t len, const String&string);	//!< Replaces a section of the internal character buffer with the specified string. \param offset Index of the first character to remove (0=first character in the buffer) \param len Number of characters to remove \param strn String to replace the removed section with. The string is not required to be of the same length as the section it replaces.
		void			ReplaceInputSection(index_t offset, size_t len, const char*string);	//!< Replaces a section of the internal character buffer with the specified string. \param offset Index of the first character to remove (0=first character in the buffer) \param len Number of characters to remove \param strn String to replace the removed section with. The string is not required to be of the same length as the section it replaces.
		void			DelayRead(Time::Delta seconds=0.1_s);									//!< Enables character input handling after the specified time interval has passed. \param seconds Seconds to wait before enabling character handling.

		void			ResetTranslation();					//!< Resets the internal input character translation to the internal default.
		void			Filter(const char*characters);		//!< Modifies the internal input character translation to only allow the specified characters. \param characters Characters to allow. All non-specified characters will be blocked.
		void			Allow(const char*characters);		//!< Modifies the internal input character translation to allow the specified characters. \param characters Characters to allow. Non-specified character translations are not modified.
		void			Disallow(const char*characters);			//!< Modifies the internal input character translation to disallow the specified characters. \param characters Characters to block. Non-specified character translations are not modified.
		void			Translate(const char*from, const char*to);	//!< Modifies the internal input character translation. \param from Character field to translate from \param to Character field to translate to. Must be of the same length as \b from.

	
		void			ExportBuffer(CharacterBuffer&to)	{internalBuffer.swap(to); internalBuffer.Wipe();}
		void			ImportBuffer(CharacterBuffer&from)	{internalBuffer.swap(from); from.Wipe();}
	};


	extern Keyboard keyboard;
}

#endif
