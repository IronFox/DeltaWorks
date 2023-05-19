#include <global_root.h>
#include "segment_map.h"

#ifndef BEGIN
	#define BEGIN
#endif

#ifndef END
	#define END
#endif

namespace DeltaWorks::Fractal
{
	void VertexMap::build()
	{
		BEGIN
		for (unsigned y = 0; y < vertex_range; y++)
			for (unsigned x = 0; x <= y; x++)
			{
				unsigned index = getIndex(x,y);
	
				TMapVertex&inf = vertex_descriptor[index];
	
				inf.x = x;
				inf.y = y;
				inf.borderDirection = -1;
				inf.indexAlongBorder = 0;
				inf.index = index;
				
				inf.grid_vertex = !(x%2) && !(y%2);
				
				if (!x)
				{
					inf.borderDirection = 0;
					inf.indexAlongBorder = y;
				}
				else
					if (y == vertex_range-1)
					{
						inf.borderDirection = 1;
						inf.indexAlongBorder = x;
					}
					else
						if (x == y)
						{
							inf.borderDirection = 2;
							inf.indexAlongBorder = vertex_range-x-1;
						}
	
				if (!(y%2))
					if (!(x%2))
					{
						//grid vertex
						inf.dir = 255;
						inf.parent0 = inf.parent1 = inf.parent2 = inf.parent3 = UNSIGNED_UNDEF;
						switch (inf.borderDirection)
						{
							case -1:
								inf.child[0] = getIndex(x-1,y-1);
								inf.child[1] = getIndex(x-1,y);
								inf.child[2] = getIndex(x,y+1);
								inf.child[3] = getIndex(x+1,y+1);
								inf.child[4] = getIndex(x+1,y);
								inf.child[5] = getIndex(x,y-1);
							break;
							case 0:
								inf.child[0] = getIndexC(x,y+1);
								inf.child[1] = getIndexC(x+1,y+1);
								inf.child[2] = getIndexC(x+1,y);
								inf.child[3] = getIndexC(x,y-1);
								inf.child[4] = UNSIGNED_UNDEF;
								inf.child[5] = UNSIGNED_UNDEF;
							break;
							case 1:
								inf.child[0] = getIndexC(x+1,y);
								inf.child[1] = getIndexC(x,y-1);
								inf.child[2] = getIndexC(x-1,y-1);
								inf.child[3] = getIndexC(x-1,y);
								inf.child[4] = UNSIGNED_UNDEF;
								inf.child[5] = UNSIGNED_UNDEF;
							break;
							case 2:
								inf.child[0] = getIndexC(x-1,y-1);
								inf.child[1] = getIndexC(x-1,y);
								inf.child[2] = getIndexC(x,y+1);
								inf.child[3] = getIndexC(x+1,y+1);
								inf.child[4] = UNSIGNED_UNDEF;
								inf.child[5] = UNSIGNED_UNDEF;
							break;
							default:
								FATAL__("Invalid edge index encountered: "+String((int)inf.borderDirection));
						}
					}
					else
					{
						inf.dir = 1;
						inf.parent0 = getIndex(x-1,y);
						inf.parent1 = getIndex(x+1,y);

						inf.parent2 = getIndex(x-1,y-2);
						if (y+2 < vertex_range)
						{
							inf.sibling[0] = getIndex(x,y+1);
							inf.sibling[1] = getIndex(x+1,y+1);
							inf.parent3 = getIndex(x+1,y+2);
							ASSERT_LESS__(inf.parent3,vertex_count);
						}
						else
						{
							inf.sibling[0] = inf.sibling[1] = UNSIGNED_UNDEF;
							inf.parent3 = UNSIGNED_UNDEF;
						}
						if (y > 0)
						{
							inf.sibling[3] = getIndex(x-1,y-1);
							inf.sibling[2] = getIndex(x,y-1);
						}
						else
							inf.sibling[3] = inf.sibling[2] = UNSIGNED_UNDEF;
					}
				else
					if (!(x%2))
					{
						inf.dir = 0;
						inf.parent0 = getIndex(x,y-1);
						if (y+1 < vertex_range)
						{
							inf.parent1 = getIndex(x,y+1);
							inf.parent2 = getIndex(x+2,y+1);
						}
						else
						{
							inf.parent1 = UNSIGNED_UNDEF;	//this can only happen if reduction is enabled
							inf.parent2 = UNSIGNED_UNDEF;
						}
						if (x)
						{
							inf.sibling[0] = getIndex(x-1,y-1);
							inf.sibling[1] = getIndex(x-1,y);
							inf.parent3 = getIndex(x-2,y-1);
							ASSERT_LESS__(inf.parent3,vertex_count);
						}
						else
						{
							inf.sibling[0] = inf.sibling[1] = UNSIGNED_UNDEF;
							inf.parent3 = UNSIGNED_UNDEF;
						}
	
						if (y+1 < vertex_range)
							inf.sibling[2] = getIndex(x+1,y+1);
						else
							inf.sibling[2] = UNSIGNED_UNDEF;
						if (x < y)
							inf.sibling[3] = getIndex(x+1,y);
						else
							inf.sibling[3] = UNSIGNED_UNDEF;
					}
					else
					{
						inf.dir = 2;
						inf.parent1 = getIndex(x-1,y-1);

						if (y+1 < vertex_range)
						{
							inf.parent0 = getIndex(x+1,y+1);
							inf.parent2 = getIndex(x-1,y+1);
						}
						else
						{
							inf.parent0 = UNSIGNED_UNDEF;	//this can only happen if reduction is enabled
							inf.parent2 = UNSIGNED_UNDEF;
						}
	
						
						if (x != y)
						{
							inf.sibling[0] = getIndex(x+1,y);
							inf.sibling[1] = getIndex(x,y-1);
							inf.parent3 = getIndex(x+1,y-1);
							ASSERT_LESS__(inf.parent3,vertex_count);
						}
						else
						{
							inf.sibling[0] = inf.sibling[1] = UNSIGNED_UNDEF;
							inf.parent3 = UNSIGNED_UNDEF;
						}

						inf.sibling[2] = getIndex(x-1,y);
						if (y+1 < vertex_range)
							inf.sibling[3] = getIndex(x,y+1);
						else
							inf.sibling[3] = UNSIGNED_UNDEF;
					}                    
			}
	
	
	
		
		for (unsigned i = 0; i < vertex_range; i++)
		{
			border_index[0][i] = getIndex(0,i);
			border_index[1][i] = getIndex(i,vertex_range-1);
			border_index[2][vertex_range-i-1]  = getIndex(i,i);
		}
		
		if (child_border_index != nullptr)
			for (unsigned i = 0; i < child_border_vertex_count; i++)
			{
				child_border_index[0][i] = border_index[0][i*2+1];
				child_border_index[1][i] = border_index[1][i*2+1];
				child_border_index[2][i] = border_index[2][i*2+1];
			}
	
		for (unsigned i = 1; i < vertex_range; i++)
		{
			inner_border_index[0][i-1] = getIndex(1,i);
			inner_border_index[1][i-1] = getIndex(i-1,vertex_range-2);
			inner_border_index[2][vertex_range-i-1] = getIndex(i-1,i);
		}
	
	
	
		if (child_vertex_index != nullptr)
		{
			unsigned*current = child_vertex_index;

			for (BYTE k = 0; k < 3; k++)
			{
				unsigned*b = border_index[k];
				for (unsigned i = 1; i < vertex_range; i += 2)
				{
					(*current++) = b[i];
				}
			}

			for (unsigned y = 1; y < vertex_range; y += 2)
			{
				for (unsigned x = 2; x <= y; x += 2)
					(*current++) = getIndex(x, y);
				for (unsigned x = 1; x < y; x += 2)
					(*current++) = getIndex(x, y);
			}
			for (unsigned y = 2; y < vertex_range - 1; y += 2)
				for (unsigned x = 1; x <= y; x += 2)
					(*current++) = getIndex(x, y);

			if (current != child_vertex_index + child_vertex_count)
			{
				ErrMessage("Generator Exception: expected " + IntToStr(child_vertex_count) + " (of " + IntToStr(vertex_count) + " total) generated vertices but got " + IntToStr(current - child_vertex_index));
				FATAL__("expected " + IntToStr(child_vertex_count) + " generated vertices but got " + IntToStr(current - child_vertex_index));
			}
		}
		
		if (inner_vertex_index != nullptr)
		{
			unsigned * current = inner_vertex_index;
			for (unsigned i = 0; i < vertex_count; i++)
				if (vertex_descriptor[i].borderDirection == -1)
					(*current++) = i;
			/*
			for (unsigned y = 1; y < vertex_range; y++)
			{
			for (unsigned x = 2; x <= y; x+=2)
			(*current++) = getIndex(x,y);
			for (unsigned x = 1; x < y; x+=2)
			(*current++) = getIndex(x,y);
			}
			for (unsigned y = 2; y < vertex_range-1; y+=2)
			for (unsigned x = 1; x <=y; x+=2)
			(*current++) = getIndex(x,y);*/

			if (current != inner_vertex_index + inner_vertex_count)
			{
				ErrMessage("Generator Exception: expected " + IntToStr(inner_vertex_count) + " (of " + IntToStr(vertex_count) + " total) generated vertices but got " + IntToStr(current - inner_vertex_index));
				FATAL__("expected " + IntToStr(inner_vertex_count) + " generated vertices but got " + IntToStr(current - inner_vertex_index));
			}
		}
		
		
		if (parent_vertex_info != nullptr)
		{
			TParentInfo*pvi = parent_vertex_info;
			for (unsigned y = 0; y <= vertex_range / 2; y++)
				for (unsigned x = 0; x <= y; x++)
					(pvi++)->index = getIndex(x * 2, y * 2);

			if (pvi != parent_vertex_info + parent_vertex_count)
			{
				ErrMessage("Generator Exception: expected " + IntToStr(parent_vertex_count) + " (of " + IntToStr(vertex_count) + " total) generated vertices but got " + IntToStr(pvi - parent_vertex_info));
				FATAL__("expected " + IntToStr(parent_vertex_count) + " generated vertices but got " + IntToStr(pvi - parent_vertex_info));
			}
			for (BYTE k = 0; k < 3; k++)
			{
				unsigned	yoff = k ? vertex_range / 2 : 0,
					xoff = k == 2 ? vertex_range / 2 : 0;

				pvi = parent_vertex_info;
				for (unsigned y = 0; y <= vertex_range / 2; y++)
					for (unsigned x = 0; x <= y; x++)
						(pvi++)->origin[k] = getIndex(xoff + x, yoff + y);

			}

			pvi = parent_vertex_info;
			for (unsigned y = 0; y <= vertex_range / 2; y++)
				for (unsigned x = 0; x <= y; x++)
					(pvi++)->origin[3] = getIndex(vertex_range / 2 - x, vertex_range - y - 1);
		}
	
		Array<unsigned>	forward_map(vertex_count * 4);
		if (parent_vertex_count > 0)
		{
			for (unsigned i = 0; i < parent_vertex_count; i++)
			{
				ASSERT__(parent_vertex_info[i].index < vertex_count);
				M::Vec::ref4(forward_map + parent_vertex_info[i].index * 4) = M::Vec::ref4(parent_vertex_info[i].origin);
			}

			TParentInfo	*ipvi = inner_parent_vertex_info;
			for (unsigned i = 0; i < parent_vertex_count; i++)
			{
				const TMapVertex&inf = vertex_descriptor[parent_vertex_info[i].index];
				if (inf.borderDirection == -1)
					(*ipvi++) = parent_vertex_info[i];
			}
			if (ipvi != inner_parent_vertex_info + inner_parent_vertex_count)
			{
				ErrMessage("Generator Exception: expected " + IntToStr(inner_parent_vertex_count) + " (of " + IntToStr(vertex_count) + " total) generated vertices but got " + IntToStr(ipvi - inner_parent_vertex_info));
				FATAL__("expected " + IntToStr(inner_parent_vertex_count) + " generated inner parent vertices but got " + IntToStr(ipvi - inner_parent_vertex_info));
			}
		}
		else
			forward_map.Fill(0);

		

		for (unsigned i = 0; i < vertex_count; i++)
		{
			//cout << " "<<i;
			TMapVertex&info = vertex_descriptor[i];
			//cout << i<<" / "<<vertex_count<<endl;
			for (BYTE k = 0; k < 4; k++)
			{
				if (info.grid_vertex)
				{
					info.parent_space[k].match = forward_map[i*4+k];
					info.parent_space[k].parent0 = UNSIGNED_UNDEF;
					info.parent_space[k].parent1 = UNSIGNED_UNDEF;
					info.parent_space[k].parent2 = UNSIGNED_UNDEF;
				}
				else
				{
					ASSERT2__(info.parent0 == UNSIGNED_UNDEF || info.parent0<vertex_count,info.parent0,vertex_count);
					ASSERT2__(info.parent1 == UNSIGNED_UNDEF || info.parent1<vertex_count,info.parent1,vertex_count);
					ASSERT2__(info.parent2 == UNSIGNED_UNDEF || info.parent2<vertex_count,info.parent2,vertex_count);
					info.parent_space[k].match = UNSIGNED_UNDEF;
					info.parent_space[k].parent0 = info.parent0 != UNSIGNED_UNDEF ? forward_map[info.parent0*4+k] : UNSIGNED_UNDEF;
					info.parent_space[k].parent1 = info.parent1 != UNSIGNED_UNDEF ? forward_map[info.parent1*4+k] : UNSIGNED_UNDEF;
					info.parent_space[k].parent2 = info.parent2 != UNSIGNED_UNDEF ? forward_map[info.parent2*4+k] : UNSIGNED_UNDEF;
					if (info.borderDirection == -1 && info.parent3 != UNSIGNED_UNDEF)
					{
						ASSERT2__(info.parent3<vertex_count,info.parent3,vertex_count);
						info.parent_space[k].parent3 = forward_map[info.parent3*4+k];
					}
					else
						info.parent_space[k].parent3 = 0;
				}
			}
		}
	
		
	
	
		/*
		Array<bool>	test_map(vertex_count);
		memset(test_map,false,vertex_count);
		
		for (unsigned i = 0; i < parent_vertex_count; i++)
			if (!test_map[parent_vertex_info[i].index])
				test_map[parent_vertex_info[i].index] = true;
			else
				FATAL__("map test failed for parent element "+IntToStr(i));
	
		for (unsigned i = 0; i < child_vertex_count; i++)
		{
			if (!test_map[child_vertex_index[i]])
				test_map[child_vertex_index[i]] = true;
			else
				FATAL__("map test failed for child element "+IntToStr(i));
			if (vertex_descriptor[child_vertex_index[i]].parent0 == UNSIGNED_UNDEF)
				FATAL__("map test failed for parent0 of child element "+IntToStr(i));
			if (vertex_descriptor[child_vertex_index[i]].parent1 == UNSIGNED_UNDEF)
				FATAL__("map test failed for parent1 of child element "+IntToStr(i));
	
		}
		*/
		
		
		
		END
	}
}
