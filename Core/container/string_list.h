#ifndef string_listH
#define string_listH



#include "../math/basic.h"
#include "sorter.h"
#include "../global_string.h"

#include "hashtable.h"
#include "buffer.h"
#include "../general/undef.h"

namespace DeltaWorks
{
	namespace Container
	{
		class StringList;
	}

	namespace Tokenizer
	{
		class ItemSet:protected Container::StringSet
		{
		protected:
			typedef	Container::StringSet Super;
			size_t			max_len;
			bool			first_char[0x100];
			
			void			addTokens(const char*string);
		public:
			/**/			ItemSet():max_len(0)
			{
				memset(first_char,false,sizeof(first_char));
			}
			/**/			ItemSet(const String&token_string):max_len(0)
			{
				memset(first_char,false,sizeof(first_char));
				addTokens(token_string.c_str());
			}
			/**/			ItemSet(const char*token_string):max_len(0)
			{
				memset(first_char,false,sizeof(first_char));
				addTokens(token_string);
			}
			ItemSet&		operator=(const String&token_string)
			{
				clear();
				addTokens(token_string.c_str());
				return *this;
			}

			ItemSet&		operator=(const char*token_string)
			{
				clear();
				addTokens(token_string);
				return *this;
			}
			void			clear()
			{
				Super::clear();
				max_len = 0;
				memset(first_char,false,sizeof(first_char));
			}
			inline	size_t	maxLength()	const	//! Retrieves the maximum item length (in characters) of the local set 
			{
				return max_len;
			}
			inline	bool	itemBeginsWith(BYTE c) const	//! Checks whether or not any of the contained items begins with the specified character (cast to size_t char)
			{
				return first_char[c];
			}
			using Super::IsSet;
		};


		class CharacterTable	//! Character table, providing the respective index for set characters and 0 for undefined characters
		{
		protected:
			size_t			field[0x100];
		public:
			/**/			CharacterTable()
			{
				memset(field,0,sizeof(field));
			}
			/**/			CharacterTable(const String&tokens)	//!< Sets the local character map based on the characters found in the specified string
			{
				memset(field,0,sizeof(field));
				const char*c = tokens.c_str(),*first = c;
				while (*c)
				{
					field[(BYTE)*c] = c-first+1;
					c++;
				}
			}
			/**/			CharacterTable(const char*tokens)
			{
				memset(field,0,sizeof(field));
				const char*c = tokens,*first = c;
				while (*c)
				{
					field[(BYTE)*c] = c-first+1;
					c++;
				}
			}
							
			CharacterTable&	operator=(const String&tokens)		//!< Sets the local character map based on the characters found in the specified string
			{
				memset(field,0,sizeof(field));
				const char*c = tokens.c_str(),*first = c;
				while (*c)
				{
					field[(BYTE)*c] = c-first+1;
					c++;
				}
				return *this;
			}
			CharacterTable&	operator=(const char*tokens)		//!< Sets the local character map based on the characters found in the specified string
			{
				memset(field,0,sizeof(field));
				const char*c = tokens,*first = c;
				while (*c)
				{
					field[(BYTE)*c] = c-first+1;
					c++;
				}
				return *this;
			}
			CharacterTable&	operator=(char token)		//!< Sets the local character map to the single given character
			{
				memset(field,0,sizeof(field));
				field[(BYTE)token] = 1;
				return *this;
			}
			
			inline	size_t	Query(BYTE c) const	//! Queries the index entry of the specified character (cast to size_t char). The returned value will be the index of the specified character in the original string (plus 1) or 0 if the character is not defined in this table
			{
				return field[c];
			}
			inline	void	clear()		//! Clears the local character set
			{
				memset(field,0,sizeof(field));
			}
		};


		class CharacterSet	//! Character set using a boolean field
		{
		protected:
			bool				field[0x100];
		public:
			/**/			CharacterSet()
			{
				memset(field,false,sizeof(field));
			}
			/**/			CharacterSet(const String&tokens)	//!< Sets the local character map based on the characters found in the specified string
			{
				memset(field,false,sizeof(field));
				const char*c = tokens.c_str();
				while (*c)
					field[(BYTE)*c++] = true;
			}
			/**/			CharacterSet(const char*tokens)
			{
				memset(field,false,sizeof(field));
				const char*c = tokens;
				while (*c)
					field[(BYTE)*c++] = true;
			}
							
			CharacterSet&	operator=(const String&tokens)		//!< Sets the local character map based on the characters found in the specified string
			{
				memset(field,false,sizeof(field));
				const char*c = tokens.c_str();
				while (*c)
					field[(BYTE)*c++] = true;
				return *this;
			}
			CharacterSet&	operator=(const char*tokens)		//!< Sets the local character map based on the characters found in the specified string
			{
				memset(field,false,sizeof(field));
				const char*c = tokens;
				while (*c)
					field[(BYTE)*c++] = true;
				return *this;
			}
			
			inline	bool			isset(BYTE c) const	//! Checks if a specific character is set (cast to unsigned char)
			{
				return field[c];
			}
			inline	void			clear()		//! Clears the local character set
			{
				memset(field,false,sizeof(field));
			}
			inline	void			Set(BYTE c, bool b = true)
			{
				field[c] = b;
			}
		};


	
		class Configuration	//! Tokenizer configuration. Warning: Can be slow to copy
		{
		public:
			CharacterSet		main_separator,	//!< Primary field of token separator characters. Most commonly at least space. Primary separators will not be inserted into the final token list
								trim_characters;	//!< Field of characters that should be trimmed at the beginning and end of each token. Should a trimmed string become empty this way then it will not be made token
			/** 
				@brief Field of recursion tokens (up). 
				
				When encountered, the recursion level will increase until the corresponding entry of @a recursion_down is encountered. 
				Content between (and including) a character of recursion_up and its respective counter part are considered one token.
				As the name suggests, characters of this table located within such a token further increase recursion
				
				Typical use for this table would be strings like "([{"
			*/
			CharacterTable		recursion_up,
								recursion_down,	//!< Counter part to @a recursion_up . Typical use for this table would be strings like ")]}"
								quotations;		//!< Field of characters that are considered quotations. Only the same quotation character can terminate a started string quotation. Neither tokens nor recursion nor inner strings are detected within quotations
								
			/**
				@brief Set of operator tokens.
				
				Operators are queried longest to shortest and define a single token if encountered.
			*/
			ItemSet				operators;
			char				quotation_escape_character;	//!< Character used to negate the effect of an encountered following quotation. Two subsequent escape characters are reduced to just one. A single effect-less escape character is left unchanged
			bool				recursion_break,	//!< Set true if areas from a bottom level @a recursion_up character to its respective @a recursion_down character should be considered a separate token even if none of the other tokenize conditions is met
								string_break,		//!< Set true if areas from a bottom level quotation begin to its respective quotation end should be considered a separate token even if none of the other tokenize conditions is met
								convert_tabs;		//!< Set true to automatically convert tab-characters ('\t') to space
			
			
			/**/				Configuration():main_separator(" \t"),trim_characters(" \t"),quotation_escape_character(0),recursion_break(false),string_break(false),convert_tabs(true)
								{}
			/**/				Configuration(	const String&main_separator_, const String&trim_characters_, const String&recursion_up_, const String&recursion_down_,
												const String&quotations_,const String&operators_, char quotation_escape_character_,
												bool recursion_break_, bool string_break_, bool convert_tabs_):
												main_separator(main_separator_),trim_characters(trim_characters_),recursion_up(recursion_up_),recursion_down(recursion_down_),
												quotations(quotations_),operators(operators_),quotation_escape_character(quotation_escape_character_),
												recursion_break(recursion_break_),string_break(string_break_),convert_tabs(convert_tabs_)
												{}
			/**/				Configuration(	const char*main_separator_, const char*trim_characters_, const char*recursion_up_, const char*recursion_down_,
												const char*quotations_,const char*operators_, char quotation_escape_character_,
												bool recursion_break_, bool string_break_, bool convert_tabs_):
												main_separator(main_separator_),trim_characters(trim_characters_),recursion_up(recursion_up_),recursion_down(recursion_down_),
												quotations(quotations_),operators(operators_),quotation_escape_character(quotation_escape_character_),
												recursion_break(recursion_break_),string_break(string_break_),convert_tabs(convert_tabs_)
												{}
		};
	
		typedef Configuration		Config;
	
		class TemporaryStatus
		{
		public:
			size_t			recursion_buffer[0x100];	//!< Encountered character index that started the recursion
			BYTE			recursion_level;			//!< Current recursive level
			size_t			in_string;					//!< Index of the encountered quotation character if any. 0 if not in string

			/**/			TemporaryStatus():recursion_level(0),in_string(0)
							{}

		};

		class Status:public TemporaryStatus	//! Status including string buffer
		{
		public:
			Container::Buffer<char>	buffer;		//!< Buffer used to store characters

			/**/					Status()
									{}
			/**/					Status(size_t len):buffer(len)
									{}
		};
	
		struct TSegment	//! String segment
		{
			const char				*first,
									*last;
		};
	
		/**
		@brief Tokenizes a string composition in accordance to the specified configuration
		
		@param composition Composition string to tokenize
		@param config Tokenizer configuration
		@param target String list to append identified tokens to
		@param clear_list Clears the target list before adding tokens to it
		
		@return true if the string is valid in accordance with the specified tokenizer configuration, false if strings or recursive layers are left open when the tokenizer reaches the end of the specified composition
		*/
		bool			tokenize(const char*composition, const Configuration&config, Container::StringList&target, bool clear_list=true);
		bool			tokenize(const String&composition, const Configuration&config, Container::StringList&target, bool clear_list=true);	//!< @overload
		bool			Tokenize(const StringRef&composition, const Configuration&config, Container::BasicBuffer<StringRef>&target, bool clear_list=true);
	
	
		void			tokenizeSegment(const String&composition_segment, const Configuration&config, Status&status, Container::StringList&target);
		void			tokenizeSegment(const char*composition_segment, const Configuration&config, Status&status, Container::StringList&target);
		bool			finish(const Configuration&config, Status&status, Container::StringList&target);
	
	
		String			trim(const String&string, const Configuration&config);
		TSegment		trim(const TSegment&segment, const Configuration&config);
		bool			trimSegment(TSegment&segment, const Configuration&config);
	
		/**
		@brief Checks the quotation of the specified string and returns the number of quoted characters
		
		For this function to return non-0 the specified string must begin with a quotation character and end with the same quotation character again
		
		@param string String to check
		@param config Configuration to use to determine quotation and escape characters
		@return Length of the string plus 1 inbetween the leading and trailing quotation characters, excluding escape characters. 0 if the quotation is invalid
		*/
		size_t			CheckQuotation(const StringRef&string, const Configuration&config);
	
		/**
		@brief Removes quotation characters from the beginning and end as well as escape characters from the specified string
		
		@param string String to dequote
		@param config Configuration to determine what a string quote is
		@param result String to write the dequoted string to
		@return True on success, false otherwise. @a result will contain the unmodified input string if dequote() failed
		*/
		bool			Dequote(const StringRef&string, const Configuration&config, String&result);
		bool			DequoteChangesString(const StringRef&string, const Configuration&config);
	
		/**
		@brief Similar to the above but without signaling success of failure
		
		If dequoting failed then the result will be the unmidified input string
		
		@param string String to dequote
		@param config Configuration to determine quotes from
		@return Dequoted string
		*/
		String			Dequote(const String&string, const Configuration&config);
		String			Dequote(const StringRef&string, const Configuration&config);

		/**
		@brief Dequotes all entries of the specified list

		@param list List to dequote
		@param config Configuration to determine quotes from
		@return true if all entries of the specified list could be dequoted successfully, false if at least one failed

		*/
		bool			DequoteAll(Container::StringList&list, const Configuration&config);
	
	}

	namespace Container
	{
		class StringList:private Buffer<String,0,SwapStrategy>
		//	private List::M::Vector3<String>
		{
		private:
			//typedef	List::M::Vector3<String>	Super;
			typedef Buffer<String,0,SwapStrategy>	Super;

			
			String				fuse(size_t index, size_t length, const char*glue, size_t glue_len) const;
			void				setConfig();
			size_t				checkQuotation(const String&line)	const;
			
			struct	SBucket
			{
				String			*entry;
				SBucket			*next;
			};
			

			static String		empty_string;
		public:
			/**/				StringList();
			/**/				StringList(const String&Line);
			/**/				StringList(const char*Line);
			/**/				StringList(const StringList&other);

			void				add(const char*line);		//!< Appends a string to the end of the list
			void				add(const String&line);	//!< @overload
			void				add(String&&line);	//!< @overload
			void				add(const Tokenizer::TSegment&segment);	//!< @overload
			void				add(const StringRef&segment);	//!< @overload
			void				Insert(size_t index, const char*line);	//!< Inserts a string before the specified list index
			void				Insert(size_t index, const String&line);	//!< Inserts a string before the specified list index
			void				addEntries(const StringList&other);		//!< Appends all entries of the specified other list
			String&				longest();															//!< Returns a reference to the longest string found in the local list. Returns a reference to a global empty string if the local list is empty
			const String&		longest()													const;	//!< Const @overload
			String&				shortest();															//!< Returns a reference to the shortest string found in the local list. Returns a reference to a global empty string if the local list is empty
			const String&		shortest()													const;	//!< Const @overload
			String&				max();																//!< Returns a reference to the orthographic maximum string found in the local list. Returns a reference to a global empty string if the local list is empty
			const String&		max()														const;	//!< Const @overload
			String&				min();																//!< Returns a reference to the orthographic minimum string found in the local list. Returns a reference to a global empty string if the local list is empty
			const String&		min()														const;	//!< Const @overload
			String&				First();															//!< Returns a reference to the first string of the local list. Returns a reference to a global empty string if the local list is empty
			const String&		First()														const;	//!< Const @overload
			String&				Last();																//!< Returns a reference to the last string of the local list. Returns a reference to a global empty string if the local list is empty
			const String&		Last()														const;	//!< Const @overload
			size_t				charactersInCommon()										const;	//!< Queries how many characters (starting from the beginning) all contained strings have in common with each other. Comparison is case sensitive
			size_t				charactersInCommon(const String&sample)					const;	//!< Queries how many characters (starting from the beginning) all contained strings have in common with the specified sample. Comparison is case sensitive
			
			void				eraseOccurrences(const String&string, bool case_sensitive=true);	//!< Erases all entries that are equal (case sensitive or case ignore) to the specified string @param string String to match @param case_sensitive Set true to perform a case sensitive comparison, false otherwise
			String&				Get(size_t index);												//!< Retrieves a reference to the string of the specified index or a reference to a global empty string if the specified index is invalid
			const String&		Get(size_t index)											const;	//!< Const @overload
			String&				getReverse(size_t index);											//!< Similar to get() but retrieves the element from the end of the list rather than the beginning
			const String&		getReverse(size_t index)									const;	//!< Const @overload
			size_t				find(const String&line)									const;
			size_t				findCaseSensitive(const String&line)						const;
			size_t				findCaseIgnore(const String&line)							const;
			size_t				lookup(const String&line)									const;
			size_t				lookupCaseSensitive(const String&line)						const;
			size_t				lookupCaseIgnore(const String&line)						const;
			size_t				countOccurrences(const String&expression)					const;
			String				list()														const;
			
			std::ostream&		printLines(std::ostream&stream, const char*indent)					const;	//!< Prints the local list to the specified out stream (e.g. cout) @param stream Stream to print to (e.g. cout) @param indent Indent string that should be printed before each new line @return parameter stream
			std::ostream&		printLines(std::ostream&stream, const String&indent)					const;	//!< Prints the local list to the specified out stream (e.g. cout) @param stream Stream to print to (e.g. cout) @param indent Indent string that should be printed before each new line @return parameter stream
			INLINE String		fuse(size_t index, size_t count, const String&glue)	const;
			INLINE String		fuse(size_t index, size_t count, const char*glue)		const;
			INLINE String		fuse(size_t index, size_t count, char glue)				const;
			INLINE String		fuse(size_t index, const String&glue)					const;
			INLINE String		fuse(size_t index, const char*glue)						const;
			INLINE String		fuse(size_t index, char glue)								const;
			INLINE String		implode(const String&glue)									const;
			INLINE String		implode(const char*glue)									const;
			INLINE String		implode(char glue)											const;
			INLINE String&		operator[](size_t index);
			INLINE const String&operator[](size_t index)									const;
			StringList&			operator=(const StringList&other);
			
			StringList&			operator<<(String&&string);
			StringList&			operator<<(const String&string);
			StringList&			operator<<(const Tokenizer::TSegment&segment);	//!< @overload
			StringList&			operator<<(const StringRef&segment);	//!< @overload
			StringList&			operator<<(const char*string);	//!< @overload
			
			//bool				operator>>(String&target);
			operator			size_t()													const;
			bool				IsSorted()													const;
			void				QuickSort();
			void				RadixSort();
			void				swap(StringList&other)
			{
				Super::swap(other);
			}
			void				adoptData(StringList&other)
			{
				Super::adoptData(other);
			}

			using Super::clear;
			using Super::Clear;
			using Super::Count;
			using Super::size;
			using Super::Revert;
			using Super::IsEmpty;
			using Super::IsNotEmpty;
			using Super::erase;
			using Super::Erase;
			using Super::append;
			using Super::Append;
			using Super::moveAppend;
			using Super::MoveAppend;
			using Super::moveToArray;
			using Super::MoveToArray;
			using Super::copyToArray;
			using Super::CopyToArray;
			using Super::begin;
			using Super::end;
			
		};




		#include "string_list.tpl.h"
	}
}

#endif
