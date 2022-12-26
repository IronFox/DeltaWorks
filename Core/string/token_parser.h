#ifndef token_parserH
#define token_parserH
#include "tokenizer.h"
#include "../container/hashtable.h"

namespace DeltaWorks
{

	class TokenParser;


	inline void	helloFunction();

	inline void	helloFunction2(int a, float b);

	inline void	helloFunction3()	throw()
	{

	}

	inline void	helloFunction4()
	{


	};


	template <typename x>
		inline void	helloFunction5()
		{


		};

	template <>
		inline void	helloFunction5<float>()
		{

		};

	namespace Parser
	{
		class Connection;
		class Node;
		typedef std::shared_ptr<Node>	PNode;
		typedef std::weak_ptr<Node>		WNode;


		enum operation_t
		{
			PO_NONE,
			PO_ABORT,
			PO_FINISH
		};
		enum condition_t
		{
			PC_NEVER,
			PC_LEVEL_ZERO
		};
		enum block_type_t
		{
			BT_NONE,
			BT_TOGGLE,
			BT_BLOCK,
			BT_UNBLOCK
		};

		struct Sequence
		{
			String					state;
			index_t					begin,
									end;

									Sequence():begin(InvalidIndex),end(InvalidIndex)
									{}

			Sequence&				setState(const String&new_state)
									{
										state = new_state;
										return *this;
									}
			Sequence&				setRange(index_t begin_, index_t end_)
									{
										begin = begin_;
										end = end_;
										return *this;
									}
			Sequence&				setEnd(index_t end_)
									{
										end = end_;
										return *this;
									}
			count_t					GetLength()	const	{return end - begin;}
			index_t					operator[](index_t element)	const	{ASSERT_LESS__(element,GetLength()); return begin + element;}
			index_t					at(index_t element) const 	{ASSERT_LESS__(element,GetLength()); return begin + element;}

			void					swap(Sequence&other)
									{
										state.swap(other.state);
										swp(begin,other.begin);
										swp(end,other.end);
									}
			friend void				swap(Sequence&a, Sequence&b){a.swap(b);}
		};

		struct Stream : public Ctr::Vector<Sequence,Strategy::Swap>
		{
		public:
			typedef Ctr::Vector<Sequence,Strategy::Swap>	Super;

			typedef Ctr::Vector<index_t>	MapEntry;
			Ctr::StringTable<MapEntry,Strategy::Swap>	sequence_map;	//!< Maps state names to sequences

			count_t			FindAll(const String&name, Vector<Sequence*>&result)
			{
				result.Clear();
				//index_t at;
				MapEntry*entry = sequence_map.QueryPointer(name);
				if (entry)
				{
					foreach (*entry,item)
						result << &Super::at(*item);
					return result.Count();
				}
				return 0;
			}
			
			Sequence*			FindAny(const String&name)
			{
				MapEntry*entry = sequence_map.QueryPointer(name);
				if (entry && entry->Count())
					return &Super::at(entry->First());
				return nullptr;
			}
			

			void				swap(Stream&other)
								{
									Super::swap(other);
									sequence_map.swap(other.sequence_map);
								}
			friend void			swap(Stream&a, Stream&b){a.swap(b);}
			void				adoptData(Stream&other)
								{
									Super::adoptData(other);
									sequence_map.adoptData(other.sequence_map);
								}
		};
		//typedef Buffer<Sequence,0,Strategy::Swap> 	Stream;


		class Connection
		{
		public:
			unsigned			ident;
			unsigned			block_ident;
			char				level_change;
			block_type_t		block_change;
			WNode				target;

								Connection();
			Connection*			increaseLevel();
			Connection*			decreaseLevel();
			Connection*			block(unsigned ident);
			Connection*			unblock(unsigned ident);
		};


		class Node : public std::enable_shared_from_this<Node>
		{
		public:
			Ctr::IndexTable<Connection>	connections;
			Connection				otherwise,
									conditioned;
			TokenParser				*super;
			operation_t				operation;
			condition_t				condition;
			String					name;

									Node();
			void					Init();
			friend void				swap(Node&a, Node&b){a.swap(b);}
			void					swap(Node&other)
			{
				connections.swap(other.connections);
				swp(otherwise,other.otherwise);
				swp(conditioned,other.conditioned);
				swp(super,other.super);
				swp(operation,other.operation);
				swp(condition,other.condition);
				name.swap(other.name);
			}

		};



		typedef Ctr::Vector<Stream,Strategy::Swap>		Result;
	}


	class TokenParser
	{
	private:
		Ctr::StringTable<Parser::PNode>				nodes;
		Ctr::Vector<Parser::PNode>					detour_nodes;
		//Buffer<Parser::Connection,0>				connections;
		Ctr::IndexTable<Parser::Connection>			root;
		
		Ctr::Array<Parser::PNode>						node_export;


		friend class Parser::Node;
	public:
		Parser::PNode			CreateStatus(const String&name, Parser::operation_t op=Parser::PO_NONE);
		Parser::Connection&		Define(const String&name, unsigned ident0);
		void					Define(const String&name, unsigned ident0, unsigned ident1);
		void					Define(const String&name, unsigned ident0, unsigned ident1, unsigned ident2);
		void					Define(const String&name, unsigned ident0, unsigned ident1, unsigned ident2, unsigned ident3);
		Parser::Connection&		Route(const String&name_from, const String&name_to);
		Parser::Connection&		Route(const String&name_from, unsigned ident0, const String&name_to);
		void					Route(const String&name_from, unsigned ident0, unsigned ident1, const String&name_to);
		void					Route(const String&name_from, unsigned ident0, unsigned ident1, unsigned ident2, const String&name_to);
		void					Route(const String&name_from, unsigned ident0, unsigned ident1, unsigned ident2, unsigned ident3, const String&name_to);
		Parser::Connection&		ConditionedRoute(const String&name_from, const String&name_to, Parser::condition_t condition);
		void					GlobalBlock(unsigned ident, unsigned block_ident, Parser::block_type_t type);
		void					GlobalDetour(unsigned begin_symbol, unsigned end_symbol, const String&state_name);
		String					StateToStr(const String&name);

		void					ParseTokenSequence(const TokenList&tokens, Parser::Result&out, index_t begin = 0, index_t end = InvalidIndex)	const;
	};







}

#endif
