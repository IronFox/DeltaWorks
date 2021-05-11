#include "keyboard.h"
#include <string/encoding.h>


/******************************************************************

engine keyboard-interface.

******************************************************************/

namespace Engine
{


	Keyboard keyboard(input);

	
	void Keyboard::keyDown(Key::Name index)
	{
		map.keyDown(index);
	}

	void Keyboard::keyUp(Key::Name index)
	{
		map.keyUp(index);
	}
	


	void Keyboard::input(char32_t c)
	{
		if (read)
		{
			if (!block || block_start.Elapsed() > block_len)
			{
				block = false;
				
				if (StringFunctions::IsInvalidChar(c) || c == '\n' || (filter && !filter(c)))
					return;
				if (map.pressed[Key::Shift])
					c = StringFunctions::toupper(c);

				if (c)
				{
					if (reader)
						reader(c);
					else
					{
						internalBuffer.Append(c);
						if (onCharHandled)
							onCharHandled(c);
					}
				}
			}
		}
	}


	void				Keyboard::CharacterBuffer::Truncate(size_t toLength)
	{
		if (GetLength() <= toLength)
			return;
		WipeChars(Super::pointer()+toLength,GetLength()-toLength);
		Super::Truncate(toLength+1);
		Super::at(toLength)=0;
		DBG_ASSERT__(IsSane());
	}

	void				Keyboard::CharacterBuffer::SetToUTF8(const StringRef&ref)
	{
		UTF32String str;
		StringEncoding::UTF8::ToUTF32(ref,str);
		SetTo(str);
	}

	void				Keyboard::CharacterBuffer::SetTo(const Character*str, size_t strLen)
	{
		Wipe();
		Append(str,strLen);
		DBG_ASSERT__(IsSane());
	}


	bool				Keyboard::CharacterBuffer::IsSane() const
	{
		if (Super::IsEmpty())
			return false;
		for (index_t i = 0; i < GetLength(); i++)
			if (Super::at(i) == 0)
				return false;
		return Super::Last() == 0;
	}



	void				Keyboard::CharacterBuffer::ReplaceSection(index_t from, size_t len, const Character*str, size_t strLen)
	{
		DBG_ASSERT__(IsSane());
		if (len == 0 && strLen == 0)
			return;
		if (from + len > GetLength())
		{
			if (from >= GetLength())
			{
				Append(str,strLen);

				return;
			}
			len = GetLength() - from;
		}
		
		Array<Character> tail;
		size_t tailLength = GetLength() - (from+len);
		if (tailLength)
		{
			tail.SetSize(tailLength);
			memcpy(tail.pointer(),Super::pointer()+from+len,tailLength);
			Truncate(from);
		}
		Append(str,strLen);
		if (tailLength)
		{
			Append(tail.pointer(),tailLength);
			WipeChars(tail.pointer(),tailLength);
		}
		DBG_ASSERT__(IsSane());
	}

	void			Keyboard::CharacterBuffer::Append(const Character*str, size_t strLen)
	{
		if (strLen == 0)
			return;
		DBG_ASSERT__(IsSane());
		Super::EraseLast();	//erase terminating zero
		for (index_t i = 0; i < strLen; i++)
			if (str[i])
				Super::Append(str[i]);
		Super::Append(0);
		DBG_ASSERT__(IsSane());
	}



	UTF32StringRef Keyboard::GetRawInput()	const
	{
		return internalBuffer.ToRef();
	}

	count_t Keyboard::GetRawInputLen()	const
	{
		return internalBuffer.GetLength();
	}


	void Keyboard::DropLastCharacter()
	{
		internalBuffer.EraseLast();
	}

	void Keyboard::TruncateIfLonger(size_t len)
	{
		internalBuffer.Truncate(len);
	}

	void	Keyboard::FilterAppendUTF8(const char*strn)
	{
		const auto end = strn + StringFunctions::strlen(strn);
		while (strn < end)
		{
			char32_t c32;
			if (!StringEncoding::UTF8::ToUTF32(strn,end,c32))
				return;
			input(c32);
		}
	}

	void	Keyboard::FilterAppendUTF8(const String&str)
	{
		auto strn = str.c_str();
		const auto end = strn + str.GetLength();
		while (strn < end)
		{
			char32_t c32;
			if (!StringEncoding::UTF8::ToUTF32(strn,end,c32))
				return;
			input(c32);
		}
	}

	String	Keyboard::GetInputAsUTF8() const
	{
		String str;
		StringEncoding::UTF32::ToUTF8(GetRawInput(),str);
		return str;
	}


	void Keyboard::FillInputUTF8(const String&strn)
	{
		internalBuffer.SetToUTF8(strn);
	}

	void Keyboard::FillInputUTF8(const char*strn)
	{
		internalBuffer.SetToUTF8(strn);
	}

	void Keyboard::ReplaceInputSectionUTF8(index_t offset, size_t len, const String&strn)
	{
		UTF32String str;
		StringEncoding::UTF8::ToUTF32(strn,str);
		internalBuffer.ReplaceSection(offset,len,str);
	}

	void Keyboard::ReplaceInputSectionUTF8(index_t offset, size_t len, const char*strn)
	{
		UTF32String str;
		StringEncoding::UTF8::ToUTF32(strn,str);
		internalBuffer.ReplaceSection(offset,len,str);
	}



	void Keyboard::FlushInput()
	{
		internalBuffer.Wipe();
	}








	Keyboard::Keyboard(InputMap&map_):block(false),map(map_),read(false),reader(NULL),onCharHandled(NULL)
	{
		ResetTranslation();
	}

	Keyboard::~Keyboard()
	{}

	void Keyboard::DelayRead(Time::Delta len)
	{
		block = true;
		block_len = len;
		block_start.Update();
	}


	void Keyboard::ResetTranslation()
	{}


	void Keyboard::SetFilter(const FAllowed&filter)
	{
		this->filter = filter;
	}



}
