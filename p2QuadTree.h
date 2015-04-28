// ----------------------------------------------------
// Quadtree implementation --
// ----------------------------------------------------

#ifndef __P2QUADTREE_H__
#define __P2QUADTREE_H__

#include "Collider.h"
#include "p2DynArray.h"

#define QUADTREE_MAX_ITEMS 2

// Helper function to check if one rectangle complately contains another
bool Contains(const SDL_Rect& a, const SDL_Rect& b);
bool Intersects(const SDL_Rect& a, const SDL_Rect& b);

// Tree node -------------------------------------------------------
class p2QuadTreeNode
{

public:

	SDL_Rect				rect;
	p2DynArray<Collider*>	objects;
	p2QuadTreeNode*			parent;
	p2QuadTreeNode*			childs[4];

public:

	p2QuadTreeNode(SDL_Rect r) : 
		rect(r),
		objects(QUADTREE_MAX_ITEMS)
	{
		parent = childs[0] = childs[1] = childs[2] = childs[3] = NULL;
	}

	~p2QuadTreeNode()
	{
		for(int i = 0; i < 4; ++i)
			if(childs[i] != NULL) delete childs[i];
	}
	 
	void Insert(Collider* col)
	{
		if (childs[0] == NULL && childs[1] == NULL && childs[2] == NULL && childs[3] == NULL)
		{
			if (objects.Count() >= QUADTREE_MAX_ITEMS)
			{
				Divide();
			}
			else
			{
				objects.PushBack(col);
				return;
			}
		}
		InsertChilds(col);
		return;
	}

	void InsertChilds(Collider* col)
	{
		if (Intersects(childs[0]->rect, col->rect) && Intersects(childs[1]->rect, col->rect) && Intersects(childs[2]->rect, col->rect) && Intersects(childs[3]->rect, col->rect))
		{
			objects.PushBack(col);
		}
		else
		{
			for (int i = 0; i < 4; i++)
			{
				if (Intersects(childs[i]->rect, col->rect))
				{
					childs[i]->Insert(col);
				}
			}
		}
	}

	void Divide()
	{
		SDL_Rect new_node;
		new_node.h = rect.h / 2;
		new_node.w = rect.w / 2;
		new_node.x = rect.x;
		new_node.y = rect.y;

		childs[0] = new p2QuadTreeNode(new_node);
		childs[0]->parent = this;

		new_node.x += new_node.w;
		childs[1] = new p2QuadTreeNode(new_node);
		childs[1]->parent = this;

		new_node.y += new_node.h;
		childs[2] = new p2QuadTreeNode(new_node);
		childs[2]->parent = this;

		new_node.x -= new_node.w;
		childs[3] = new p2QuadTreeNode(new_node);
		childs[3]->parent = this;

		Collider* redistribute[QUADTREE_MAX_ITEMS];
		for (int i = 0; i < objects.Count(); i++)
		{
			redistribute[i] = objects[i];
		}
		objects.Clear();
		for (int i = 0; i < QUADTREE_MAX_ITEMS; i++)
		{
			InsertChilds(redistribute[i]);
		}
	}

	int CollectCandidates(p2DynArray<Collider*>& nodes, const SDL_Rect& r) const
	{
		int ret = 0;
		for (int i = 0; i < objects.Count(); i++)
		{
			nodes.PushBack(objects[i]);
			ret++;
		}
		if (childs[0] != NULL)
		{
			for (int i = 0; i < 4; i++)
			{
				if (Intersects(childs[i]->rect, r))
				{
					ret += childs[i]->CollectCandidates(nodes, r);
				}
			}
		}
		return ret;
	}


	void CollectRects(p2DynArray<p2QuadTreeNode*>& nodes) 
	{
		nodes.PushBack(this);

		for(int i = 0; i < 4; ++i)
			if(childs[i] != NULL) childs[i]->CollectRects(nodes);
	}

};

// Tree class -------------------------------------------------------
class p2QuadTree
{
public:

	// Constructor
	p2QuadTree() : root(NULL)
	{}

	// Destructor
	virtual ~p2QuadTree()
	{
		Clear();
	}

	void SetBoundaries(const SDL_Rect& r)
	{
		if(root != NULL)
			delete root;

		root = new p2QuadTreeNode(r);
	}

	void Insert(Collider* col)
	{
		if(root != NULL)
		{
			if(Intersects(root->rect, col->rect))
				root->Insert(col);
		}
	}

	void Clear()
	{
		if(root != NULL)
		{
			delete root;
			root = NULL;
		}
	}

	int CollectCandidates(p2DynArray<Collider*>& nodes, const SDL_Rect& r) const
	{
		int tests = 1;
		if(root != NULL && Intersects(root->rect, r))
			tests = root->CollectCandidates(nodes, r);
		return tests;
	}

	void CollectRects(p2DynArray<p2QuadTreeNode*>& nodes) const
	{
		if(root != NULL)
			root->CollectRects(nodes);
	}

public:

	p2QuadTreeNode*	root;

};

#endif // __p2QuadTree_H__