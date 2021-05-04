#include "console.h"

/******************************************************************

Language/Font-depended console.

******************************************************************/

namespace Engine
{

	static LogFile		console_log("console.log",true);
	#define CONSOLE_LOG(msg) console_log << __func__<<"("<<__LINE__<<"): "<<msg<<nl;


void Console::clearHistory()
{
	lines.clear();
}

void Console::closeEvent()
{
	close();
}

void Console::fetchInput()
{
	CONSOLE_LOG("retrieving input");
    String input = keyboard.GetInputAsUTF8();
	CONSOLE_LOG("input is '"<<input<<"'");
    if (input.IsNotEmpty())
    {
		CONSOLE_LOG("erasing history occurrences");
        inputs.eraseOccurrences(input);
		CONSOLE_LOG("appending to input histpry");
        inputs.add(input);
		CONSOLE_LOG("history modified");
    }
    if (parser)
	{
		CONSOLE_LOG("sending to parser");
        parser(input.c_str());
		CONSOLE_LOG("parser returned");
	}
	CONSOLE_LOG("flushing input");
    keyboard.FlushInput();
	CONSOLE_LOG("reseting selected input");
    selected_input = 0;
	CONSOLE_LOG("input handled");
}

void Console::prevInput()
{
    if (selected_input < inputs.Count())
    {
        selected_input++;
        keyboard.FillInputUTF8(inputs.getReverse(selected_input-1));
    }
}

void Console::nextInput()
{
    if (selected_input)
    {
        selected_input--;
        keyboard.FillInputUTF8(inputs.getReverse(selected_input-1));
    }
}

void Console::back()
{
    keyboard.DropLastCharacter();
}

Console::Console():profile(true,false),selected_input(0),parser(NULL),status(0),is_focused(false),target_open(false)
{
    input.PushProfile();
		input.BindProfile(profile);
		input.Bind(Key::Return,[this](){fetchInput();});
		input.Bind(Key::Back,[this](){back();});
		input.Bind(Key::Up,[this](){prevInput();});
		input.Bind(Key::Down,[this](){nextInput();});
    input.PopProfile();
}

void	Console::bindCloseKey(Key::Name key)
{
	input.PushProfile();
		input.BindProfile(profile);
		input.Bind(key,[this](){closeEvent();});
	input.PopProfile();
}

Console::~Console()
{
    killFocus();
}

void Console::emptyLine()
{
    lines.add("");
}

void Console::print(const String&line)
{
    String local = line;
    while (local.GetLength())
    {
        index_t nl = local.Find('\n');
        if (nl != InvalidIndex)
        {
            lines.add(local.SubStringRef(0,nl));
            local.Erase(0,nl+1);
        }
        else
        {
            lines.add(local);
            return;
        }
    }
}

void Console::setInterpretor(pInputParser interpretor)
{
    parser = interpretor;
}

void Console::focus()
{
    if (is_focused)
        return;
    is_focused = true;
    keyboard.read = true;
    input.PushProfile();
        input.BindProfile(profile);
}

void Console::killFocus()
{
    if (!is_focused)
        return;
    is_focused = false;
    input.PopProfile();
    keyboard.read = false;
}

void Console::unfocus()
{
	killFocus();
}

void Console::forgetLastInput()
{
    inputs.Erase(inputs.Count()-1);
}


void Console::setState(bool open)
{
    target_open = open;
    if (open)
        focus();
    else
        killFocus();
    status = open;
}

void Console::setTarget(bool open)
{
    target_open = open;
    if (open)
        focus();
    else
        killFocus();
}


void Console::open()
{
    target_open = true;
    focus();
}

void Console::close()
{
    target_open = false;
    killFocus();
}


bool Console::focused()	const
{
    return is_focused;
}

bool Console::isOpen()	const
{
	return target_open;
}

void Console::toggle()
{
	setTarget(!target_open);
}

}
