#ifndef xmlH
#define xmlH

#include "../global_string.h"
#include "../io/string_file.h"
#include "../string/string_buffer.h"
#include "../container/buffer.h"
#include "../container/hashtable.h"


/******************************************************************

E:\include\io\xml.h

******************************************************************/

namespace DeltaWorks
{

	namespace XML
	{
		struct TScannerNode;
		class ScannerRule;
		class Scanner;


		enum class Encoding
		{
			UTF8,
			Windows1252
		};



		enum export_style_t	//! Style used to export XML content to files or string buffers
		{
			Raw,	//!< Export data as available. Whitespaces and newlines are exported if they are contained in the exporting structure
			Nice,	//!< Export inner content if XML node has no children and ignore node-following content. Newlines and indentation are added where appropriate
			Tidy	//!< Export inner content if XML node has no children and ignore node-following content. Neither newlines nor indentation are added
		};
	

		struct  TAttribute
		{
			String					name,
									value;
		};


		class AttributeTable
		{
		public:

			typedef Vector0<TAttribute>::const_iterator	const_iterator;
			const_iterator		begin() const {return attributeList.begin();}
			const_iterator		end() const {return attributeList.end();}
			size_t				size() const {return attributeList.size();}

			void				Set(const String&attrib_name, const String&attrib_value);
			const String&		Require(const String&attribName) const;
			bool				Query(const String&attrib_name, String&val_out)	const;	//!< Queries the string content of the specified attribute of the local node. The method does not crash if @a this is NULL. \param attrib_name Name of the requested attribute \param val_out String reference to store the respective attribute's value in \return true if the local object is not NULL and the requested attribute exists, false otherwise.
			bool				Query(const char*attrib_name, String&val_out)	const;	//!< @copydoc Query()
			bool				Query(const String&attrib_name, StringRef&val_out)	const;	//!< Queries the string content of the specified attribute of the local node. The method does not crash if @a this is NULL. \param attrib_name Name of the requested attribute \param val_out String reference to store the respective attribute's value in \return true if the local object is not NULL and the requested attribute exists, false otherwise.
			bool				Query(const char*attrib_name, StringRef&val_out)	const;	//!< @copydoc Query()
			void				Unset(const String&attrib_name);

			friend void			swap(AttributeTable&a,AttributeTable&b)
			{
				a.swap(b);
			}
			void				swap(AttributeTable&other)
			{
				attributeList.swap(other.attributeList);
				attributeMap.swap(other.attributeMap);
			}
			void				Clear()
			{
				attributeList.Clear();
				attributeMap.Clear();
			}

		private:
			Vector0<TAttribute>	attributeList;
			StringTable<index_t>attributeMap;

		};

		class Node
		{
		public:
			/*No point making these private:*/
			String				name,
								inner_content,
								following_content;
			Vector0<Node,Swap>	children;
			AttributeTable		attributes;
								
			Node&				NewChild(const String&name);
			Node&				AddChild(const String&name)			{return NewChild(name);}
			Node&				AddNewChild(const String&name)		{return NewChild(name);}
			template <typename T>
				void			Set(const String&attribName, const T&attribValue)	{return attributes.Set(attribName,String(attribValue));}
			template <typename T>
				Node&			SetMore(const String&attribName, const T&attribValue)	{Set(attribName,attribValue); return *this;}
			void				Unset(const String&attrib_name)	{attributes.Unset(attrib_name);}
			Node*          		Find(const String&path);				//!< Returns the first matching node (if any) @param path Path string in the form 'node/subnode/.../subnodeN' @return Pointer to the matching node or NULL if no such could be found
			const Node*			Find(const String&path)		const;	//!< @copydoc Find()
			Node*          		Find(const StringRef&path);				//!< @copydoc Find()
			const Node*			Find(const StringRef&path)		const;	//!< @copydoc Find()
			Node*          		Find(const char*path)				/** @copydoc Find() */ {return Find(StringRef(path));}
			const Node*			Find(const char*path)			const	/** @copydoc find() */ {return Find(StringRef(path));}
			Node&				Create(const String&path, const String&inner_content="");
			/**
			Queries the string content of the specified attribute of the local node.
			The method does not crash if @a this is NULL. 
			@param name Name of the requested attribute
			@param[out] outResult String reference to store the respective attribute's value in
			@return true if the local object is not NULL and the requested attribute exists, false otherwise.
			*/
			bool				Query(const String&name, String&outResult)	const	{return this && attributes.Query(name,outResult);}
			bool				Query(const char*name, String&outResult)	const	/** @copydoc Query()*/ {return this && attributes.Query(name,outResult);}
			/**
			Queries the string content of the specified attribute of the local node.
			The method does not crash if @a this is NULL.
			@param name Name of the requested attribute
			@param[out] outResult String reference to store the respective attribute's value in
			@return true if the local object is not NULL and the requested attribute exists, false otherwise.
			*/
			bool				Query(const String&name, StringRef&outResult)	const	{return this && attributes.Query(name,outResult);}
			bool				Query(const char*name, StringRef&outResult)	const	/** @copydoc Query() */ {return this && attributes.Query(name,outResult);}


			void				swap(Node&other);
			friend void			swap(Node&a, Node&b)	{a.swap(b);}

		private:

			friend class Container;

	                    
			//Node          			*parent;
		};

		class Container
		{
		private:
			StringBuffer		myBuffer;	//avoid multi-threading issues
		public:
			Node           		root_node;
			Encoding			encoding,							//!< Character encoding
								hostEncoding;

			/**/				Container(Encoding hostEncoding=Encoding::UTF8):hostEncoding(hostEncoding)		{root_node.name = "xml"; encoding = Encoding::UTF8; }
			/**/				Container(const PathString&source,Encoding hostEncoding=Encoding::UTF8):hostEncoding(hostEncoding)	{root_node.name = "xml"; encoding = Encoding::UTF8; LoadFromFile(source);}
			void				Clear();													//!< Clears all local data
			Node*				Find(const String&path);				//!< Returns the first matching node (if any) @param path Path string in the form 'node/subnode/.../subnodeN' @return Pointer to the matching node or NULL if no such could be found
			const Node*			Find(const String&path)		const;	//!< @copydoc find()
			Node&				Create(const String&path, const String&inner_content="");

			/**
			@brief Attempts to load the local XML configuration from the specified char field
				
			The specified char field must include the terminating 0 and be mutable. The array length will not be altered but dividing zeroes will be inserted as necessary
			@param field Char array as specified
			*/
			void				LoadFromCharArray(Ctr::ArrayData<char>&field);
			/**
			@brief Attempts to load the local XML configuration from the specified file
			@param filename Name of the file to load
			@param content_out Optional string to store unprocesses file content in. Pass valid string pointer or NULL if no content exporting should be performed
			*/
			void                LoadFromFile(const PathString&filename, String*content_out=NULL);
			/**
			@brief Attempts to load the local XML configuration from the specified content string
			@param content String to parse
			*/
			void				LoadFromString(const String&content);
			void                SaveToFile(const PathString&filename, export_style_t style=Nice) const;
			void				SaveToStringBuffer(StringBuffer&target, export_style_t style=Nice) const;
		};
	
	
		class ScannerRule;
		typedef std::shared_ptr<ScannerRule>	PScannerRule;
		typedef std::weak_ptr<ScannerRule>	WScannerRule;

	
		struct TScannerNode
		{
			AttributeTable			attributes;
			String					content;
			PScannerRule			rule;
		};
	


		class ScannerRule
		{
		public:
			typedef void(*pNodeCallback)(const Vector0<TScannerNode>&node);
		protected:
			StringTable<WScannerRule>
									connections;
			String					name;
			bool					want_content;
			pNodeCallback			on_enter,on_exit;
			friend class Scanner;	//only gcc knows why i need to declare this :S
		public:
			/**/					ScannerRule(const String&name, bool want_content, pNodeCallback on_enter, pNodeCallback on_exit);
			PScannerRule			Map(const String&name, bool want_content=true,pNodeCallback on_enter=NULL, pNodeCallback on_exit=NULL);
			PScannerRule			Map(const PScannerRule&node);
			bool					Unmap(const String&name);
			bool					Unmap(ScannerRule*node);
		};
	
	
		class Scanner
		{
		private:
			ScannerRule::pNodeCallback			except;
			char					buffer[0x1000];
			size_t					buffer_fill_state;
			String					error_string;
			bool					error_is_filenotfound;
			XML::Encoding			hostEncoding;
			PScannerRule			root;

			friend class ScannerRule;
			
			bool					read(char until, StringBuffer&to, FILE*f);
			bool					skip(char until, FILE*f);
			bool					skip(const char*until, FILE*f);
		public:
			/**/					Scanner(Encoding hostEncoding=Encoding::UTF8);
			void					SetExcept(ScannerRule::pNodeCallback callback_on_except=NULL);
			bool					Scan(const PathString&filename);
			const String&			GetError()						const;
			bool					ErrorIsFileNotFound()			const;
			PScannerRule			GetRoot() const {return root;}
		};
	}
}

#endif
