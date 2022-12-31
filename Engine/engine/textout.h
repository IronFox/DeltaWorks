#ifndef engine_textoutH
#define engine_textoutH

/******************************************************************

Font-dependent textout-manager. The appearance of the
resulting render depends on the used font.

******************************************************************/



#include <global_string.h>
#include <math/vector.h>
#include <io/str_stream.h>

namespace Engine
{
	using namespace DeltaWorks;

	BYTE hexStr(const char*line);
	BYTE hexChar(char c);

	enum    FontStyle
	{
		Normal,
		Bold,
		Italic,
		NormalStyle = Normal,
		BoldStyle=Bold,
		ItalicStyle=Italic
	};


	typedef M::TVec4<>	TFontColor;
			

	struct TFontState:public TFontColor		//! Font status fragment
	{
			float		left,				//!< Current root position (x)
						top,				//!< Current root position (y)
						depth,				//!< Current root position (z)
						indent;				//!< Current indentation offset (x)
		union
		{
			struct
			{
				float	x_scale,			//!< Current font scale (width)
						y_scale;			//!< Current font scale (height)
			};
			struct
			{
				float	scale_x,			//!< Current font scale (width)
						scale_y;			//!< Current font scale (height)
			};
		};
			float		lineOffset;				//!< Current line
	};

	/*
	template class EveFont	//the height of a character must always be 1
	{
		float	GetWidth(const char*str);
		float	GetWidth(const StringRef&str);
		float	GetWidth(char);
		void	begin(const TFontState&state);
		void	AlterColor(const TFontColor&color);
		void	Write(const char*str);
		void	Write(const StringRef&str);
		void	end();
	}
	*/

	/**
		@brief Virtual textout control class
		
		Text output is passed to virtual methods which may be implemented in various ways
	*/
	class VirtualTextout
	{
	protected:
		TFontState		stack[0x100];
		TFontColor		colorStack[0x100];
		BYTE			stackDepth,colorStackDepth;
			
			
		virtual	void	BeginOutput()=0;
		virtual	void	WriteSegment(const char*field, size_t len)=0;
		virtual	void	WriteSegment(const char32_t*field, size_t len)=0;
		virtual	void	EndOutput()=0;
		virtual	void	AlterColor(const TFontColor&color)=0;
	public:
		TFontState		state;	//!< Current output status


			
			
		/**/			VirtualTextout();
		virtual			~VirtualTextout()	{};
			
	
		void			SetColor(float red, float green, float blue, float alpha);	//!< Respecify the current textout color \param red Red color segment (0 ... 1) \param green Green color segment (0 ... 1)	\param blue Blue color segment (0 ... 1)	\param alpha Opacity (0 = invisible ... 1 = opaque)
		void			SetColor4fv(const float*color);								//!< Respecify the current textout color via a 4 element array \param color 4 segment float array in rgba order.
		void			SetColor4dv(const double*color);							//!< Respecify the current textout color via a 4 element array \param color 4 segment double array in rgba order.
		void			SetColor(float red, float green, float blue);				//!< Respecify the current textout color (opaque) \param red Red color segment (0 ... 1) \param green Green color segment (0 ... 1)	\param blue Blue color segment (0 ... 1) 
		void			SetColor(const M::TVec3<>&color);
		void			SetColor(const M::TVec3<double>&color);
		void			SetColor(const M::TVec3<>&color,float alpha);
		void			SetColor(const M::TVec3<double>&color, double alpha);
		void			SetColor(const M::TVec4<>&color);
		void			SetColor(const M::TVec4<double>&color);
		void			SetColor3fv(const float*color);							//!< Respecify the current textout color via a 3 element array (opaque) \param color 3 segment float array in rgb order.
		void			SetColor3dv(const double*color);						//!< Respecify the current textout color via a 3 element array (opaque) \param color 3 segment double array in rgb order.
		void			SetColor3fv(const float*color, float alpha);			//!< Respecify the current textout color via a 3 element array and an alpha value \param color 3 segment float array in rgb order. @param alpha Opacity (0 = invisible ... 1 = opaque)
		void			SetColor3dv(const double*color, float alpha);			//!< Respecify the current textout color via a 3 element array and an alpha value \param color 3 segment double array in rgb order. @param alpha Opacity (0 = invisible ... 1 = opaque)
		void			Tint(float red, float green, float blue);				//!< Multiplies the current font color with the specified color
				
		virtual float	QueryUnscaledHeight() const=0;							//!< Determines the height of a string in its native (unscaled) size. Virtual call
		virtual	float	QueryUnscaledWidth(const StringRef&) const=0;			//!< Determines the width of a string in its native (unscaled) size. Virtual call \param line Pointer to a field of characters containing the string to analyse \param len Number of characters to analyse out of line \return Unscaled length of the specified string
		virtual	float	QueryUnscaledWidth(const UTF32StringRef&) const=0;		//!< Determines the width of a string in its native (unscaled) size. Virtual call \param line Pointer to a field of characters containing the string to analyse \param len Number of characters to analyse out of line \return Unscaled length of the specified string
		float			QueryUnscaledWidth(const String&line) const				{return QueryUnscaledWidth(line.ref());}
		float			QueryUnscaledWidth(const UTF32String&line) const		{return QueryUnscaledWidth(line.ref());}
		float			QueryScaledWidth(const char*line) const;				//!< Determines the width of a string. Hidden virtual call \param line Pointer to a zero terminated character array containing the string to analyse \return Scaled length of the specified string
		float			QueryScaledWidth(const String&line) const;				//!< Determines the width of a string. Hidden virtual call \param line String to analyse \return Scaled length of the specified string
		float			QueryScaledWidth(const StringRef&line) const;			//!< Determines the width of a string. Hidden virtual call \param line String to analyse \return Scaled length of the specified string
		float			QueryScaledWidth(const char32_t*line) const;			//!< Determines the width of a string. Hidden virtual call \param line Pointer to a zero terminated character array containing the string to analyse \return Scaled length of the specified string
		float			QueryScaledWidth(const UTF32String&line) const;			//!< Determines the width of a string. Hidden virtual call \param line String to analyse \return Scaled length of the specified string
		float			QueryScaledWidth(const UTF32StringRef&line) const;		//!< Determines the width of a string. Hidden virtual call \param line String to analyse \return Scaled length of the specified string
		float			QueryScaledHeight() const;								//!< Determines the height of a string. Hidden virtual call \return Scaled height of a single line

		void			NewLine();												//!< Increases line count and resets current line indentation to 0. Hidden virtual call
		void			MoveBy(float by_x, float by_y, float by_z=0);				//!< Move the out-cursor. Also resets the active line to 0. \param by_x X-delta \param by_y Y-delta \param by_z Optional Z-delta
		void			MoveTo(float x, float y, float z=0);					//!< Position the top-left corner of the out-cursor at a new location. Also resets the active line to 0. \param x X-coordinate \param y Y-coordinate \param z Optional z coordinate
		void			MoveTo(const M::TVec2<>&p);								//!< Position the top-left corner of the out-cursor at a new location. Also resets the active line to 0.
		void			MoveTo(const M::TVec3<>&p);								//!< Position the top-left corner of the out-cursor at a new location. Also resets the active line to 0.
		void			MoveTo(const float p[2]);								//!< Position the top-left corner of the out-cursor at a new location. Also resets the active line to 0. \param p Two component position vector
		void			MoveTo3fv(const float p[3]);							//!< Position the top-left corner of the out-cursor at a new location. Also resets the active line to 0. \param p Two component position vector
		void			SetScale(float x, float y);								//!< Redefine character scale \param x Character width \param y Character height
		void			SetScale(float xy)				{SetScale(xy,xy);}
		void			Scale(float byX, float byY);							//!< Alter the character scale \param byX Width-factor \param byY Height-factor
		void			Scale(float by)					{Scale(by,by);}
		void			PushState();											//!< Pushes the current output status to the local stack
		void			PushColorState();
		void			PopState();												//!< Restores the current output status from the local stack
		void			PopColorState();
		void			SetLineOffset(float offset);
		void			SetLine(int line_index);								//!< Changes the active line. Hidden virtual call \param line_index Line to change to (0=first line)
		float			GetLineOffset()	const;									//!< Queries the current line offset (increased by newlines)

		template <typename Char>
			void		Write(const StringType::ReferenceExpression<Char>&str);								//!< Prints a string segment with a predefined length. Hidden virtual call @param str Pointer to an array of characters containing at least @b len characters @param len Number of characters to print
		template <typename Char>
			void		Write(const Char*str)	{Write(StringType::ReferenceExpression<Char>(str));}
		template <typename Char>
			void		Write(const StringType::Template<Char>&str) {Write(str.ToRef());}
		template <typename Char>
			void		WriteTagged(const Char*str, Char tag)	{WriteTagged(StringType::ReferenceExpression<Char>(str),tag);}
		template <typename Char>
			void		WriteTagged(const StringType::Template<Char>&str, Char tag)	{WriteTagged(str.ToRef(),tag);}
		template <typename Char>
			void		WriteTagged(const StringType::ReferenceExpression<Char>&str, Char tag);
		template <typename Char>
			void		WriteLn(const Char*str)	{WriteLn(StringType::ReferenceExpression<Char>(str));}
		template <typename Char>
			void		WriteLn(const StringType::Template<Char>&str) {WriteLn(str.ToRef());}
		template <typename Char>
			void		WriteLn(const StringType::ReferenceExpression<Char>&str)
			{
				Write(str);
				state.lineOffset += QueryScaledHeight();
			}
		template <typename Char>
			void		WriteTaggedLine(const Char*str, Char tag) {WriteTaggedLine(StringType::ReferenceExpression<Char>(str),tag);}
		template <typename Char>
			void		WriteTaggedLine(const StringType::Template<Char>&str, Char tag) {WriteTaggedLine(str.ToRef(),tag);}
		template <typename Char>
			void			WriteTaggedLine(const StringType::ReferenceExpression<Char>&str, Char tag)
			{
				WriteTagged(str,tag);
				state.lineOffset += QueryScaledHeight();
			}
	};
	
	
	/*!
		\brief Textout control class

		The template Textout class acts as a wrapper class for the underlying font object. It controls the current cursor position, color, scale and line of a compatible font object.
		Currently working with this module are the classes EveGLOutlineFont (unfortunately windows only), EveGLTextureFont and (with certain limitations) EveGLRasterFont.
		Each Textout instance provides its own font object but may use an external font object (of the same type) as well.
	*/

	template <class Font>
		class Textout:public VirtualTextout
		{
			typedef VirtualTextout	Super;
		protected:
			Font			localFont,*activeFont;
				
			virtual	void	BeginOutput() override;
			virtual	void	WriteSegment(const char*field, size_t len) override;
			virtual	void	WriteSegment(const char32_t*field, size_t len) override;
			virtual	void	EndOutput() override;
			virtual	void	AlterColor(const TFontColor&color) override;
			template <typename Char>
				void		Stream(const Char*str, size_t len);
			template <typename T>	
				void		StreamUnsigned(T value);
			template <typename T, typename UT>
				void		StreamSigned(T value);
			template <typename T>
				void		StreamFloat(T value, unsigned precision);
		public:
			typedef Textout<Font>	Self;
				
			/**/			Textout();
			void			SetFont(Font*font);										//!< Adopt an external font object of template type Font \param font Pointer to an external font object. Passing NULL will cause Textout to switch back to its internal font object.
			Font&			GetFont();												//!< Get a reference to the internal font object \return Reference to the internal font object
			Font*			GetActiveFont();										//!< Get a pointer to the currently active font object. This may be the internal object or an external font object assigned via SetFont(...). \return Pointer to the currently used font object.
	
			float			GetUnscaledWidth(const StringRef&) const;				//!< Determines the length of a string in its native (unscaled) size \param line Pointer to a field of characters containing the string to analyse \param len Number of characters to analyse out of line \return Unscaled length of the specified string
			float			GetUnscaledWidth(const String&line)	const				{return GetUnscaledWidth(line.ref());}
			float			GetUnscaledWidth(const char*line)	const				{return GetUnscaledWidth(StringRef(line));}
			float			GetUnscaledWidth(const UTF32StringRef&) const;			//!< Determines the length of a string in its native (unscaled) size \param line Pointer to a field of characters containing the string to analyse \param len Number of characters to analyse out of line \return Unscaled length of the specified string
			float			GetUnscaledWidth(char32_t c) const						{return activeFont->GetWidth(c);}
			float			GetUnscaledHeight() const								{return activeFont->GetHeight();}
			float			GetScaledWidth(const char*line) const					{return GetScaledWidth(StringRef(line));}
			float			GetScaledWidth(const String&line) const					{return GetScaledWidth(line.ref());}
			float			GetScaledWidth(const StringRef&line) const;				//!< Determine the length of a string \param line String to analyse \return Scaled length of the specified string
			float			GetScaledWidth(char c) const							{return GetUnscaledWidth(c)*state.x_scale;}
			float			GetScaledWidth(const char32_t*line) const					{return GetScaledWidth(UTF32StringRef(line));}
			float			GetScaledWidth(const UTF32String&line) const					{return GetScaledWidth(line.ref());}
			float			GetScaledWidth(const UTF32StringRef&line) const;				//!< Determine the length of a string \param line String to analyse \return Scaled length of the specified string
			float			GetScaledWidth(char32_t c) const							{return GetUnscaledWidth(c)*state.x_scale;}
			float			GetScaledHeight() const									{return activeFont->GetHeight()*state.y_scale;}
			virtual	float	QueryUnscaledWidth(const StringRef&line) const override;
			virtual	float	QueryUnscaledWidth(const UTF32StringRef&line) const override;
			virtual	float	QueryUnscaledHeight() const override;

			Textout<Font>&	operator<<(const String&);
			Textout<Font>&	operator<<(const UTF32String&);
			Textout<Font>&	operator<<(const StringRef&);
			Textout<Font>&	operator<<(const UTF32StringRef&);
			Textout<Font>&	operator<<(const char*);
			Textout<Font>&	operator<<(const TNewLine&);
			Textout<Font>&	operator<<(const TSpace&);
			Textout<Font>&	operator<<(const TTabSpace&);
			Textout<Font>&	operator<<(char);
			Textout<Font>&	operator<<(BYTE);
			Textout<Font>&	operator<<(long long);
			Textout<Font>&	operator<<(unsigned long long);
			Textout<Font>&	operator<<(int);
			Textout<Font>&	operator<<(unsigned);
			Textout<Font>&	operator<<(long);
			Textout<Font>&	operator<<(unsigned long);
			Textout<Font>&	operator<<(short);
			Textout<Font>&	operator<<(unsigned short);
			Textout<Font>&	operator<<(float);
			Textout<Font>&	operator<<(double);
			template <typename T>
				Textout<Font>&	operator<<(const M::TVec2<T>&v)	{return *this << '('<<v.x << ", "<<v.y<<')';}
			template <typename T>
				Textout<Font>&	operator<<(const M::TVec3<T>&v)	{return *this << '('<<v.x << ", "<<v.y<< ", "<<v.z<<')';}
			template <typename T>
				Textout<Font>&	operator<<(T&v)	
				{
					using StringConversion::ToString;
					return *this << ToString(v);
				}

			void			BeginNewLine();		//!< Non-virtual overload of NewLine()
			void			Print(const char*str, size_t len);					//!< Prints a string segment with a predefined length @param str Pointer to an array of characters containing at least @b len characters @param len Number of characters to print
			void			Print(const char*str);
			void			Print(const String&str);
			void			Print(const StringRef&str);
			void			Print(const UTF32StringRef&str);
			/**
			This method prints the string tag-sensitive. Assuming the tag-variable is '%', then the following tags are possible:
			'%push':		Pushes the current output status to the local stack
			'%pop':			Restores the current output status from the local stack
			'%rrggbbaa':	Hex representation of the color to use from this point on
			<br>
			A string "%0000ffffsomething %push%ff0000ffvery %popstupid" would color 'something' and 'stupid' blue but the 'very' in the middle red.
			*/
			void			PrintTagged(const char*str, char tag);
			void			PrintTagged(const String&str, char tag);
			void			PrintLn(const char*str);
			void			PrintLn(const String&str);
			void			PrintLn(const StringRef&str);
			void			PrintTaggedLine(const char*str, char tag);
			void			PrintTaggedLine(const String&str, char tag);
		};

}
 
#include "textout.tpl.h"
#endif
