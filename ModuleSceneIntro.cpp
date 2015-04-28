#include "Globals.h"
#include "Application.h"
#include "ModuleSceneIntro.h"

// Reference at https://youtu.be/6OlenbCC4WI?t=382

ModuleSceneIntro::ModuleSceneIntro(Application* app, bool start_enabled) : Module(app, start_enabled)
{
	graphics = NULL;
	fx = 0;
}

ModuleSceneIntro::~ModuleSceneIntro()
{}

// Load assets
bool ModuleSceneIntro::Start()
{
	LOG("Loading Intro assets");
	bool ret = true;

	App->renderer->camera.x = App->renderer->camera.y = 0;
	
	App->collision->Enable();
	srand(SDL_GetTicks());

	return ret;
}

// Load assets
bool ModuleSceneIntro::CleanUp()
{
	LOG("Unloading Intro scene");

	App->collision->Disable();

	return true;
}

// Update: draw background
update_status ModuleSceneIntro::Update()
{
	// Add random collider on ENTER
	if(App->input->GetKey(SDL_SCANCODE_RETURN) == KEY_DOWN)
	{
		SDL_Rect r;
		r.x = rand() % App->renderer->camera.w;
		r.y = rand() % App->renderer->camera.h;
		r.w = rand() % App->renderer->camera.w / 4;
		r.h = rand() % App->renderer->camera.h / 4;

		App->collision->AddCollider(r, COLLIDER_ENEMY);
	}

	return UPDATE_CONTINUE;
}