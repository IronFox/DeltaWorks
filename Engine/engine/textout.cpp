#include "textout.h"

/******************************************************************

Font-dependent textout-manager. The appearance of the
resulting render depends on the used font.

******************************************************************/

namespace Engine
{

	VirtualTextout::VirtualTextout():stackDepth(0),colorStackDepth(0)
	{
		M::Vec::set(state,1);
	    state.left = 0;
	    state.top = 1;
		state.depth = 0;
	    state.x_scale = 1;
	    state.y_scale = 1;
	    state.lineOffset = 0;
		state.indent = 0;
	}

	void VirtualTextout::NewLine()
	{
		state.indent = 0;
		state.lineOffset += QueryScaledHeight();
	}

	void	VirtualTextout::Tint(float red, float green, float blue)
	{
		state.red *= red;
		state.green *= green;
		state.blue *= blue;
	}


	void	VirtualTextout::SetColor(const M::TVec3<>&color)
	{
		state.rgb = color;
		state.alpha = 1;
	}
	void	VirtualTextout::SetColor(const M::TVec3<double>&color)
	{
		M::Vec::copy(color,state.rgb);
		state.alpha = 1;
	}
	void	VirtualTextout::SetColor(const M::TVec3<>&color,float alpha)
	{
		state.rgb = color;
		state.alpha = alpha;
	}


	void	VirtualTextout::SetColor(const M::TVec3<double>&color, double alpha)
	{
		M::Vec::copy(color,state.rgb);
		state.alpha = alpha;
	}

	void	VirtualTextout::SetColor(const M::TVec4<>&color)
	{
		M::Vec::copy(color,state);
	}

	void	VirtualTextout::SetColor(const M::TVec4<double>&color)
	{
		M::Vec::copy(color,state);
	}

	
	void VirtualTextout::SetColor(float red, float green, float blue, float alpha)
	{
	    M::Vec::def(state,red,green,blue,alpha);
	}

	void VirtualTextout::SetColor4fv(const float*color)
	{
		M::Vec::copy(M::Vec::ref4(color),state);
	}

	void VirtualTextout::SetColor4dv(const double*color)
	{
		M::Vec::copy(M::Vec::ref4(color),state);
	}

	void VirtualTextout::SetColor(float red, float green, float blue)
	{
	    M::Vec::def(state,red,green,blue,1);
	}

	void VirtualTextout::SetColor3fv(const float*color)
	{
		M::Vec::copy(M::Vec::ref3(color),state.rgb);
	    state.alpha = 1;
	}

	void VirtualTextout::SetColor3dv(const double*color)
	{
		M::Vec::copy(M::Vec::ref3(color),state.rgb);
	    state.alpha = 1;
	}

	void VirtualTextout::SetColor3fv(const float*color, float alpha)
	{
		M::Vec::copy(M::Vec::ref3(color),state.rgb);
	    state.alpha = alpha;
	}

	void VirtualTextout::SetColor3dv(const double*color, float alpha)
	{
		M::Vec::copy(M::Vec::ref3(color),state.rgb);
	    state.alpha = alpha;
	}
	
	
	
	

	void VirtualTextout::MoveBy(float x, float y, float z)
	{
	    state.left += x;
	    state.top += y;
		state.depth += z;
		state.lineOffset = 0;
		state.indent = 0;
	}


	void VirtualTextout::MoveTo(float x, float y, float z)
	{
	    state.left = x;
	    state.top = y;
		state.depth = z;
		state.lineOffset = 0;
		state.indent = 0;
	}

	void VirtualTextout::MoveTo(const M::TVec3<>&p)
	{
		MoveTo(p.x,p.y,p.z);
	}

	void VirtualTextout::MoveTo(const M::TVec2<>&p)
	{
		MoveTo(p.x,p.y);
	}



	void VirtualTextout::MoveTo(const float p[2])
	{
		MoveTo(p[0],p[1]);
	}

	void VirtualTextout::MoveTo3fv(const float p[3])
	{
		MoveTo(p[0],p[1],p[2]);
	}

	void VirtualTextout::SetScale(float x, float y)
	{
	    state.x_scale=x;
	    state.y_scale=y;
	}

	void VirtualTextout::Scale(float by_x, float by_y)
	{
	    state.x_scale*=by_x;
	    state.y_scale*=by_y;
	}


	void VirtualTextout::PushState()
	{
		DBG_ASSERT__(stackDepth < 0xFF);
	    stack[stackDepth++] = state;
	}

	void VirtualTextout::PopState()
	{
		DBG_ASSERT__(stackDepth > 0);
	    state = stack[--stackDepth];
	}


	void VirtualTextout::PushColorState()
	{
	    colorStack[colorStackDepth++] = state;
	}

	void VirtualTextout::PopColorState()
	{
	    ((TFontColor&) state) = colorStack[--colorStackDepth];
	}
	
	void VirtualTextout::SetLine(int line_)
	{
	    state.lineOffset = float(line_) * QueryScaledHeight();
	}

	void VirtualTextout::SetLineOffset(float offset)
	{
	    state.lineOffset = offset;
	}

	float VirtualTextout::GetLineOffset()	const
	{
	    return state.lineOffset;
	}

	

	
	float VirtualTextout::QueryScaledWidth(const char*line) const
	{
		return QueryUnscaledWidth(StringRef(line))*state.x_scale;
	}

	float VirtualTextout::QueryScaledWidth(const String&line) const
	{
		return QueryUnscaledWidth(line.ToRef())*state.x_scale;
	}
	float VirtualTextout::QueryScaledWidth(const StringRef&line) const
	{
		return QueryUnscaledWidth(line)*state.x_scale;
	}
	
	float VirtualTextout::QueryScaledWidth(const char32_t*line) const
	{
		return QueryUnscaledWidth(UTF32StringRef(line))*state.x_scale;
	}

	float VirtualTextout::QueryScaledWidth(const UTF32String&line) const
	{
		return QueryUnscaledWidth(line.ToRef())*state.x_scale;
	}
	float VirtualTextout::QueryScaledWidth(const UTF32StringRef&line) const
	{
		return QueryUnscaledWidth(line)*state.x_scale;
	}

	float VirtualTextout::QueryScaledHeight() const
	{
		return QueryUnscaledHeight() * state.y_scale;
	}



BYTE hexChar(char c)
{
    if (c>='0' && c <= '9')
        return c-0x30;
    if (c>='a' && c <= 'f')
        return 10+c-'a';
    if (c>='A' && c <= 'F')
        return 10+c-'A';
    return 0;
}


BYTE hexStr(const char*line)
{
    return hexChar(line[0])*0x10+hexChar(line[1]);
}

}
