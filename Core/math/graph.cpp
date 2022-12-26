#include "../global_root.h"
#include "graph.h"

namespace DeltaWorks
{

	namespace Math
	{
		namespace GraphDetails
		{
			Node::Node():isSelected(false),inDegree(0),dfsIn(0),dfsOut(0)
			{}

			bool	Node::OutboundConnectedTo(const PEdge&e) const
			{
				return outEdges.Contains(e);
			}

			bool	Node::InboundConnectedTo(const PEdge&e) const
			{
				return inEdges.Contains(e);
			}

			bool	Node::InboundConnectedTo(const PNode&other) const
			{
				return in.Contains(other);
			}

			bool	Node::OutboundConnectedTo(const PNode&other) const
			{
				return out.Contains(other);
			}

			index_t	Node::Scan(index_t in)
			{
				isSelected = true;
				dfsIn = in;
				foreach (out,node)
				{
					PNode n = node->lock();
					if (!n->isSelected)
						in = n->Scan(in+1);
				}
				dfsOut = in+1;
				return dfsOut;
			}


			void Edge::LinkTo(const PNode&from_, const PNode&to_)
			{
				ASSERT__(!from);
				ASSERT__(!to);
				to = to_;
				from = from_;
				to->in.Append(from);
				from->out.Append(to);
				to->inEdges.Append(shared_from_this());
				from->outEdges.Append(shared_from_this());
			}

		}

		Graph::PNode		Graph::AddNode()
		{
			PNode rs(new GraphDetails::Node{});
			nodes << rs;

			return rs;
		}

		void				Graph::EraseNode(const PNode&n)
		{
			if (!n)
				return;
			MarkErased(n);
			Unlink(n);

			VerifyIntegrity();
		}



		void				Graph::MarkErased(const PNode&n)
		{
			nodes.Erase(n);
		}

		void				Graph::MarkErased(const PEdge&e)
		{
			edges.Erase(e);
		}


		void				Graph::Unlink(const GraphDetails::PNode&node)
		{
			auto&n = *node;
			foreach (n.in,wn)
			{
				PNode other = wn->lock();
				ASSERT__(other);
				ASSERT__(other->out.FindAndErase(node));
			}
			foreach (n.out,wn)
			{
				PNode other = wn->lock();
				ASSERT__(other);
				ASSERT__(other->in.FindAndErase(node));
			}
			foreach (n.inEdges,we)
			{
				PEdge e = we->lock();
				ASSERT__(e);
				ASSERT__(e->from->outEdges.FindAndErase(e));
				MarkErased(e);
			}
			foreach (n.outEdges,we)
			{
				PEdge e = we->lock();
				ASSERT__(e);
				ASSERT__(e->to->inEdges.FindAndErase(e));
				MarkErased(e);
			}

			n.in.Clear();
			n.out.Clear();
			n.inEdges.Clear();
			n.outEdges.Clear();
		}

		void						Graph::CheckValidity(const TCodeLocation&loc, const PNode&n)	const
		{
			nodes.CheckValidity(loc,n);
		}

		void						Graph::CheckValidity(const TCodeLocation&loc, const PEdge&e)	const
		{
			edges.CheckValidity(loc,e);
		}

		Graph::PNode				Graph::MergeNodes(const PNode&a, const PNode&b)
		{
			if (a == b)
				return a;
			CheckValidity(CLOCATION, a);
			CheckValidity(CLOCATION, b);

			PNode rs = AddNode();


			foreach (a->inEdges,in)
			{
				PEdge ein = in->lock();
				if (ein->from != b)
					AddEdge(ein->from,rs);
			}
			foreach (b->inEdges,in)
			{
				PEdge ein = in->lock();
				if (ein->from != a)
					AddEdge(ein->from,rs);
			}
			foreach (a->outEdges,out)
			{
				PEdge eout = out->lock();
				if (eout->to != b)
					AddEdge(rs,eout->to);
			}
			foreach (b->outEdges,out)
			{
				PEdge eout = out->lock();
				if (eout->to != a)
					AddEdge(rs,eout->to);
			}

			MarkErased(a);
			Unlink(a);
			MarkErased(b);
			Unlink(b);

			//VerifyIntegrity();
			return rs;
		}

		count_t				Graph::CountOccurrences(const PNode&n)	const
		{
			return nodes.CountOccurrences(n);
		}

		count_t				Graph::CountOccurrences(const PEdge&e)	const
		{
			return edges.CountOccurrences(e);
		}


		void				Graph::VerifyIntegrity(bool thoroughly) const
		{
			for (index_t i = 0; i < edges.Count(); i++)
			{
				const auto&e = edges[i];
				if (!e)
					continue;
				ASSERT__(e->index == i);
				CheckValidity(CLOCATION, e->to);
				CheckValidity(CLOCATION, e->from);
				ASSERT__(e->to != e->from);

				ASSERT__(e->to->inEdges.Contains(e));
				ASSERT__(e->to->InboundConnectedTo(e->from));
				ASSERT__(e->from->outEdges.Contains(e));
				ASSERT__(e->from->OutboundConnectedTo(e->to));

				if (thoroughly)
					ASSERT__(CountOccurrences(e) == 1);
			}


			for (index_t i = 0; i < nodes.Count(); i++)
			{
				const auto&n = nodes[i];
				if (!n)
					continue;
				ASSERT__(n->index == i);

				ASSERT__(n->in.Count() == n->inEdges.Count());
				ASSERT__(n->out.Count() == n->outEdges.Count());

				foreach (n->in,win)
				{
					auto in = win->lock();
					CheckValidity(CLOCATION,in);
					ASSERT__(in != n);
					ASSERT__(in->OutboundConnectedTo(n));
				}
				foreach (n->out,wout)
				{
					auto out = wout->lock();
					CheckValidity(CLOCATION,out);
					ASSERT__(out != n);
					ASSERT__(out->InboundConnectedTo(n));
				}
				foreach (n->inEdges,wei)
					CheckValidity(CLOCATION,wei->lock());
				foreach (n->outEdges,weo)
					CheckValidity(CLOCATION,weo->lock());


				if (thoroughly)
					ASSERT__(CountOccurrences(n) == 1);
			}
		}



		GraphDetails::PEdge		Graph::AddEdge(const PNode&node0, const PNode&node1)
		{
			CheckValidity(CLOCATION,node0);
			CheckValidity(CLOCATION,node1);
			ASSERT__(node0 != node1);

			foreach (node0->outEdges,e)
			{
				auto edge = e->lock();
				if (edge->to == node1)
					return edge;
			}
			foreach (node1->outEdges,e)
			{
				auto edge = e->lock();
				if (edge->to == node0)
					return edge;
			}

			PEdge edge(new GraphDetails::Edge());
			edge->LinkTo(node0,node1);
			edges << edge;

		//	VerifyIntegrity();

			return edge;
		}
	
		bool				Graph::CreateTopologicalOrder(Container::BasicVector<PNode>&out)
		{
			Container::Queue<PNode>	queue;
			foreach (nodes,n)
			{
				if (!(*n))
					continue;
				(*n)->inDegree = (int)(*n)->in.Count();
				if (!(*n)->inDegree)
					queue.Push(*n);
			}
    
			out.clear();
			while (queue.IsNotEmpty())
			{
				PNode node = queue.Pop();
				if (node->inDegree > 0)
					continue;
				//node->inDegree--;
				out.append(node);
				foreach (node->out,out)
				{
					PNode n = out->lock();
					if (!--n->inDegree)
						queue.Push(n);
				}
			}
			return out.Count() == nodes.Count();
		}
		void				Graph::PerformDepthFirstSearch()
		{
			count_t cnt(0);
			foreach (nodes,n)
				if ((*n))
					(*n)->isSelected = false;
			foreach (nodes,n)
				if ((*n) && (*n)->in.IsEmpty())
					cnt = (*n)->Scan(cnt+1);
		};

	}

}