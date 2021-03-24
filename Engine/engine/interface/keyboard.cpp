#include "keyboard.h"


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
	
	char Keyboard::convertInput(char c)
	{
		c = translation[(BYTE)c];
		if (map.pressed[Key::Shift] && c>='a' && c <='z')
			c+='A'-'a';
		return c;
	}


	void Keyboard::input(char c)
	{
		if (read)
		{
			if (!block || block_start.Elapsed() > block_len)
			{
				block = false;
				
				c = translation[(BYTE)c];
				if (map.pressed[Key::Shift] && c>='a' && c <='z')
					c+='A'-'a';

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

	void				Keyboard::CharacterBuffer::SetTo(const char*str, size_t strLen)
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



	void				Keyboard::CharacterBuffer::ReplaceSection(index_t from, size_t len, const char*str, size_t strLen)
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
		
		Array<char> tail;
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

	void			Keyboard::CharacterBuffer::Append(const char*str, size_t strLen)
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



	const char* Keyboard::GetInput()	const
	{
		return internalBuffer.GetCharacters();
	}

	size_t Keyboard::GetInputLen()	const
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

	void	Keyboard::FilterAppend(const char*strn)
	{
		while (*strn)
		{
			input(*strn);
			++strn;
		}
	}

	void	Keyboard::FilterAppend(const String&strn)
	{
		FilterAppend(strn.c_str());

	}


	void Keyboard::FillInput(const String&strn)
	{
		internalBuffer.SetTo(strn);
	}

	void Keyboard::FillInput(const char*strn)
	{
		internalBuffer.SetTo(strn);
	}

	void Keyboard::ReplaceInputSection(index_t offset, size_t len, const String&strn)
	{
		internalBuffer.ReplaceSection(offset,len,strn);
	}

	void Keyboard::ReplaceInputSection(index_t offset, size_t len, const char*strn)
	{
		internalBuffer.ReplaceSection(offset,len,strn);
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
	{
		Filter("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789°!\"§$%&/()=?´`{[]}\\#+'*~-_.:,;|<> ");

		translation[(BYTE)'â'] = 'a';
		translation[(BYTE)'ô'] = 'o';
		translation[(BYTE)'û'] = 'u';
		translation[(BYTE)'î'] = 'i';
		translation[(BYTE)'ê'] = 'e';
		translation[(BYTE)'Â'] = 'A';
		translation[(BYTE)'Ô'] = 'O';
		translation[(BYTE)'Û'] = 'U';
		translation[(BYTE)'Î'] = 'I';
		translation[(BYTE)'Ê'] = 'E';

		translation[(BYTE)'à'] = 'a';
		translation[(BYTE)'ò'] = 'o';
		translation[(BYTE)'ù'] = 'u';
		translation[(BYTE)'ì'] = 'i';
		translation[(BYTE)'è'] = 'e';
		translation[(BYTE)'À'] = 'A';
		translation[(BYTE)'Ò'] = 'O';
		translation[(BYTE)'Ù'] = 'U';
		translation[(BYTE)'Ì'] = 'I';
		translation[(BYTE)'È'] = 'E';
	}


	void Keyboard::Filter(const char*characters)
	{
		memset(translation,0,sizeof(translation));
		unsigned index = 0;
		while (characters[index])
		{
			translation[(BYTE)characters[index]] = characters[index];
			index++;
		}
	}

	void Keyboard::Allow(const char*characters)
	{
		unsigned index = 0;
		while (characters[index])
		{
			translation[(BYTE)characters[index]] = characters[index];
			index++;
		}
	}

	void Keyboard::Disallow(const char*characters)
	{
		unsigned index = 0;
		while (characters[index])
			translation[(BYTE)characters[index++]] = 0;
	}

	void Keyboard::Translate(const char*from, const char*to)
	{
		while (*from && *to)
			translation[(BYTE)*from++] = *to++;
	}


}
