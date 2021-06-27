#ifndef configH
#define configH

/******************************************************************

Extended configuration-file-parser.

******************************************************************/



#include <stdio.h>

#include "../global_string.h"


#include "../container/string_list.h"
#include "../string/string_converter.h"
//#include "../general/useful.h"
#include "../io/string_file.h"
#include "../io/xml.h"

#include "../container/hashtable.h"
#include "../container/hashMappedList.h"

#include <time.h>

#define CFG_NO_ERROR				0
#define CFG_FILE_NOT_FOUND_ERROR	(-1)
#define CFG_PARSE_ERROR				(-2)
#define CFG_OUT_OF_CONTEXT_CMD		(-3)
#define CFG_SYNTAX_ERROR			(-4)
#define CFG_TYPE_ERROR				(-5)
#define CFG_UNEXPECTED_TYPE			(-6)
#define CFG_DEF_NOT_FOUND_ERROR		(-7)
//#define 


#include "log.h"	//for debugging purposes

namespace DeltaWorks
{

	namespace Config
	{

		extern LogFile  					logfile;
		
		using namespace DeltaWorks::Container;
		
		class Context
		{
			typedef Context					Self;
		public:
			struct Attribute
			{
				typedef Attribute		Self;

				String					name,
										assignment_operator,
										value,
										comment;
				bool					commented;	//indicates that the entire line Is to be commented

				void					swap(Attribute&other)
				{
					name.swap(other.name);
					assignment_operator.swap(other.assignment_operator);
					value.swap(other.value);
					comment.swap(other.comment);
					swp(commented,other.commented);
				}
				friend void				swap(Self&a, Self&b)	{a.swap(b);}
			};
			
		protected:
			Buffer0<Context,Swap>			children;
			StringTable<index_t>			childMap;
			Buffer0<Attribute,Swap>			attributes;
			StringTable<index_t>			attributeMap;
			Buffer0<Context,Swap>			modes;
			StringTable<index_t>			modeMap;
			bool							isMode;

			Attribute&					protectedDefine(const String&name, const String&value="");
			Context&					protectedDefineContext(const String&name);	//!< Defines a new context within this one if one of the specified name does not already exist
			Context&					protectedDefineContext(const String*names, count_t num_names);
		public:
			String						name,	//!< Name 
										comment;

										Context():isMode(false)	{};

			void						swap(Context&other)
			{
				comment.swap(other.comment);
				children.swap(other.children);
				childMap.swap(other.childMap);
				attributes.swap(other.attributes);
				attributeMap.swap(other.attributeMap);
				modes.swap(other.modes);
				modeMap.swap(other.modeMap);
				swp(isMode,other.isMode);
				name.swap(other.name);
			}
			friend void					swap(Self&a, Self&b)	{a.swap(b);}
											
			virtual						~Context()	{};
		
			/**
			Attempts to update the given attribute to the specified new value.
			An Except::Program::DataConsistencyFault exception is thrown if the specified path is not found.
			If missing variables are expected, use Define() instead.
			@param path Attribute path to look for
			@param newValue Attribute value to update to
			@return Reference to the found attribute
			*/
			Attribute&					Change(const String&path, const String&newValue);
			Attribute&					Change(const String&path, int newValue)		{return Change(path,String(newValue));}
			Attribute&					Change(const String&path, float newValue)	{return Change(path,String(newValue));}
			Attribute&					Change(const String&path, bool newValue)	{return Change(path,String(newValue));}
			Attribute&					Change(const String&path, const char*newValue)	{return Change(path,String(newValue));}
			Attribute&					Define(const String&name, const String&value="");
			Attribute&					Define(const String&name, int value)		{return Define(name,String(value));}
			Attribute&					Define(const String&name, float value)		{return Define(name,String(value));}
			Attribute&					Define(const String&name, bool value)		{return Define(name,String(value));}
			Attribute&					Define(const String&name, const char*value)		{return Define(name,String(value));}
			Context&					DefineContext(const String&name);	//!< Defines a new context within this one if one of the specified name does not already exist
			Context&					CreateContext(const String&name);	//!< Defines a new context within this one even if one of the specified name already exists.
			Context&					DefineModeContext(const String&mode_name, const String&context_name);
			Context&					CreateModeContext(const String&mode_name, const String&context_name);
			Context&					DefineMode(const String&name);
				
			Attribute*					GetAttrib(const String&path);
			const Attribute*			GetAttrib(const String&path)						const;
			const String&				GetOperator(const String&path)						const;
			const String&				GetComment(const String&path)						const;
			const String&				Get(const String&path, const String&except="")	const;
			const String&				GetString(const String&path, const String&except="")	const	{return Get(path,except);}
			bool						QueryString(const String&path, String&outResult)	const;
			const String&				RequireString(const String&path)	const;
			int							RequireInt(const String&path)			const;
			int							GetInt(const String&path, int except=0)			const;
			unsigned					RequireUnsigned(const String&path)	const;
			unsigned					GetUnsigned(const String&path, unsigned except=0)	const;
			float						RequireFloat(const String&path)		const;
			float						GetFloat(const String&path, float except=0)		const;
			bool						RequireBool(const String&path)		const;
			bool						GetBool(const String&path, bool except=false)		const;
			Key::Name					RequireKey(const String&path)		const;
			Key::Name					GetKey(const String&path, Key::Name except)		const;
			Context*					GetContext(const String&path);	//path of form: 'mode:folder/folder/folder' or 'folder/folder/folder'
			Context*					GetContext(const String*path, size_t path_len, const String&mode);
			const Context*				GetContext(const String&path)						const;
			const Context*				GetContext(const String*path, size_t path_len, const String&mode)	const;
				
				
			bool						IsMode() const	{return isMode;};
			bool						IsEmpty() 											const;
			void						ExportModes(Ctr::ArrayData<Context*>&out);
			void						ExportModes(Ctr::ArrayData<const Context*>&out)			const;
			void						ExportChildren(Ctr::ArrayData<Context*>&out);
			void						ExportChildren(Ctr::ArrayData<const Context*>&out)			const;
			void						ExportAttributes(Ctr::ArrayData<Attribute*>&out);
			void						ExportAttributes(Ctr::ArrayData<const Attribute*>&out)		const;

			virtual	void				Clear();
			void						WriteContent(StringBuffer&, size_t max_name_len, size_t max_value_len, const String&indent="") const;
			void						Cleanup();	//!< Recursivly erases all empty sub contexts and modes
				
			void						RetrieveMaxNameValueLength(size_t&name_len, size_t&value_len, size_t indent=0)	const;
				
		};
		
		class Container : public Context
		{
		protected:
			String							error;
				
			class Config:public Tokenizer::Configuration
			{
			public:
				/**/						Config();
			};
			static Config					configuration;
		
			static unsigned					GetDepth(const char*line);
		
				
		public:
			/**/							Container();
			/**/							Container(const PathString&filename);
											
			static bool						IsOperator(const String&string);
			bool							LoadFromFile(const PathString&filename);
			bool							LoadFromFile(StringFile&file);
			bool							SaveToFile(const PathString&filename, const String&generatorName);
			void							SaveToFile(StringFile&file, const String&generatorName);
			void							SaveToStringBuffer(StringBuffer&, const String&generatorName);
			bool							HasErrors()	const;
			const String&					GetError()	const;
			const String&					Report() const {return GetError();}
			virtual	void					Clear() override;
		};
		
		
		#if 0

		class CXContext;
		typedef std::shared_ptr<CXContext>	PCXContext;

		/**
		@brief XML based configuration context
			
		CXContext loads its configuration from XML nodes or entire files.
		The general format Is:<br>
		<br>
		&lt;some_config&gt;<br>
		variable1 = value;<br>
		variable2 = value1, value2, value3;<br>
		variable3 = ($variable1), ($variable2);<br>
		&lt;sub_config&gt;<br>
			variable4 = hello world;<br>
		&lt;/sub_config&gt;<br>
		variable5 = ($sub_config.variable4);
		condition_variable = yes;
		&lt;conditional_sub condition_variable="yes"&gt;<br>
			variable4 = hello world;<br>
		&lt;/conditional_sub&gt;<br>
		&lt;/some_config&gt;<br>
		<br>
		One instance of CXContext may contain any number of variables and sub contexts.<br>
		@b Variables: <br>
		Variables are defined in the form [name] = [value];<br>
		The must provide a '=' character but may be empty ([name] = ;). They must be terminated
		by a semicolon.<br>
		Variables may each have any number of values (separated by comma) but only one name.
		Their values may contain embedded variables in the form ($path), which are replaced
		during finalization.
		The special tag ($*) may be used to inherit the previous value of the same variable during
		stacked loading. It may also be used to embed inherited values.<br>
		@b Groups: <br>
		Groups are defined as XML sub groups (<group>...</group>) and may be defined recursively.
		All attributes with the exception of @a inherit are considered conditions where
		name="value" equals <a>if (($name)=="value")</a>. A group Is ignored if one or more conditions
		are not met.<br>
		@a inherit specifies one or more other groups to inherit variables from. Only variables are
		ever inherited. If the local group does already specify a variable of same name then the contents
		of the inherited variable are inserted where ($*) tags are specified (if any).<br>
		<br>
		*/
		class CXContext
		{
		public:
			typedef Ctr::Array<String,Adopt>	Variable;	//!< Variable type. Variables may provide any number of elements
		protected:
			bool						variables_finalized;	//!< True if variable contents of all local variables have been processed
			Ctr::Array<XML::TAttribute>		conditions;			//!< Context conditions to be evaluated during finalization
			CXContext					*parent;			//!< Pointer to the next superior context. NULL if this context Is the top most
			Vector0<PCXContext>			pre_finalize_children;	//!< Sub contexts prior to finalization. Multiple contexts with the same name may co-exist if conditional variables are exclusive
				
			void						parse(const String&content);	//!< Parses a coherent variable block as extracted from XML inner and following content
			bool						Process(String&expression, bool singular, String*error_out)	const;	//!< Processes an expression that may contain variables
	
			Variable*					innerFindVariable(const Ctr::ArrayData<String>&segments);		//!< Recursively looks for a variable via its path
			const Variable*				innerFindVariable(const Ctr::ArrayData<String>&segments) const;	//!< @overload
			CXContext*					innerFindContext(const Ctr::ArrayData<String>&segments);		//!< Recursively looks for a context via its path
			const CXContext*			innerFindContext(const Ctr::ArrayData<String>&segments) const;	//!< @overload
			void						finalizeVariables();	//!< Processes all variables and removes inner variable expressions. Invocation will fail if one or more inner variables cannot be found. The method will only actually change anything if @a variables_finalized Is false and set same variable to true
			void						trimVariables();					//!< Trims all variables and removes empty and ($*) variable segments
				
			static	bool				validNameChar(char c);				//!< Checks if a character Is a valid name character @return true if valid
			static	bool				IsPathSeparator(char c);			//!< Checks if a character Is a path separation character ('.' or '/')
		
		
		public:
			String							name;		//!< Name of this context
			HashMappedList<String,Variable> variables;	//!< Variable table (one variable per name)
			HashMappedList<String,PCXContext>children;	//!< Sub context table (one per name)
				
											CXContext():variables_finalized(false),parent(NULL)
											{}
			/**
			@brief Attempts to locate a variable based on its path.

			The path may contain variables which are evaluated before lookup
					
			@param path Path of the variable to find.
				May include variables in the form ($variable_name)
			@param error_out Optional: String to put an error description into if the method fails
			@return true on success											
			*/
			Variable*					Lookup(String path, String*error_out=NULL);
			const Variable*				Lookup(String path, String*error_out=NULL)	const; //!< @overload
			void						Clear();	//!< Clears any local data and sub contexts
			/**
			@brief Loads the local content from an XML container
					
			Unless @a stack Is specified all local content will be replaced by any content found in the specified XML container
			and the configuration finalized on success.
					
			May throw exceptions
					
			@param container XML container to load from
			@param stack Set true to clear the local configuration before loading and automatically finalize the final configuration
			@param error_out Optional: String to put an error description into if the method fails
			*/
			void						LoadFromXML(const XML::Container&container, bool stack=false);
				
			/**
			@brief Identical to loadFromXML with the exception of that the XML content Is automatically loaded from the specified file
					
			May throw exceptions

			@param filename Name of the (XML) file to load
			@param stack Set true to clear the local configuration before loading and automatically finalize the final configuration
			*/
			void						LoadFromFile(const PathString&filename, bool stack=false);
			/**
			@brief Attempts to locate a variable via its path				
					
			A variable path may consist of any number of segments separated either by . or /
			Variables are located starting from the root context. The method will follow the parent
			sequence until parent it NULL before attempting to follow the specified path
			*/
			Variable*					FindVariable(const String&path);
			const Variable*				FindVariable(const String&path) const;	//!< @overload
			PCXContext					FindContext(const String&path) const;		//!< Similar to FindVariable() for contexts
			/**
			@brief Embedded loading method
					
			Content will be loaded from the specified XML node into the local context recursively.
			Existing variables will be overwritten possibly replacing ($*) tags via inheritance inclusion.
			Other variables are not replaced at this point.
			As the name suggests loadStacked() may be invoked several times each loading configuration content on top of the existing
			content. loadStacked() should no longer be invoked once finalize() was called.
					
			A method other than loadFromFile()/loadFromXML() must explicitly invoke clear() before and finalize() after the loading process.

			May throw exceptions
					
			@param node XML node to load from. Sub nodes are loaded into sub contexts recursively
			*/
			void						LoadStacked(const XML::Node*node);
			/**
			@brief Sets (creates/modifies) the specified variable to the specified value
					
			@param variable_name Path to the variable to set
			@param variable_value Value of the variable to set
			@return true on success
			*/
			bool						Set(const String&variable_name, const String&variable_value);
			bool						IsChildOf(const CXContext*other)	const;	//!< Checks if the local context Is a child of the specified other context @return true if the local context Is a decendent of the specified context, false otherwise
			/**
			@brief Finalizes the loading process
					
			Replaces ($...) variables in variable values and evaluates context conditions.
			Shifts contexts from @a pre_finalize_children to @a children if they passed evaluation.
			Also trims empty variable values.
			This method should only be invoked if loadStacked() or loadFrom...() were invoked with stack=true.
			Once finalize() has been called no further loadStacked() invocation should be performed.

			May throw exceptions
			*/
			void						Finalize();
				
			/**
			@brief Debug method to print the local configuration to the console.
			*/
			void						PrintToCOut(unsigned indent=0)	const;
		};
	
		typedef CXContext::Variable	CXVariable;

		#endif /*0*/
	}


	typedef Config::Container	Configuration;



}

#endif
