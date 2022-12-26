#ifndef engine_gl_extensionsH
#define engine_gl_extensionsH

#include <global_root.h>

#include "../../gl/gl.h"
#include "../../gl/glext.h"
#include <general/undef.h>
#include "light.h"

#undef glBlendFuncSeparate
#undef glGenBuffers
#undef glBindBuffer
#undef glBufferData
#undef glDeleteBuffers

#ifndef __CUDACC__

	#include "../../gl/gl_enhancement.h"
	#include <stdio.h>
	#include "../renderer/renderer.h"
	#include <container/string_list.h>
	#include <string/string_buffer.h>
	//#include "../../container/string_list.h"
	#include <string/tokenizer.h>
	#include <math/resolution.h>
	#include <io/file_stream.h>

	#if SYSTEM==WINDOWS
		#include "../../gl/wglext.h"
	#elif SYSTEM==UNIX
		#ifndef Bool
			typedef int	Bool;
		#endif
		#ifndef Status
			typedef int Status;
		#endif		
		#include "../../gl/glx.h"
		#include "../../gl/glxext.h"
	#endif


		


	namespace Engine
	{

		#define EXT_MULTITEXTURE_BIT			0x0001
		#define EXT_REGISTER_COMBINERS_BIT		0x0002
		#define EXT_VERTEX_PROGRAM_BIT			0x0004
		#define EXT_VERTEX_BUFFER_OBJECT_BIT	0x0010
		#define EXT_COMPILED_ARRAYS_BIT			0x0020
		#define EXT_SHADER_OBJECTS_BIT			0x0040
		#define EXT_WIN_CONTROL_BIT				0x0080
		#define EXT_PBUFFER_BIT					0x0100
		#define EXT_RENDER_TEXTURE_BIT			0x0200
		#define EXT_OCCLUSION_QUERY_BIT			0x0400
		#define EXT_MAKE_CURRENT_READ_BIT		0x0800
		#define EXT_WINDOW_POS_BIT				0x1000
		#define EXT_FRAME_BUFFER_OBJECT_BIT		0x2000
		#define EXT_BLEND_FUNC_SEPARATE_BIT		0x4000

		

		CONSTRUCT_ENUMERATION4(DepthStorage,Texture,PrivateBuffer,SharedBuffer,NoDepthStencil);

		/*


		#if SYSTEM==WINDOWS
		#define GET_GL_PROC_ADDRESS(name) wglGetProcAddress(name)
		#elif defined(GLX_ARB_get_proc_address)
		#define GET_GL_PROC_ADDRESS(name) glXGetProcAddressARB((GLubyte*) name)
		#endif
		*/

		class ExtCont
		{
		public:
				Array<String,Adopt>	field;

				void					divide(String line);
				bool					lookup(const String&element);
				String					listExtensions(const String&plank="", count_t maxCharsPerLine=0);
				String					listColumns(const String&plank="", BYTE columns = 4);
		};

		
		/**
			@brief Individual color target configuration
		*/
		struct TFBOColorTarget
		{
			GLuint			textureHandle;		//!< Render target texture handle
			GLenum			textureFormat;		//!< Internal texture format

							TFBOColorTarget():textureHandle(0),textureFormat(0)
							{}
		};

		struct TFBODepthTarget
		{
			GLuint			handle;					//!< Handle of the texture or depth buffer, depending on storageType
			DepthStorage	storageType;			//!< Storage type for depth information


							TFBODepthTarget():handle(0)
							{}
		};

		/**
			@brief Multi-target FBO

			Up to four color targets may be addressed simultaneously
		*/
		struct TFBOConfig
		{
			TFBOColorTarget	colorTarget[4];	//!< Color texture references. References beyond numColorTargets are undefined
			TFBODepthTarget	depthTarget;		//!< Points to a depth buffer or depth texture, depending on depthIsTexture
			Resolution		resolution;			//!< Resolution in pixels of all render targets and the depth buffer/texture
			BYTE			numColorTargets;	//!< Number of targets in colorTarget to actually use
		
		
							TFBOConfig():numColorTargets(0)
							{}
		};
		
		struct TFrameBuffer:public TFBOConfig
		{
			GLuint		frameBuffer;	//!< General rendering target (frame buffer)
						
						TFrameBuffer():frameBuffer(0)
						{}
						TFrameBuffer(GLuint buffer, const TFBOConfig&attachment):frameBuffer(buffer),TFBOConfig(attachment)
						{}
		};


		#ifdef GL_ARB_shader_objects
		

		namespace GLShader
		{

			class	Instance;
		
			class	Variable
			{
			private:
				Instance				*instance;
				GLint					handle;
				static	bool			lockUninstalled,	//less strict
										assertIsInstalled;//strict
				String					name;
				
				friend class GLShader::Instance;

				//static bool				wasInstalled;
				//bool					_PrepareUpdate();
				//bool					_FinishUpdate();
				
				/**/					Variable(Instance*,GLint,const String&name);
			public:
				/**/					Variable();

				bool					setf(float value);
				inline bool				Set(float value)		{return setf(value);}
				bool					set4f(float x, float y, float z, float w);
				inline bool				Set(float x, float y, float z, float w)		{return set4f(x,y,z,w);}
				bool					Set(float x, float y, float z);
				bool					Set(float x, float y);
				bool					Set(const M::TVec2<>&);
				bool					Set(const M::TVec3<>&);
				bool					Set(const M::TVec4<>&);
				bool					Set(const M::TMatrix3<>&);
				bool					Set(const M::TMatrix4<>&);
				bool					seti(int value);
				inline bool				SetInt(int value)		{return seti(value);}
				bool					set2i(int x, int y);
				inline bool				SetInt(int x, int y)		{return set2i(x,y);}
				bool					SetBool(bool value);
				
				static	void			LockUninstalledShaderVariables()
										{
											lockUninstalled = true;
										}
				static	void			AssertShaderIsInstalled()
										{
											assertIsInstalled = true;
										}

				GLint					GetHandle()	const	{return handle;}
				
				bool					Exists()	const {return handle != -1;}
				operator				bool()	const {return handle != -1;}
			};

			bool		ExtractFileContent(const String&filename, String&target, StringBuffer&logOut);
			bool		ExtractFileContent(const String&filename, String&target);


			class Composition
			{
			public:
				String					sharedSource,
										vertexSource,
										fragmentSource, 
										geometrySource;

				void					adoptData(Composition&other);
				void					Clear();
				bool					IsEmpty()	const	{return sharedSource.IsEmpty() && vertexSource.IsEmpty() && fragmentSource.IsEmpty() && geometrySource.IsEmpty();}
				bool					IsNotEmpty()const	{return !IsEmpty();}
				Composition&			Load(const String&objectSource);
				bool					LoadFromFiles(const String&sharedFile, const String&vertexFile, const String&fragmentFile);
				bool					LoadFromFiles(const String&sharedFile, const String&vertexFile, const String&fragmentFile, const String&geometryFile);
				bool					LoadFromFile(const String&objectFile);

				void					Append(const Composition&other);	//!< Appends the specified other composition to the end of the local one
				void					Prefix(const Composition&other);	//!< Adds the specified other composition before the local one
			};


			class Template;
			class Instance;

		
				/**
					@brief GLSL Shader precompiler
			
			
					The shader template is a powerful structure to automatically assemble shaders depending on the current opengl light/fog status and user-definable variables.
					Code formatting:<br />
					Similar to the GLShader::Instance class shader code may be provided as individual source blocks, or one block with source segments separated by [vertex], [fragment], and [geometry] respectivly.
					In addition to the normal shader functionality precompiler directives become available in the form #directive much like C/C++ precompiler lines. Supported directives are:<ul>
					<li>#if [condition]<br />
						inserts the code specified in the following lines if the specified condition resolves to true. [condition] supports standard operators + - * / && || ! == != < > <= >= and parenthesis to group expressions.
						apart from said operators integer/boolean constants may be used and/or global integer variables. The following variables are predefined and automatically filled if appropriate:<ul>
						<li>fog: this variable will be filled with 1 if fog is currently enabled and 0 if not</li>
						<li>omni: this variable is valid only if currently inside a light-loop block and set to 1 if the currently processed light is of type omni</li>
						<li>direct: this variable is valid only if currently inside a light-loop block and set to 1 if the currently processed light is of type directional</li>
						<li>spot: this variable is valid only if currently inside a light-loop block and set to 1 if the currently processed light is of type spot</li>
						</ul>
						<br />Other variables may be used as required but must be specified by the client application or will always resolve to 0<br />
						#if blocks must be finalized with an #elif, #else or #endif directive</li>
					<li>#elif [condition]<br />
						similar to #if except only allowed as a followup of #if/#elif<br />
						#elif blocks must be finalized with an #elif, #else or #endif directive</li>
					<li>#else<br />
						specifies code to be included if preceeding conditions evaluate to false<br />
						#else blocks must be finalized with an #endif directive</li>
					<li>#endif<br />
						terminates an #if, #elif, or #else block and returns to the next upper level</li>
					<li>#light \<light\><br />
						Initiates a light loop which will iterate through all enabled lights. The code specified within is repeated once per enabled light.
						Light loops may be recursive. The specified name between \< and \> is initiated as a constant and may be used in the inner code segments.<br />
						Example:<br />
						#light \<i\><br />
							gl_FragColor += shade(\<i&\>,1.0,0.5,0.0);<br />
						#endlight<br />
						Light loops must be terminated using the #endlight directive</li>
					<li>#version [index]<br />
						Passed on to the underlying shader. Not interpreted by the template structure</li>
					<li>#extension [extension] : enable<br />
						Passed on to the underlying shader. Not interpreted by the template structure</li>
					</ul><br /><br />
					<b>Predefined functions</b><br />
					The following functions will be included into the build if called referenced the source. Their names should not be used for other purposes.
					The final parameter 'shadow' should be 0 if the light source is in shadow, and 1 if it is fully lit, or any value inbetween effectively toning diffuse and specular light.
					<ul>
						<li>vec4 omniLight(int index,vec3 position,vec3 normal,vec3 reflected, float shadow);<br />
							Calculates the light color/intensity emitted by the light source with the specified index, assuming it's an omni-directional lightsource</li>
						<li>vec4 directLight(int index,vec3 position,vec3 normal,vec3 reflected, float shadow);<br />
							Calculates the light color/intensity emitted by the light source with the specified index, assuming it's a directional lightsource</li>
						<li>vec4 spotLight(int index,vec3 position,vec3 normal,vec3 reflected, float shadow);<br />
							Calculates the light color/intensity emitted by the light source with the specified index, assuming it's a spot lightsource</li>
						<li>vec4 shade(vec3 position, vec3 normal, vec3 reflected);
							Calculates the summed color/intensity of all enabled lightsources</li>
						<li>vec4 customShade(vec3 position, vec3 normal, vec3 reflected, vec3 ambient, vec3 diffuse, vec3 specular);
							Calculates the summed color/intensity given a custom material </li></ul>
					
					<br />
					The specified parameters are:<ul>
						<li>index: Index of the light in question 0...n</li>
						<li>position: Vertex/fragment position in eye-space</li>
						<li>normal: Normalized vertex/fragment normal in eye-space</li>
						<li>reflected: Normalized reflected viewing direction in eye-space used to calculate specular lighting</li>
					</ul><br />
					Included aspects are the relations between normals and the respective light direction, spot exponent and attenuation if applicable
				*/
				/*
					#light <i>\n\
						#if omni\n\
							gl_FragColor += omniLight(<i>,frag_coord,normal,reflected,fragmentShadow<i>(frag_coord));\n\
						#elif direct\n\
							gl_FragColor += directLight(<i>,frag_coord,normal,reflected,fragmentShadow<i>(frag_coord));\n\
						#elif spot\n\
							gl_FragColor += spotLight(<i>,frag_coord,normal,reflected,fragmentShadow<i>(frag_coord));\n\
						#endif\n\
					#endlight
				*/
			class Template
			{
			public:
				class VariableMap;
				class UserConfiguration;
				class RenderConfiguration;


				enum variable_index_t
				{
					FogVariableIndex = 1,
					LightingVariableIndex = 2
				};

				class Expression;
				typedef std::shared_ptr<Expression>	PExpression;
		
				class Expression:public IToString
				{
				public:
					unsigned					level;
		
					virtual						~Expression()	{}
					virtual	int					Evaluate(const UserConfiguration&, Light::Type)=0;			//!< Evaluate the value of the local expression
					virtual	bool				AdoptLeft(const PExpression&)		{return false;}	//!< Attempts to adopt the next expression to the left as a sub expression @return true if the expression has been adopted, false otherwise
					virtual	bool				AdoptRight(const PExpression&)	{return false;}	//!< Attempts to adopt the next expression to the right as a sub expression @return true if the expression has been adopted, false otherwise
					virtual	bool				Validate()	{return true;}					//!< Check the integrity of the local expression and all its children
				};
		
		
				/**
				@brief Expression that evaluates two sub expressions and combines their respective values

				Sub expressions are deleted automatically on object destruction unless NULL.
				Both a left and a right expression are adopted.
				*/
				class BinaryExpression:public Expression
				{
				public:
					PExpression				first,		//!< First sub expression
											second;	//!< Second sub expression

					virtual	bool			AdoptLeft(const PExpression&exp)	override
											{
												first = exp;
												return true;
											}
					virtual	bool			AdoptRight(const PExpression&exp)	override
											{
												second = exp;
												return true;
											}
					virtual	String			ConvertToString()	const	override
											{
												return "<"+(first?first->ConvertToString():String("NULL"))+", "+(second?second->ConvertToString():String("NULL"))+">";
											}
					virtual	bool			Validate()	override	{return first && second && first->Validate() && second->Validate();}
				};

				#undef DEFINE_GL_TEMPLATE_OPERATOR_EXPRESSION
				#define DEFINE_GL_TEMPLATE_OPERATOR_EXPRESSION(_CLASS_NAME_,_OPERATOR_)\
					class _CLASS_NAME_:public BinaryExpression\
					{\
					public:\
						virtual	int			Evaluate(const UserConfiguration&status, Light::Type type)	override\
											{\
												return first->Evaluate(status,type) _OPERATOR_ second->Evaluate(status,type);\
											}\
					};

				DEFINE_GL_TEMPLATE_OPERATOR_EXPRESSION(ProductExpression,*);
				DEFINE_GL_TEMPLATE_OPERATOR_EXPRESSION(DifferenceExpression,-);
				DEFINE_GL_TEMPLATE_OPERATOR_EXPRESSION(SumExpression,+);
				DEFINE_GL_TEMPLATE_OPERATOR_EXPRESSION(QuotientExpression,/);
				DEFINE_GL_TEMPLATE_OPERATOR_EXPRESSION(GreaterExpression,>);
				DEFINE_GL_TEMPLATE_OPERATOR_EXPRESSION(GreaterOrEqualExpression,>=);
				DEFINE_GL_TEMPLATE_OPERATOR_EXPRESSION(LessExpression,<);
				DEFINE_GL_TEMPLATE_OPERATOR_EXPRESSION(LessOrEqualExpression,<=);
				DEFINE_GL_TEMPLATE_OPERATOR_EXPRESSION(EqualExpression,==);
				DEFINE_GL_TEMPLATE_OPERATOR_EXPRESSION(NotEqualExpression,!=);
				DEFINE_GL_TEMPLATE_OPERATOR_EXPRESSION(AndExpression,&&);
				DEFINE_GL_TEMPLATE_OPERATOR_EXPRESSION(OrExpression,||);

		
				/**
				@brief Expression that negates (not) the returned value of the respective inner expression

				Adopts one expression the right
				*/
				class NegationExpression:public Expression
				{
				public:
					PExpression				inner;
				
					virtual	int				Evaluate(const UserConfiguration&status, Light::Type type)	override
											{
												ASSERT_NOT_NULL__(inner);
												return !inner->Evaluate(status,type);
											}
					virtual	bool			AdoptRight(const PExpression&exp)	override
											{
												inner = exp;
												return true;
											}
					virtual	bool			Validate()	override
											{
												return inner && inner->Validate();
											}
					virtual	String			ConvertToString()	const	override
											{
												return "!<"+(inner?inner->ConvertToString():String("NULL"))+">";
											}
				};
		
				/**
				@brief Expression that simply returns a constant value
				*/
				class ConstantExpression:public Expression
				{
				public:
					int						value=0;

					/**/					ConstantExpression()	{}
					/**/					ConstantExpression(int val):value(val)	{}
				
					virtual	int				Evaluate(const UserConfiguration&, Light::Type)	override	{return value;}
					virtual	String			ConvertToString()	const	override
											{
												return "Constant<"+String(value)+">";
											}
				};
		
				/**
				@brief Expression that returns a variable value depending on the current template status
				*/
				class VariableExpression:public Expression
				{
				public:
					index_t					index;	//!< Variable index (0=undefined)
					String					name;	//!< Variable name
				
				
					/**/					VariableExpression():index(0)	{}
					/**/					VariableExpression(const String&name, index_t index):name(name),index(index)	{}
					virtual	int				Evaluate(const UserConfiguration&values, Light::Type)	override;
					virtual	String			ConvertToString()	const	override
											{
												return "Variable<"+name+"@"+String(index)+">";
											}
				};
		
				/**
				@brief Expression that returns true if the specified light type matches the stored one
				*/
				class LightTypeExpression:public Expression
				{
				public:
					Light::Type				type;	//!< Stored light type
								
					/**/					LightTypeExpression():type(Light::None){}
					/**/					LightTypeExpression(Light::Type t):type(t){}
					
					virtual	int				Evaluate(const UserConfiguration&, Light::Type lightType)	override
											{
												return lightType == type;
											}
					virtual	String			ConvertToString()	const	override
											{
												return "LightType<"+String(type)+">";
											}
								
				};
		
				class Block;
				class RootBlock;
		
				class LightShadowAttachment
				{
				public:
					struct SamplerAssignment
					{
						String				samplerName;
						GLint				samplerLevel;

						void				swap(SamplerAssignment&other)
											{
												swp(samplerLevel,other.samplerLevel);
												samplerName.swap(other.samplerName);
											}

					};
					String					sharedAttachment,
											vertexShaderAttachment,
											fragmentShadowCode;
					Ctr::PreAllocVector<SamplerAssignment,0,Swap>
											samplerAssignments;

					void					swap(LightShadowAttachment&other)
											{
												sharedAttachment.swap(other.sharedAttachment);
												vertexShaderAttachment.swap(other.vertexShaderAttachment);
												fragmentShadowCode.swap(other.fragmentShadowCode);
												samplerAssignments.swap(other.samplerAssignments);
											}
					bool					IsEmpty()	const	{return fragmentShadowCode.IsEmpty();}

					void					PredefineSampler(const char*samplerName, GLint samplerLevel)
											{
												SamplerAssignment&a = samplerAssignments.Append();
												a.samplerName = samplerName;
												a.samplerLevel = samplerLevel;
											}
				};

				static	Ctr::PreAllocVector<LightShadowAttachment,0,Swap>		shadowAttachments;


				/**
				@brief Single code line of a shader block
				*/
				class Line
				{
				public:
					Array<String,Adopt>		segments;		//!< String line segments. Between each two line segments is one light constant. Typically this array only holds one element
				
					void					adoptData(Line&other)
											{
												segments.adoptData(other.segments);
											}
				};
		

				class Block;
				typedef std::shared_ptr<Block>	PBlock;
		
				/**
				@brief A block of shader code lines
				*/
				class Block
				{
				public:
					enum Type		//! Block type
					{
						Any,				//!< This block is always appended
						Conditional,		//!< This block is appended based on a condition
						ElseConditional,	//!< This block is appended based on a previous and a local condition
						Else,				//!< This block is appended based on a previous condition
						LightLoop			//!< This block iterates through enabled lightsources
					};
			
					Type					type;					//!< Local block type
					PExpression				condition;				//!< Condition expression. May be NULL. The condition expression is automatically deleted on object destruction.
					String					lightLoopConstant;	//!< Constant expression used to address the current light loop iteration (typically something like "<i>"). Unless overriden, this expression is inherited from the respective parent block during construction.
			
					Array<Line,Adopt>		innerLines,	//!< Inner lines preceeding any conditional sub block. Lines following condition blocks are found in the @a trailingLines member of sub blocks
											trailingLines;	//!< Lines trailing the local block. They lie, in fact, outside this block on the next parent layer
					Ctr::Vector<PBlock>	children;		//!< Child blocks
					PBlock					 parent;		//!< Parent block (if any). may be NULL
				
					Block()					:type(Any),condition(NULL),parent(NULL)	{}

					void					Clear()
											{
												condition.reset();
												innerLines.free();
												trailingLines.free();
												children.clear();
												type = Any;
												lightLoopConstant = "";
											}
					void					Assemble(StringBuffer&target, const UserConfiguration&values, const RenderConfiguration&renderConfig, Light::Type type, index_t lightIndex);
					bool					UsesLighting()	const;		//!< Recursive check if lighting functions are used
					void					adoptData(Block&other)
											{
												type = other.type;
												condition = other.condition;
												lightLoopConstant.adoptData(other.lightLoopConstant);
												innerLines.adoptData(other.innerLines);
												trailingLines.adoptData(other.trailingLines);
												children.adoptData(other.children);
												parent = other.parent;

												other.type = Any;
												other.condition = NULL;
												other.parent = NULL;
											}
				};
		
				/**
				@brief Root template block
				*/
				class RootBlock
				{
				private:
					struct Token	//! Expression token identifier
					{
						enum Identifier //! Expression token identifier
						{
							String,	//!< Custom content string
							PUp,	//!< Parenthesis up '(' token
							PDown,	//!< Parenthesis down ')' token
							Plus,	//!< '+'
							Minus,	//!< '-'
							Times,	//!< '*'
							Divide,	//!< '/'
							And,	//!< '&&'
							Or,		//!< '||'
							Equals,	//!< '=='
							Differs,	//!< '!='
							Greater,	//!< '>'
							Less,		//!< '<'
							GreaterOrEqual,	//!< '>='
							LessOrEqual,	//!< '<='
							Not,			//!< '!'
							Space,			//!< Blank
						
							Mask=0x0FFF		//!< Mask to apply to a given token in order to identify its type (eg. 'token&(Token::Mask)')
						};
					};
				
					struct Level		//!< Token level
					{
						enum Identifier //!< Token level
						{
							None=0x0000,
							Zero=0x1000,
							One=0x2000,
							Two=0x3000,
							Three=0x4000,
							Four=0x5000,
							Five=0x6000,
						
							Mask=0xF000
						};
					};
				
					PExpression					_ParseCondition(const char*condition, VariableMap&map, String&error);
					PExpression					_ProcessLayer(TokenList&tokens,VariableMap&map, index_t begin, index_t end, String&error);
					static	void				_ShadowFunction(index_t level,StringBuffer&buffer);

				public:
					PBlock						root = PBlock(new Block());

					bool 						shadeInvoked,				//!< Specifies that the shade() function is called in this block
												shade2Invoked,
												customShadeInvoked,		//!< Specifies that the customShade() function is called in this block
												spotlightInvoked,			//!< Specifies that the spotLight() function is called in this block
												omnilightInvoked,			//!< Specifies that the omniLight() function is called in this block
												directlightInvoked;		//!< Specifies that the directLight() function is called in this block
				
				
					bool						Scan(const String&source, VariableMap&map, StringBuffer&logOut, index_t&lineOffset);
					String						Assemble(const RenderConfiguration&,const UserConfiguration&, bool isShared);
					void						Assemble(const RenderConfiguration&,const UserConfiguration&,StringBuffer&target, bool isShared=false);
					bool						UsesLighting()	const;		//!< Queries whether or not lighting-relevant constants will be queried during assembly. If this value is false then RenderConfiguration will not be queried

					void						Clear(){root.reset(new Block());}
				};
		
				class VariableMap
				{
				protected:
					Ctr::StringTable<index_t>	variableMap;
					Ctr::Vector<UserConfiguration*>
												attachedConfigurations;
					bool						changed;

					friend class				RootBlock;
					friend class				Instance;
					friend class				UserConfiguration;

					void						Reg(UserConfiguration*config)	{attachedConfigurations.Append(config);}
					void						Unreg(UserConfiguration*config){attachedConfigurations.FindAndErase(config);}

					index_t						Define(const String&variableName);
				public:
					VariableMap();
					virtual						~VariableMap();
					index_t						Lookup(const String&varName)	const;
					void						Clear();		//!< Clears all local variables and redefines standard variables
					void						SubmitChanges();	//!< Notifies attached user configurations of changes to the map. The method has no effect if no changes have been made
				};
		
				class Configuration;

				class ConfigurationComponent
				{
				private:
					Ctr::Vector<Configuration*>linkedConfigs;
				protected:
					friend class				Configuration;

					index_t						version;

					void						Unreg(Configuration*config);	//!< Unlinks a registered configuration
					void						Reg(Configuration*config);		//!< Links a configuration
					void						SignalHasChanged();				//!< Notifies linked components that local data has changed
				public:
					ConfigurationComponent()	:version(0)	{}
					virtual						~ConfigurationComponent();
				};

				class RenderConfiguration:public ConfigurationComponent	//! Configuration container for pipeline related settings that are relevant for shader assembly
				{
				protected:
					Ctr::PreAllocVector<Light::Type,8>	lights;		//!< Known light types
					bool						lightingEnabled,
												fogEnabled;
					friend class				UserConfiguration;
					friend class				RootBlock;
					friend class				Block;
					friend class				Instance;
					friend class				Configuration;

				public:
					RenderConfiguration();
					void						ReDetect();							//!< Updates lights and fog variable
					void						SetLights(count_t count,...);		//!< Manually sets light types. The first parameter must specify how many light sources are to be considered enabled. Subsequent parameters must provide at least @a count values of type Light::Type
					void						SetLighting(bool lighting);			//!< Globally enables or disables lighting
					void						Clear();							//!< Clears all light sources and resets configuration to default
				};
				typedef RenderConfiguration		RenderConfig;

				static RenderConfiguration		globalRenderConfig;		//!< Global configuration that keeps track of shader-relevant pipeline settings

				/**
				@brief Variable values associated with an instance of VariableMap

				These values are required for proper shader assembly
				*/
				class UserConfiguration:public ConfigurationComponent
				{
				protected:
					VariableMap					*map;		//!< Pointer to a variable name field. May be NULL
					Array<int,Primitive>		values;		//!< Variable values
				
					friend class				RootBlock;
					friend class				Block;
					friend class				Template;
					friend class				Configuration;
					friend class				VariableMap;
				
					void						Adapt();
					void						SignalMapDestruction();
				public:
					/**/						UserConfiguration();
					virtual						~UserConfiguration();
					bool						Set(index_t variable, int value);					//!< Sets the value of a specific variable by index. @param variable Index of the variable to update @param value Value to set @return true on success
					bool						SetByName(const String&variableName, int value);	//!< Sets the value of a specific variable by name. The local configuration must be linked with a variable map for this method to succeed. @param variableName Name of the variable to update @param value Value to set @return true on success
					int							GetValue(index_t variable)	const;					//!< Queries the current value of a variable by index @param variable Variable index starting at 1 @return Current variable value or 0 if the specified index is invalid
					int							GetValueByName(const String&variableName) const;	//!< Queries the current value of a variable by namex @param variableName Name of the variable to query @return Current variable value or 0 if the specified name is invalid
					void						ResetAllValues();									//!< Resets all local variables to 0.
					void						Clear();											//!< Clears the local structure (also unlinks it from any VariableMap instance)
					void						Link(VariableMap*map, bool adapt=true);				//!< Updates map link and registration. Potentially updates variable field length. Checks and aborts if @a this is NULL
					VariableMap*				GetVariableMap();									//!< Retrieves the map currently associated with this configuration, if any @return variable map or NULL if no such exists
					const VariableMap*			GetVariableMap() const;								//!< Retrieves the map currently associated with this configuration, if any @return variable map or NULL if no such exists

					void						UpdateRenderVariables(const RenderConfiguration&);	//!< Updates the local rendering-related variables based on the specified render configuration
					void						UpdateRenderVariables();							//!< Identical to the above but queries @b globalRenderConfig instead
				};

				typedef UserConfiguration		UserConfig;

				class Configuration:public Array<int,Primitive>
				{
				protected:
					UserConfiguration			*userConfig;
					RenderConfiguration			*renderConfig;

					index_t						userConfigVersion,
												renderConfigVersion;

					bool						valid,				//!< All configuration components are linked
												registered,			//!< The configurations know of this instance. Initially set to false if components are passed as constructor parameters and @a registerRow is false
												structureChanged;	//!< Indicates that at least one configuration pointer has changed since the last update() call

					friend class				Template;
				public:
					Configuration()				:userConfig(NULL),renderConfig(NULL),valid(false),registered(false),structureChanged(false)	{}
					/**/						Configuration(RenderConfiguration&newRenderConfig,UserConfiguration&newUserConfig,bool registerNow=true)
												:userConfig(&newUserConfig),renderConfig(&newRenderConfig),valid(true),registered(registerNow),
												structureChanged(false)
												{
													if (registerNow)
													{
														userConfig->Reg(this);
														renderConfig->Reg(this);
													}
												}
												
					virtual						~Configuration();
					bool						Update();										//!< Updates the local key if needed
					bool						RequiresUpdate()	const;						//!< Checks if the local structure or any of the linked components have changed

					void						Unlink();		//!< Unlinks linked configurations
					void						Link(RenderConfiguration&renderConfig, UserConfiguration&userConfig);
					void						Link(RenderConfiguration&renderConfig);
					void						Link(UserConfiguration&userConfig);

					void						UpdateRenderVariables()
												{
													if (valid)
														userConfig->UpdateRenderVariables(*renderConfig);	//should trigger the whole changed cascade automatically
												}
					void						SignalComponentDestroyed(ConfigurationComponent*component);		//!< Notifies this structure that a linked component was destroyed
					void						SignalComponentHasChanged();		//!< Notifies this structure that a linked component has changed
					inline	UserConfiguration*	GetUserConfig()			{return userConfig;}
					inline	const UserConfig*	GetUserConfig() const		{return userConfig;}
					inline	RenderConfiguration*GetRenderConfig()			{return renderConfig;}
					inline	const RenderConfig*	GetRenderConfig() const	{return renderConfig;}
				};

				typedef Configuration			Config;
		
			protected:
				RootBlock						sharedTemplate,
												vertexTemplate,
												geometryTemplate,
												fragmentTemplate;
				GLenum 							geometryType,
												outputType;
				unsigned						maxVertices;
				bool							isLoaded,
												usesLighting;			//!< True if any shading-related functionality is used by the shader
				VariableMap						localMap,				//!< Locally stored variable name map. This would be the standard variable map
												*currentMap;			//!< Actually used variable name map. Usually maps to @a localMap. Using the same variable map on different template instances, though, can be useful when working with just one huge user config
				UserConfiguration				localUserConfig;		//!< Locally stored user variable config. Contains all user variable values that may be referenced during shader assembly
				Configuration					localConfig,
												*currentConfig;
				StringBuffer					log;
				Ctr::GenericHashContainer<Array<int,Primitive>,GLShader::Instance>	container;	//!< All assembled shaders mapped to their respective keys
				

				struct TUniformInit	//! Uniform pInherit structure
				{
					enum Type		//! PInherit type
					{
						Int,		//!< Integer type
						Float,		//!< Float type
						Float3		//!< 3d float type
					};
					
					String						name;	//!< Name of the variable to predefine
					union
					{
						int						ival;	//!< Integer value
						float					fval;	//!< Float value
						M::TVec3<>					f3val;
					};
					Type						type;	//!< Contained type
				};
				
				Ctr::PreAllocVector<TUniformInit,4>		uniformInit;	//!< Predefined variables
				
			public:
		
				Template()						:geometryType(GL_TRIANGLES),outputType(GL_TRIANGLE_STRIP),maxVertices(12),isLoaded(false),usesLighting(false),currentMap(&localMap),localConfig(globalRenderConfig,localUserConfig,false),currentConfig(&localConfig)	{}
				void							Clear();
				bool							Load(const Composition&composition, GLenum geometryType=GL_TRIANGLES, GLenum outputType=GL_TRIANGLE_STRIP, unsigned maxVertices=12);
				bool							LoadComposition(const String&objectSource, GLenum geometryType=GL_TRIANGLES, GLenum outputType=GL_TRIANGLE_STRIP, unsigned maxVertices=12);
				void							LoadRequired(const Composition&composition, GLenum geometryType=GL_TRIANGLES, GLenum outputType=GL_TRIANGLE_STRIP, unsigned maxVertices=12);
				void							LoadRequiredComposition(const String&objectSource, GLenum geometryType=GL_TRIANGLES, GLenum outputType=GL_TRIANGLE_STRIP, unsigned maxVertices=12);
				void							LoadFromFile(const String&filename, GLenum geometryType=GL_TRIANGLES, GLenum outputType=GL_TRIANGLE_STRIP, unsigned maxVertices=12);
				String							Report();
				bool							IsLoaded()	const;

				inline	VariableMap*			GetVariableMap()			{return currentMap;}
				inline	const VariableMap*		GetVariableMap()	const	{return currentMap;}
				inline	UserConfiguration*		GetUserConfig()				{return currentConfig->GetUserConfig();}
				inline	const UserConfig*		GetUserConfig()		const	{return currentConfig->GetUserConfig();}
				inline	RenderConfiguration*	GetRenderConfig()			{return currentConfig->GetRenderConfig();}
				inline	const RenderConfig*		GetRenderConfig()	const	{return currentConfig->GetRenderConfig();}
						
				void							SetVariableMap(VariableMap*map, bool adjustUserConfig=true);	//!< Changes the variable map to use. Should not be changed before loading data into the local object @param 
				void							SetConfig(UserConfiguration*config, bool adjust=true);	//!< Changes the user configuration storage object to the specified one. @param config New configuration storage. May be NULL reverting the storage object to default @param adjust Update the length of the storage's variable field to the local map
				void							SetConfig(RenderConfiguration*config);
				void							SetConfig(Configuration*config);

				void							ResetConfig()		{SetConfig((Configuration*)NULL);}
				void							ResetUserConfig()	{SetConfig(NULL,true);}
				void							ResetRenderConfig()	{SetConfig((RenderConfiguration*)NULL);}
				void							ResetVariableMap()	{SetVariableMap(NULL);}
				
				inline	index_t					FindVariable(const String&variableName) const		{return currentMap->Lookup(variableName);}	//! Locates a template variable. See VariableMap::locate() for details
				inline	bool					SetVariable(index_t variable, int value)			{return currentConfig->valid && currentConfig->GetUserConfig()->Set(variable,value);}	//!< Updates the value of a variable. See UserConfiguration::Set() for details
				/**
				@brief Constructs a shader instance based on the current render and user configurations
							
				NULL-pointer sensitive

				@param autoUpdateRenderVariables Auto-update renderpipeline-derived user variables (e.g. 'lighting'). Does not automatically redetect such
				@param[out] isNew  Filled with true if a previously undefined shader instance was constructed. Filled with false if an existing matching shader instance was found. May be NULL
				@return Pointer to a new or existing shader instance, or NULL if compilation failed. Use report() to determine a user readable cause if compilation failed for some reason
				*/
				GLShader::Instance*				BuildShader(bool autoUpdateRenderVariables=true, bool*isNew=NULL);

				/**
				@brief Assembles a full shader as it could be loaded by an external shader instance
				@param autoUpdateRenderVariables Auto-update renderpipeline-derived user variables (e.g. 'lighting'). Does not automatically redetect such
				@return Shader composition featuring all four sections 'vertex', 'fragment', 'geometry', and 'shared'
				*/
				String							AssembleSource(bool autoUpdateRenderVariables=true);
				
				
				Template&						PredefineUniformInt(const String&name, int value);	//!< Presets the specified uniform variable to the specified value on construction
				Template&						PredefineUniformFloat(const String&name, float value);	//!< Presets the specified uniform variable to the specified value on construction
				Template&						PredefineUniformVec(const String&name, const M::TVec3<>& value);	//!< Presets the specified uniform variable to the specified value on construction
				static String*					FindShaderIncludable(const String&filename);	//!< Attempts to locate or load a block template described by the specified filename. @return Existing or loaded block or NULL if loading failed
				static void						DefineShaderIncludable(const String&filename, const String&blockCode);	//!< Defines a new or overwrites an existing includable block template
			};

	


			/**
			@brief Compiled GLSL instance

			Contrary to templtes, instances have no switches and hold only ever one shader to install.
			*/
			class	Instance
			{
			private:
				GLhandleARB						programHandle,
												vertexShader,
												fragmentShader,
												geometryShader;
				StringBuffer					log;
				#ifdef INSTALLED_SHADER_INSTANCE
					static const Instance			*installedInstance;
				#endif
				static bool						warnOnError;
				
				class Initializer
				{
				public:
					String						variableName;
					int							intValue;
					index_t						start,length;
					void						swap(Initializer&other)
					{
						variableName.swap(other.variableName);
						std::swap(intValue,other.intValue);
						std::swap(start,other.start);
						std::swap(length,other.length);
					}
				};

				friend class Template;

				bool							_ExtractFileContent(const String&filename, String&target);
				GLhandleARB						_LoadShader(const String&source, GLenum programType);
				static void						_ParseUniformVariableInitializers(String&source,Ctr::BasicVector<Initializer,Strategy::Swap>& initializers);

				/**/							Instance(const Instance&other){}
				void							operator=(const Instance&other){}
				void							_ResetVariables();
			public:
				Composition						composition;		//!< Shader source code

				static bool						forgetOnDestruct;	//!< If set true the underlying GL objects are not deleted if the destructor of shader objects is invoked
		
				/**/							Instance();
				virtual							~Instance();
				void							adoptData(Instance&other);
				GLhandleARB						GetHandle()	const	{return programHandle;}
				void							Clear();
				bool							Load(const Composition&composition, GLenum geometryTpe=GL_TRIANGLES, GLenum outputTpe=GL_TRIANGLE_STRIP, unsigned maxVertices=12);
				bool							LoadComposition(const String&objectSource, GLenum geometryType=GL_TRIANGLES, GLenum outputType=GL_TRIANGLE_STRIP, unsigned maxVertices=12);
				void							LoadRequired(const Composition&composition, GLenum geometryType=GL_TRIANGLES, GLenum outputType=GL_TRIANGLE_STRIP, unsigned maxVertices=12);
				void							LoadRequiredComposition(const String&objectSource, GLenum geometryType=GL_TRIANGLES, GLenum outputType=GL_TRIANGLE_STRIP, unsigned maxVertices=12);
				void							LoadFromFile(const String&filename, GLenum geometryType=GL_TRIANGLES, GLenum outputType=GL_TRIANGLE_STRIP, unsigned maxVertices=12);
				String							Report();

				static void						SuppressFindVariableFailureWarning();
				static void						SuppressWarnings();
				static void						EnableErrorWarning();
				static void						DisableErrorWarning();
				Variable						FindVariable(const String&name, bool warnOnFail=true);
				bool							GetContent(GLint name, float*outField);
				bool							IsLoaded()	const;
				bool							IsNotEmpty()const	{return IsLoaded();}
				bool							IsEmpty()	const	{return !IsLoaded();}			//!< Not IsLoaded()
				bool							IsInstalled()	const;
				static bool						AnyIsInstalled();
				bool							Validate();
				bool							Install();				//!< Installs this shader. Installation fails if the local shader instance is NULL or not loaded
				bool							Install()		const;	//!< const shader installation. Does not write to log
				bool							PermissiveInstall();		//!< Installs the local shader, replacing any currently installed shader. Any installed shader will be uninstalled if the local shader instance is NULL or not loaded
				bool							PermissiveInstall()	const;	//!< const variant. Does not write to log
				void							Uninstall()		const;
				static void						PermissiveUninstall();
			};
		

		
			//typedef Template::Instance	Template;
		
			class Installation	//! Automatic localized shader installation. The class prevents double installation or deinstallation and automatically installs/uninstalls on construction/destruction.
			{
			private:
				Instance						*objectReference;
				Template						*templateReference;
				bool							installed;
				Installation&					operator=(const Installation&)	{return *this;}
				/**/							Installation()	{}
			public:
				/**/							Installation(Instance&object):objectReference(&object),templateReference(NULL),installed(false) {Install();}
				/**/							Installation(Template&templ):objectReference(NULL),templateReference(&templ),installed(false) {Install();}

				/**/							~Installation(){Uninstall();}
				void							Install()	//!< Installs the shader object if it hasn't been installed
												{
													if (!installed)
													{
														if (templateReference)
														{
															//template_reference->status()->redetect();
															objectReference = templateReference->BuildShader();
															if (!objectReference)
																FATAL__(templateReference->Report());
														}
														ASSERT1__(objectReference->Install(),objectReference->Report());
														installed = true;
													}
												}
				void							Uninstall()	//!< Uninstalls the shader object if it is installed
												{
													if (installed)
													{
														objectReference->Uninstall();
														installed = false;
													}
												}
				operator bool()					{Install(); return true;}	//!< Installs if not already done
			};
		
		

			#define glsl(shader)				if (Engine::GLShader::Installation __shader_installation__ = shader)
		
		}
		
		#endif

		class Extension
		{
		private:
			String								group;
			#if SYSTEM==WINDOWS
				HDC								hDC;
			#elif SYSTEM==UNIX
				::Display						*display;
				int								screen;
			#endif
			static GLuint						AllocateDepthBuffer(const Resolution&res);
			static void 						ReleaseSharedDepthBuffer(GLuint);


		public:
			struct DepthBuffer
			{
				GLuint							handle;
				count_t							referenceCount;
				Resolution						resolution;
			};
			CONSTRUCT_ENUMERATION3(VRAMQueryMethod,None, NVIDIA,ATI);
			typedef Ctr::HashTable<Resolution,index_t>	ResolutionTable;
			

			static ResolutionTable				depthBufferTable;
			static Ctr::PreAllocVector<DepthBuffer,0>		depthBufferList;
			static Ctr::IndexTable<index_t>			depthBufferMap;

			GLint								maxTextureLayers,
												maxTexcoordLayers,
												maxLights,
												maxCubeTextureSize,
												maxRegisterCombiners;
			GLfloat								maxTextureMaxAnisotropy;
			ExtCont								glExt,
												sysglExt;
			//StringBuffer						compileLog;
			//size_t							initialTotal;

			bool								IsAvailable(const String&extension);
			template <class C>
				static void						Read(C&target, const String&name);
			template <class C>
				static void						ReadSilent(C&target, const String&name);

			bool								Init(GLuint);
			String								QueryRenderState();
				
			/**
			@brief Creates a new frame buffer object
					
			@param width Width of the frame buffer object in pixels
			@param height Height of the frame buffer object in pixels
			@param depthStorage Type of depth storage to use for depth components
			@param numColorTargets Number of color targets
			@param format Pointer to a field of internal formats to use for the texture targets
			*/
			static TFrameBuffer					CreateFrameBuffer(const Resolution&res, DepthStorage depthStorage, BYTE numColorTargets, const GLenum*format);
			/**
			@brief Tests the currently bound frame buffer for frame-buffer-complete status
			A fatal exception is raised if the frame buffer is not frame-buffer-complete.
			*/
			static bool							TestCurrentFrameBuffer();

			/**
			@brief Frees all associated data and resets all internal variables
					
			The buffer must not currently be bound.
			@param buffer Buffer to free
			*/
			static void							DestroyFrameBuffer(const TFrameBuffer&buffer);
		
			/**
			@brief Binds the specified framebuffer as rendering target
			@param buffer Buffer to bind
			@return true on success, false otherwise
			*/
			static bool							BindFrameBuffer(const TFrameBuffer&buffer);
			/**
			@brief Unbinds any currently bound frame buffer object.
					
			The method has no effect if no FBO is currently bound
			*/
			static void							UnbindFrameBuffer();
			/**
			@brief Tests, whether or not the specified format provides an alpha channel
			*/
			static bool							FormatHasAlpha(GLenum format);
			/**
			@brief Attempts to resize the specified buffer to match the new width and height
			*/
			static bool							ResizeFrameBuffer(TFrameBuffer&buffer, const Resolution&res);
			/**
			@brief Copies the content (color and/or depth values) from one frame buffer object to another
					
			Typically the first FBO (from) is a multisampled one, while the second (to) is non-multisampled.
			@return true on success
			*/
			static bool							CopyFrameBuffer(const TFrameBuffer&from, const TFrameBuffer&to, bool copyColor = true, bool copyDepth=true);
			static bool							CopyFrameBuffer(const TFrameBuffer&from, const TFrameBuffer&to, const Resolution&res, bool copyColor = true, bool copyDepth=true);
			#if SYSTEM==WINDOWS
				void							Initialize(HDC hDC);
			#elif SYSTEM==UNIX
				void							Initialize(::Display*display,int screen);
			#else
				void							Initialize();
			#endif
		};

		
		
		/**
			@brief Offscreen renderer to produce one time textures
		*/
		class GLTextureFactory
		{
		private:
			GLint								previousFrameBuffer,
												viewport[4];
			GLuint								frameBuffer,
												resultTexture,
												depthBuffer;	
			M::TVec4<GLfloat>						previousClearColor;
			Resolution							resolution;
			bool								alpha,mipmapping;
		
			/**/								GLTextureFactory(const GLTextureFactory&)	{}
			GLTextureFactory&					operator=(const GLTextureFactory&)	{return *this;}
		public:
			/**/								GLTextureFactory();
			/**/								GLTextureFactory(const Resolution&, bool alpha, bool mipmapping);	//!< Directly creates the local factory
			virtual								~GLTextureFactory();
		
			/**
			@brief (re)creates the local texture factory to the specified dimensions
										
			The texture factory will initiate buffers and create textures of the specified dimensions
			@return true on success
			*/
			bool								Create(const Resolution&, bool alpha, bool mipmapping);
			bool								CheckFormat(const Resolution&, bool alpha, bool mipmapping);	//!< Checks if the specified dimensions match the local ones and updates if that should not be the case			bool				checkFormat(unsigned width, unsigned height, bool alpha);	//!< Checks if the specified dimensions match the local ones and updates if that should not be the case
			void								Clear();	//!< Destroys the local texture factory
			/**
			@brief Begins rendering to the off screen source
									
			Any previously bound frame buffer is backuped
			@return true on success
			*/
			bool								Begin();
			void								Test();
			/**
			@brief Ends offscreen rendering and copies the rendered content into a texture
										
			Any previously bound frame buffer is restored
			@return Reference to a new OpenGL texture. The returned texture is not further managed. The client application is responsible for its destruction if no longer needed
			*/
			GLuint								End();
				
			void								Abort();	//!< Aborts rendering and deletes the created texture
		
			bool								IsActive()	const;	//!< Checks if the texture factory is currently bound
			bool								IsCreated()	const;	//!< True if this factory has been created
		};
		
		extern Extension glExtensions;
		
		
		
		void		glTangent3f(GLfloat x, GLfloat y, GLfloat z);
		void		glTangent3fv(const GLfloat*v);
		void		glTangentTextureLayer(GLuint layer=GL_TEXTURE1);
		


	}

#endif

namespace OpenGLExtPointer
{

#ifdef GL_ARB_multitexture

	extern PFNGLMULTITEXCOORD1FARBPROC				glMultiTexCoord1f;
	extern PFNGLMULTITEXCOORD1FVARBPROC				glMultiTexCoord1fv;
	extern PFNGLMULTITEXCOORD2FARBPROC				glMultiTexCoord2f;
	extern PFNGLMULTITEXCOORD2FVARBPROC				glMultiTexCoord2fv;
	extern PFNGLMULTITEXCOORD3FARBPROC				glMultiTexCoord3f;
	extern PFNGLMULTITEXCOORD3FVARBPROC				glMultiTexCoord3fv;
	extern PFNGLMULTITEXCOORD4FARBPROC				glMultiTexCoord4f;
	extern PFNGLMULTITEXCOORD4FVARBPROC				glMultiTexCoord4fv;
	extern PFNGLMULTITEXCOORD1DARBPROC				glMultiTexCoord1d;
	extern PFNGLMULTITEXCOORD1DVARBPROC				glMultiTexCoord1dv;
	extern PFNGLMULTITEXCOORD2DARBPROC				glMultiTexCoord2d;
	extern PFNGLMULTITEXCOORD2DVARBPROC				glMultiTexCoord2dv;
	extern PFNGLMULTITEXCOORD3DARBPROC				glMultiTexCoord3d;
	extern PFNGLMULTITEXCOORD3DVARBPROC				glMultiTexCoord3dv;
	extern PFNGLMULTITEXCOORD4DARBPROC				glMultiTexCoord4d;
	extern PFNGLMULTITEXCOORD4DVARBPROC				glMultiTexCoord4dv;
	extern PFNGLMULTITEXCOORD1IARBPROC				glMultiTexCoord1i;
	extern PFNGLMULTITEXCOORD1IVARBPROC				glMultiTexCoord1iv;
	extern PFNGLMULTITEXCOORD2IARBPROC				glMultiTexCoord2i;
	extern PFNGLMULTITEXCOORD2IVARBPROC				glMultiTexCoord2iv;
	extern PFNGLMULTITEXCOORD3IARBPROC				glMultiTexCoord3i;
	extern PFNGLMULTITEXCOORD3IVARBPROC				glMultiTexCoord3iv;
	extern PFNGLMULTITEXCOORD4IARBPROC				glMultiTexCoord4i;
	extern PFNGLMULTITEXCOORD4IVARBPROC				glMultiTexCoord4iv;
	extern PFNGLACTIVETEXTUREARBPROC				glActiveTexture;
	extern PFNGLCLIENTACTIVETEXTUREARBPROC			glClientActiveTexture;
#endif

#ifdef GL_NV_register_combiners
	extern PFNGLCOMBINERPARAMETERFVNVPROC			glCombinerParameterfv;
	extern PFNGLCOMBINERPARAMETERFNVPROC			glCombinerParameterf;
	extern PFNGLCOMBINERPARAMETERIVNVPROC			glCombinerParameteriv;
	extern PFNGLCOMBINERPARAMETERINVPROC			glCombinerParameteri;
	extern PFNGLCOMBINERINPUTNVPROC					glCombinerInput;
	extern PFNGLCOMBINEROUTPUTNVPROC				glCombinerOutput;
	extern PFNGLFINALCOMBINERINPUTNVPROC			glFinalCombinerInput;
	extern PFNGLGETCOMBINERINPUTPARAMETERFVNVPROC	glGetCombinerInputParameterfv;
	extern PFNGLGETCOMBINERINPUTPARAMETERIVNVPROC	glGetCombinerInputParameteriv;
	extern PFNGLGETCOMBINEROUTPUTPARAMETERFVNVPROC	glGetCombinerOutputParameterfv;
	extern PFNGLGETCOMBINEROUTPUTPARAMETERIVNVPROC	glGetCombinerOutputParameteriv;
	extern PFNGLGETFINALCOMBINERINPUTPARAMETERFVNVPROC	glGetFinalCombinerInputParameterfv;
	extern PFNGLGETFINALCOMBINERINPUTPARAMETERIVNVPROC	glGetFinalCombinerInputParameteriv;
#endif
#ifdef GL_ARB_vertex_program
	extern PFNGLVERTEXATTRIB2FARBPROC				glVertexAttrib2f;
	extern PFNGLVERTEXATTRIB2DARBPROC				glVertexAttrib2d;
	extern PFNGLVERTEXATTRIB3FARBPROC				glVertexAttrib3f;
	extern PFNGLVERTEXATTRIB3DARBPROC				glVertexAttrib3d;
	extern PFNGLVERTEXATTRIB4FARBPROC				glVertexAttrib4f;
	extern PFNGLVERTEXATTRIB4DARBPROC				glVertexAttrib4d;
	extern PFNGLVERTEXATTRIB2FVARBPROC				glVertexAttrib2fv;
	extern PFNGLVERTEXATTRIB2DVARBPROC				glVertexAttrib2dv;
	extern PFNGLVERTEXATTRIB3FVARBPROC				glVertexAttrib3fv;
	extern PFNGLVERTEXATTRIB3DVARBPROC				glVertexAttrib3dv;
	extern PFNGLVERTEXATTRIB4IVARBPROC				glVertexAttrib4iv;
	extern PFNGLVERTEXATTRIB4UIVARBPROC				glVertexAttrib4uiv;
	extern PFNGLVERTEXATTRIB4FVARBPROC				glVertexAttrib4fv;
	extern PFNGLVERTEXATTRIB4DVARBPROC				glVertexAttrib4dv;
	extern PFNGLVERTEXATTRIB4NIVARBPROC				glVertexAttrib4Niv;
	extern PFNGLVERTEXATTRIB4NUIVARBPROC			glVertexAttrib4Nuiv;
	extern PFNGLVERTEXATTRIBPOINTERARBPROC			glVertexAttribPointer;
	extern PFNGLENABLEVERTEXATTRIBARRAYARBPROC		glEnableVertexAttribArray;
	extern PFNGLDISABLEVERTEXATTRIBARRAYARBPROC		glDisableVertexAttribArray;
	extern PFNGLPROGRAMSTRINGARBPROC				glProgramString;
	extern PFNGLBINDPROGRAMARBPROC					glBindProgram;
	extern PFNGLDELETEPROGRAMSARBPROC				glDeletePrograms;
	extern PFNGLGENPROGRAMSARBPROC					glGenPrograms;
	extern PFNGLPROGRAMENVPARAMETER4DARBPROC		glProgramEnvParameter4d;
	extern PFNGLPROGRAMENVPARAMETER4DVARBPROC		glProgramEnvParameter4dv;
	extern PFNGLPROGRAMENVPARAMETER4FARBPROC		glProgramEnvParameter4f;
	extern PFNGLPROGRAMENVPARAMETER4FVARBPROC		glProgramEnvParameter4fv;
	extern PFNGLPROGRAMLOCALPARAMETER4DARBPROC		glProgramLocalParameter4d;
	extern PFNGLPROGRAMLOCALPARAMETER4DVARBPROC		glProgramLocalParameter4dv;
	extern PFNGLPROGRAMLOCALPARAMETER4FARBPROC		glProgramLocalParameter4f;
	extern PFNGLPROGRAMLOCALPARAMETER4FVARBPROC		glProgramLocalParameter4fv;
	extern PFNGLGETPROGRAMENVPARAMETERDVARBPROC		glGetProgramEnvParameterdv;
	extern PFNGLGETPROGRAMENVPARAMETERFVARBPROC		glGetProgramEnvParameterfv;
	extern PFNGLGETPROGRAMLOCALPARAMETERDVARBPROC	glGetProgramLocalParameterdv;
	extern PFNGLGETPROGRAMLOCALPARAMETERFVARBPROC	glGetProgramLocalParameterfv;
	extern PFNGLGETPROGRAMIVARBPROC					glGetProgramiv;
	extern PFNGLGETPROGRAMSTRINGARBPROC				glGetProgramString;
	extern PFNGLGETVERTEXATTRIBDVARBPROC			glGetVertexAttribdv;
	extern PFNGLGETVERTEXATTRIBFVARBPROC			glGetVertexAttribfv;
	extern PFNGLGETVERTEXATTRIBIVARBPROC			glGetVertexAttribiv;
	extern PFNGLGETVERTEXATTRIBPOINTERVARBPROC		glGetVertexAttribPointerv;
	extern PFNGLISPROGRAMARBPROC					glIsProgram;
#endif

#ifdef GL_ARB_vertex_buffer_object
	extern PFNGLBINDBUFFERARBPROC					glBindBuffer;
	extern PFNGLDELETEBUFFERSARBPROC				glDeleteBuffers;
	extern PFNGLGENBUFFERSARBPROC					glGenBuffers;
	extern PFNGLISBUFFERARBPROC						glIsBuffer;
	extern PFNGLBUFFERDATAARBPROC					glBufferData;
	extern PFNGLBUFFERSUBDATAARBPROC				glBufferSubData;
	extern PFNGLGETBUFFERSUBDATAARBPROC				glGetBufferSubData;
	extern PFNGLMAPBUFFERARBPROC					glMapBuffer;
	extern PFNGLUNMAPBUFFERARBPROC					glUnmapBuffer;
	extern PFNGLGETBUFFERPARAMETERIVARBPROC			glGetBufferParameteriv;
	extern PFNGLGETBUFFERPOINTERVARBPROC			glGetBufferPointerv;
#else
	#error extension not available
#endif

#ifdef GL_EXT_compiled_vertex_array
	extern PFNGLLOCKARRAYSEXTPROC					glLockArrays;
	extern PFNGLUNLOCKARRAYSEXTPROC					glUnlockArrays;
#endif

#ifdef GL_ARB_shader_objects
	extern PFNGLDELETEOBJECTARBPROC					glDeleteObject;
	extern PFNGLGETHANDLEARBPROC					glGetHandle;
	extern PFNGLDETACHOBJECTARBPROC					glDetachObject;
	extern PFNGLCREATESHADEROBJECTARBPROC			glCreateShaderObject;
	extern PFNGLSHADERSOURCEARBPROC					glShaderSource;
	extern PFNGLCOMPILESHADERARBPROC				glCompileShader;
	extern PFNGLCREATEPROGRAMOBJECTARBPROC			glCreateProgramObject;
	extern PFNGLATTACHOBJECTARBPROC					glAttachObject;
	extern PFNGLLINKPROGRAMARBPROC					glLinkProgram;
	extern PFNGLUSEPROGRAMOBJECTARBPROC				glUseProgramObject;
	extern PFNGLVALIDATEPROGRAMARBPROC				glValidateProgram;
	extern PFNGLUNIFORM1FARBPROC					glUniform1f;
	extern PFNGLUNIFORM2FARBPROC					glUniform2f;
	extern PFNGLUNIFORM3FARBPROC					glUniform3f;
	extern PFNGLUNIFORM4FARBPROC					glUniform4f;
	extern PFNGLUNIFORM1IARBPROC					glUniform1i;
	extern PFNGLUNIFORM2IARBPROC					glUniform2i;
	extern PFNGLUNIFORM3IARBPROC					glUniform3i;
	extern PFNGLUNIFORM4IARBPROC					glUniform4i;
	extern PFNGLUNIFORM1FVARBPROC					glUniform1fv;
	extern PFNGLUNIFORM2FVARBPROC					glUniform2fv;
	extern PFNGLUNIFORM3FVARBPROC					glUniform3fv;
	extern PFNGLUNIFORM4FVARBPROC					glUniform4fv;
	extern PFNGLUNIFORM1IVARBPROC					glUniform1iv;
	extern PFNGLUNIFORM2IVARBPROC					glUniform2iv;
	extern PFNGLUNIFORM3IVARBPROC					glUniform3iv;
	extern PFNGLUNIFORM4IVARBPROC					glUniform4iv;
	extern PFNGLUNIFORMMATRIX2FVARBPROC				glUniformMatrix2fv;
	extern PFNGLUNIFORMMATRIX3FVARBPROC				glUniformMatrix3fv;
	extern PFNGLUNIFORMMATRIX4FVARBPROC				glUniformMatrix4fv;
	extern PFNGLGETOBJECTPARAMETERFVARBPROC			glGetObjectParameterfv;
	extern PFNGLGETOBJECTPARAMETERIVARBPROC			glGetObjectParameteriv;
	extern PFNGLGETINFOLOGARBPROC					glGetInfoLog;
	extern PFNGLGETATTACHEDOBJECTSARBPROC			glGetAttachedObjects;
	extern PFNGLGETUNIFORMLOCATIONARBPROC			glGetUniformLocation;
	extern PFNGLGETACTIVEUNIFORMARBPROC				glGetActiveUniform;
	extern PFNGLGETUNIFORMFVARBPROC					glGetUniformfv;
	extern PFNGLGETUNIFORMIVARBPROC					glGetUniformiv;
	extern PFNGLGETSHADERSOURCEARBPROC				glGetShaderSource;
#endif

#ifdef WGL_EXT_swap_control
	extern PFNWGLSWAPINTERVALEXTPROC				wglSwapInterval;
	extern PFNWGLGETSWAPINTERVALEXTPROC				wglGetSwapInterval;
#endif

#ifdef WGL_ARB_pixel_format
	extern PFNWGLCHOOSEPIXELFORMATARBPROC			wglChoosePixelFormat;
	extern PFNWGLGETPIXELFORMATATTRIBIVARBPROC		wglGetPixelFormatAttribiv;
	extern PFNWGLGETPIXELFORMATATTRIBFVARBPROC		wglGetPixelFormatAttribfv;
#endif

#ifdef WGL_ARB_extensions_string
	extern PFNWGLGETEXTENSIONSSTRINGARBPROC			wglGetExtensionsString;
#endif

#ifdef WGL_ARB_pbuffer
	extern PFNWGLCREATEPBUFFERARBPROC				wglCreatePbuffer;
	extern PFNWGLGETPBUFFERDCARBPROC				wglGetPbufferDC;
	extern PFNWGLRELEASEPBUFFERDCARBPROC			wglReleasePbufferDC;
	extern PFNWGLDESTROYPBUFFERARBPROC				wglDestroyPbuffer;
	extern PFNWGLQUERYPBUFFERARBPROC				wglQueryPbuffer;
#endif

#if defined(GLX_SGIX_fbconfig) && !defined(GLX_VERSION_1_3)
	extern PFNGLXGETFBCONFIGATTRIBSGIXPROC			glXGetFBConfigAttrib;
	extern PFNGLXCHOOSEFBCONFIGSGIXPROC				glXChooseFBConfig;
	extern PFNGLXCREATEGLXPIXMAPWITHCONFIGSGIXPROC	glXCreateGLXPixmapWithConfig;
	extern PFNGLXCREATECONTEXTWITHCONFIGSGIXPROC	glXCreateContextWithConfig;
	extern PFNGLXGETVISUALFROMFBCONFIGSGIXPROC		glXGetVisualFromFBConfig;
	extern PFNGLXGETFBCONFIGFROMVISUALSGIXPROC		glXGetFBConfigFromVisual;
#endif


#ifdef WGL_ARB_render_texture
	extern PFNWGLBINDTEXIMAGEARBPROC				wglBindTexImage;
	extern PFNWGLRELEASETEXIMAGEARBPROC				wglReleaseTexImage;
	extern PFNWGLSETPBUFFERATTRIBARBPROC			wglSetPbufferAttrib;
#endif

#ifdef GL_ARB_occlusion_query
	extern PFNGLGENQUERIESARBPROC					glGenQueries;
	extern PFNGLDELETEQUERIESARBPROC				glDeleteQueries;
	extern PFNGLISQUERYARBPROC						glIsQuery;
	extern PFNGLBEGINQUERYARBPROC					glBeginQuery;
	extern PFNGLENDQUERYARBPROC						glEndQuery;
	extern PFNGLGETQUERYIVARBPROC					glGetQueryiv;
	extern PFNGLGETQUERYOBJECTIVARBPROC				glGetQueryObjectiv;
	extern PFNGLGETQUERYOBJECTUIVARBPROC			glGetQueryObjectuiv;
#endif

#ifdef WGL_ARB_make_current_read
	extern PFNWGLMAKECONTEXTCURRENTARBPROC			wglMakeContextCurrent;
	extern PFNWGLGETCURRENTREADDCARBPROC			wglGetCurrentReadDC;
#endif

#ifdef GL_ARB_window_pos
	extern PFNGLWINDOWPOS2DPROC						glWindowPos2d;
	extern PFNGLWINDOWPOS2DVPROC					glWindowPos2dv;
	extern PFNGLWINDOWPOS2FPROC						glWindowPos2f;
	extern PFNGLWINDOWPOS2FVPROC					glWindowPos2fv;
	extern PFNGLWINDOWPOS2IPROC						glWindowPos2i;
	extern PFNGLWINDOWPOS2IVPROC					glWindowPos2iv;
	extern PFNGLWINDOWPOS2SPROC						glWindowPos2s;
	extern PFNGLWINDOWPOS2SVPROC					glWindowPos2sv;
	extern PFNGLWINDOWPOS3DPROC						glWindowPos3d;
	extern PFNGLWINDOWPOS3DVPROC					glWindowPos3dv;
	extern PFNGLWINDOWPOS3FPROC						glWindowPos3f;
	extern PFNGLWINDOWPOS3FVPROC					glWindowPos3fv;
	extern PFNGLWINDOWPOS3IPROC						glWindowPos3i;
	extern PFNGLWINDOWPOS3IVPROC					glWindowPos3iv;
	extern PFNGLWINDOWPOS3SPROC						glWindowPos3s;
	extern PFNGLWINDOWPOS3SVPROC					glWindowPos3sv;
#endif

#ifdef GL_EXT_geometry_shader4
	extern PFNGLPROGRAMPARAMETERIEXTPROC 			glProgramParameteri;
#endif

#ifdef GL_ARB_framebuffer_object
	extern PFNGLISRENDERBUFFERPROC					glIsRenderbuffer;
	extern PFNGLBINDRENDERBUFFERPROC				glBindRenderbuffer;
	extern PFNGLDELETERENDERBUFFERSPROC				glDeleteRenderbuffers;
	extern PFNGLGENRENDERBUFFERSPROC				glGenRenderbuffers;
	extern PFNGLRENDERBUFFERSTORAGEPROC				glRenderbufferStorage;
	extern PFNGLGETRENDERBUFFERPARAMETERIVPROC		glGetRenderbufferParameteriv;
	extern PFNGLISFRAMEBUFFERPROC					glIsFramebuffer;
	extern PFNGLBINDFRAMEBUFFERPROC					glBindFramebuffer;
	extern PFNGLDELETEFRAMEBUFFERSPROC				glDeleteFramebuffers;
	extern PFNGLGENFRAMEBUFFERSPROC					glGenFramebuffers;
	extern PFNGLCHECKFRAMEBUFFERSTATUSPROC			glCheckFramebufferStatus;
	extern PFNGLFRAMEBUFFERTEXTURE1DPROC			glFramebufferTexture1D;
	extern PFNGLFRAMEBUFFERTEXTURE2DPROC			glFramebufferTexture2D;
	extern PFNGLFRAMEBUFFERTEXTURE3DPROC			glFramebufferTexture3D;
	extern PFNGLFRAMEBUFFERRENDERBUFFERPROC			glFramebufferRenderbuffer;
	extern PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVPROC	glGetFramebufferAttachmentParameteriv;
	extern PFNGLGENERATEMIPMAPPROC					glGenerateMipmap;
	extern PFNGLBLITFRAMEBUFFERPROC					glBlitFramebuffer;
	extern PFNGLRENDERBUFFERSTORAGEMULTISAMPLEPROC	glRenderbufferStorageMultisample;
	extern PFNGLFRAMEBUFFERTEXTURELAYERPROC			glFramebufferTextureLayer;
#endif

#ifdef GL_EXT_blend_func_separate
	extern PFNGLBLENDFUNCSEPARATEEXTPROC			glBlendFuncSeparate;
#endif

#ifdef GL_VERSION_2_0
	extern PFNGLDRAWBUFFERSPROC						glDrawBuffers;
#endif

}

using namespace OpenGLExtPointer;

#ifndef __CUDACC__
	namespace Engine
	{
		template <class C>
		void Extension::Read(C&target, const String&name)
		{
			#if SYSTEM==WINDOWS
				target = (C)wglGetProcAddress(name.c_str());
			#elif SYSTEM==UNIX
				target = (C)glXGetProcAddressARB((const GLubyte*)name.c_str());
			#else
				target = NULL;
			#endif
			if (!target)
				ErrMessage("Critical OpenGL extension exception: Pointer not found: "+name);
		}

		template <class C>
		void Extension::ReadSilent(C&target, const String&name)
		{
			#if SYSTEM==WINDOWS
				target = (C)wglGetProcAddress(name.c_str());
			#elif SYSTEM==UNIX
				target = (C)glXGetProcAddressARB((const GLubyte*)name.c_str());
			#else
				target = NULL;
			#endif
		}
	}
#endif


#endif

