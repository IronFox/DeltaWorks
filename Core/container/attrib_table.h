#ifndef attrib_tableH
#define attrib_tableH

#include "../global_string.h"
#include "vectorList.h"
#include "hashtable.h"
#include "../string/converter.h"

namespace DeltaWorks
{
	namespace Container
	{
		namespace Attrib
		{

			enum enAttributeType
			{
				UnknownType,
				Boolean,
				Int,
				Float,
				Double,
				StringType,
				BoolType = Boolean,
				IntType = Int,
				FloatType = Float,
				DoubleType = Double
			};
	
			typedef enAttributeType	enType;
	
			enum enAttributeFlag
			{
				UnknownAttribute,
				Copyable=0x1,
				ReadOnly=0x2

			};
	
	

			template <typename T>
				struct TAttributeData
				{
					T				*field;
					unsigned		length;
				};
	
			struct TAttributeConfig
			{
				String			name;
				unsigned		flags;
		
				void			Set(const String&name_, unsigned flags_)
								{
									name = name_;
									flags = flags_;
								}
			};
	
			/**
				@brief Global reference to a singular attribute of any type
			*/
			struct TAttribute
			{
				enAttributeType	type;
				unsigned		field_index,			//!< Location of this attribute in its respective array in the global configuration
								global_id;
				bool			exists,copyable;
			};
	
	
			template <typename T>
				class RootConfiguration
				{
				public:
						Vector<TAttributeData<T> >	attributes;
						Vector<TAttributeConfig>	attribute_names;
						Vector<T>					values;
				
				
						void						clear()
													{
														attributes.reset();
														attribute_names.reset();
														values.reset();
													}
						void						finalize()
													{
														T*offset = values.pointer();
														for (unsigned i = 0; i < attributes.Count(); i++)
														{
															attributes[i].field = offset;
															offset += attributes[i].length;
														}
													}
				};
	
			template <typename T>
				class Storage
				{
				public:
						Ctr::Array<TAttributeData<T> >	attributes;
						Ctr::Array<T>					values;
				
				
						void						adoptData(Storage<T>&other)
													{
														attributes.adoptData(other.attributes);
														values.adoptData(other.values);
													}
						void						read(const RootConfiguration<T>&root_config)
													{
														root_config.values.copyToArray(values);
														root_config.attributes.copyToArray(attributes);
														T*offset = values.pointer();
														for (unsigned i = 0; i < attributes.Count(); i++)
														{
															attributes[i].field = offset;
															offset += attributes[i].length;
														}
													}
				};
	
			class Instance;
	
			template <typename T>
				class StaticTable
				{
				public:
					enum enConfiguration
					{
							GlobalOnly,
							Instantiable,
							Count
					};
					static RootConfiguration<T>	configuration[Count];
			
					static	void					clear()
													{
														for (unsigned i = 0; i < Count; i++)
															configuration[i].clear();
													}
					static	void					finalize()
													{
														for (unsigned i = 0; i < Count; i++)
															configuration[i].finalize();
													}
				};
	
	
			class Table
			{
			protected:
			static	bool							finalized;
			public:
					StringTable<TAttribute>			name_map;
					Vector<TAttribute>				index_map;
			
					void							clear();
			template <typename T>
				static	enAttributeType				typeOf();
			
					template <typename T>
						const TAttribute&			reg(const String&name, unsigned flags, const T& initial_value);
					template <typename T>
						const TAttribute&			reg(const String&name, unsigned flags, const T& initial_value0, const T& initial_value1);
					template <typename T>
						const TAttribute&			reg(const String&name, unsigned flags, const T& initial_value0, const T& initial_value1, const T& initial_value2);
					template <typename T>
						const TAttribute&			reg(const String&name, unsigned flags, const T initial_values[], unsigned length);
				
					static void						finalize();
			
					TAttribute						findAttribute(const String&name)	const;
			
					TAttribute						attribute(unsigned index)	const
													{
														if (index < index_map.GetLength())
															return index_map[index];
														TAttribute empty;
														empty.exists = false;
														return empty;
													}

			static	const TAttributeConfig&			attributeConfig(const TAttribute&attrib);
			static	size_t							attributeLength(const TAttribute&attrib);
			static	size_t							fieldLength(enAttributeType type, bool copyable);
			static	bool							exists(const TAttribute&attrib);
		
			
			template <typename T>
				static	bool						GetChar(const TAttribute&attrib, T*target, count_t len);
			template <typename T>
				static	bool						GetChar(const TAttribute&attrib, T*target);
			template <typename T>
				static	T							GetChar(const TAttribute&attrib);

			static	bool							Set(const TAttribute&attrib, const int*values, count_t len);
			static	bool							Set(const TAttribute&attrib, const bool*values, count_t len);
			static	bool							Set(const TAttribute&attrib, const double*values, count_t len);
			static	bool							Set(const TAttribute&attrib, const float*values, count_t len);
			static	bool							Set(const TAttribute&attrib, const String*values, count_t len);
			static	bool							Set(const TAttribute&attrib, const char*string);
			
			static	void							buildInstance(Instance&target);
			};

			class Instance
			{
			public:
					Storage<bool>					bool_storage;
					Storage<int>					int_storage;
					Storage<float>					float_storage;
					Storage<double>				double_storage;
					Storage<String>				string_storage;
			
					void							adoptData(Instance&other)
													{
														bool_storage.adoptData(other.bool_storage);
														int_storage.adoptData(other.int_storage);
														float_storage.adoptData(other.float_storage);
														double_storage.adoptData(other.double_storage);
														string_storage.adoptData(other.string_storage);
													}
				template <typename T>
					bool							GetChar(const TAttribute&attrib, T*target, unsigned len)	const;
				template <typename T>
					bool							GetChar(const TAttribute&attrib, T*target)	const;
				template <typename T>
					T								GetChar(const TAttribute&attrib)	const;

					bool							Set(const TAttribute&attrib, const int*values, unsigned len);
					bool							Set(const TAttribute&attrib, const double*values, unsigned len);
					bool							Set(const TAttribute&attrib, const float*values, unsigned len);
					bool							Set(const TAttribute&attrib, const String*values, unsigned len);
					bool							Set(const TAttribute&attrib, const char*string);
	
			};
	
		}

		#include "attrib_table.tpl.h"
	}
}

#endif
