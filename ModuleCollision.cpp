#include "Globals.h"
#include "Application.h"
#include "ModuleCollision.h"

ModuleCollision::ModuleCollision(Application* app, bool start_enabled) : Module(app, start_enabled)
{
	debug = true;

	matrix[COLLIDER_WALL][COLLIDER_WALL] = false;
	matrix[COLLIDER_WALL][COLLIDER_PLAYER] = true;
	matrix[COLLIDER_WALL][COLLIDER_ENEMY] = true;
	matrix[COLLIDER_WALL][COLLIDER_PLAYER_SHOT] = true;
	matrix[COLLIDER_WALL][COLLIDER_ENEMY_SHOT] = true;

	matrix[COLLIDER_PLAYER][COLLIDER_WALL] = true;
	matrix[COLLIDER_PLAYER][COLLIDER_PLAYER] = false;
	matrix[COLLIDER_PLAYER][COLLIDER_ENEMY] = true;
	matrix[COLLIDER_PLAYER][COLLIDER_PLAYER_SHOT] = false;
	matrix[COLLIDER_PLAYER][COLLIDER_ENEMY_SHOT] = true;

	matrix[COLLIDER_ENEMY][COLLIDER_WALL] = true;
	matrix[COLLIDER_ENEMY][COLLIDER_PLAYER] = true;
	matrix[COLLIDER_ENEMY][COLLIDER_ENEMY] = false;
	matrix[COLLIDER_ENEMY][COLLIDER_PLAYER_SHOT] = true;
	matrix[COLLIDER_ENEMY][COLLIDER_ENEMY_SHOT] = false;

	matrix[COLLIDER_PLAYER_SHOT][COLLIDER_WALL] = true;
	matrix[COLLIDER_PLAYER_SHOT][COLLIDER_PLAYER] = false;
	matrix[COLLIDER_PLAYER_SHOT][COLLIDER_ENEMY] = true;
	matrix[COLLIDER_PLAYER_SHOT][COLLIDER_PLAYER_SHOT] = false;
	matrix[COLLIDER_PLAYER_SHOT][COLLIDER_ENEMY_SHOT] = false;

	matrix[COLLIDER_ENEMY_SHOT][COLLIDER_WALL] = true;
	matrix[COLLIDER_ENEMY_SHOT][COLLIDER_PLAYER] = true;
	matrix[COLLIDER_ENEMY_SHOT][COLLIDER_ENEMY] = false;
	matrix[COLLIDER_ENEMY_SHOT][COLLIDER_PLAYER_SHOT] = false;
	matrix[COLLIDER_ENEMY_SHOT][COLLIDER_ENEMY_SHOT] = false;
}

// Destructor
ModuleCollision::~ModuleCollision()
{}

update_status ModuleCollision::PreUpdate()
{
	if(quadtree.root == NULL)
	{
		SDL_Rect c = App->renderer->camera;
		c.x = -c.x;
		quadtree.SetBoundaries(c);
	}

	// Remove all colliders scheduled for deletion & fill quadtree
	p2List_item<Collider*>* tmp = colliders.getFirst();
	p2List_item<Collider*>* tmp2;

	while(tmp != NULL)
	{
		tmp2 = tmp->next;
		if(tmp->data->to_delete == true)
		{
			delete tmp->data;
			colliders.del(tmp);
		}

		tmp = tmp2;
	}

	return UPDATE_CONTINUE;
}

// Called before render is available
update_status ModuleCollision::Update()
{
	int tests = 0;

	p2List_item<Collider*>* tmp = colliders.getFirst();

	Collider* c1;
	Collider* c2;

	while(tmp != NULL)
	{
		c1 = tmp->data;

		// Debug ---
		if(debug)
			DrawDebug(c1);

		p2List_item<Collider*>* tmp2 = tmp->next; // avoid checking collisions already checked
		while(tmp2 != NULL)
		{
			c2 = tmp2->data;

			if(c1->CheckCollision(c2->rect) == true)
			{
				if(matrix[c1->type][c2->type] && c1->callback) 
					c1->callback->OnCollision(c1, c2);
				
				if(matrix[c2->type][c1->type] && c2->callback) 
					c2->callback->OnCollision(c2, c1);
			}
			++tests;

			tmp2 = tmp2->next;
		}

		tmp = tmp->next;
	}

	// Debug ---
	if(App->input->GetKey(SDL_SCANCODE_F1) == KEY_DOWN)
		debug = !debug;

	if(debug)
	{
		// Draw quadtree nodes
		p2DynArray<p2QuadTreeNode*> nodes(25);
		quadtree.CollectRects(nodes);

		for(int i = 0; i < nodes.Count(); ++i)
			App->renderer->DrawQuad(nodes[i]->rect, 255, 255, 255, 255, false);

		// Create a collider around mouse position
		SDL_Rect m;
		m.x = App->input->GetMouseX() - 10;
		m.y = App->input->GetMouseY() - 10;
		m.w = 10 * 2;
		m.h = 10 * 2;
		
		// Draw collision candidates
		p2DynArray<Collider*> cols(25);
		App->renderer->DrawQuad(m, 255, 255, 255, 255, false);
		int t = quadtree.CollectCandidates(cols, m);
		int qtests = 0;

		for(int i = 0; i < cols.Count(); ++i)
		{
			App->renderer->DrawQuad(cols[i]->rect, 0, 255, 0, 255, false);
			for(int j = 1 + 1; j < cols.Count(); ++j)
				++qtests;
		}

		char title[250];
		sprintf_s(title, "Colliders: %d Brute Force: %d Quadtree: (%d+%d) = %d", colliders.count(), tests, t, qtests, t+qtests);
		App->window->SetTitle(title);
	}

	return UPDATE_CONTINUE;
}

void ModuleCollision::DrawDebug(Collider* col)
{
	Uint8 alpha = 80;
	switch(col->type)
	{
		case COLLIDER_NONE:
		App->renderer->DrawQuad(col->rect, 255, 255, 255, alpha);
		break;
		case COLLIDER_WALL:
		App->renderer->DrawQuad(col->rect, 0, 0, 255, alpha);
		break;
		case COLLIDER_PLAYER:
		App->renderer->DrawQuad(col->rect, 0, 255, 0, alpha);
		break;
		case COLLIDER_ENEMY:
		App->renderer->DrawQuad(col->rect, 255, 0, 0, alpha);
		break;
		case COLLIDER_PLAYER_SHOT:
		App->renderer->DrawQuad(col->rect, 255, 255, 0, alpha);
		break;
		case COLLIDER_ENEMY_SHOT:
		App->renderer->DrawQuad(col->rect, 0, 255, 255, alpha);
		break;
	}
}

// Called before quitting
bool ModuleCollision::CleanUp()
{
	LOG("Freeing all colliders");

	p2List_item<Collider*>* item = colliders.getLast();

	while(item != NULL)
	{
		delete item->data;
		item = item->prev;
	}

	colliders.clear();
	return true;
}

Collider* ModuleCollision::AddCollider(SDL_Rect rect, COLLIDER_TYPE type, Module* callback)
{
	Collider* ret = new Collider(rect, type, callback);
	colliders.add(ret);
	quadtree.Insert(ret);
	return ret;
}
