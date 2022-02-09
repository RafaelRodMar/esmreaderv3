#include<SDL.h>
#include<sdl_ttf.h>
#include <iostream>
#include <algorithm>
#include <vector>
#include <list>
#include <cmath>
#include <fstream>
#include <sstream>
#include <string>
#include <map>
#include <time.h>
#include "esmreader.h"
#include "game.h"
#include "json.hpp"
#include <chrono>
#include <random>

#include "esmreader.cpp"
#include "visualizers.cpp"

class Rnd {
public:
	std::mt19937 rng;

	Rnd()
	{
		std::mt19937 prng(std::chrono::steady_clock::now().time_since_epoch().count());
		rng = prng;
	}

	int getRndInt(int min, int max)
	{
		std::uniform_int_distribution<int> distribution(min, max);
		return distribution(rng);
	}

	double getRndDouble(double min, double max)
	{
		std::uniform_real_distribution<double> distribution(min, max);
		return distribution(rng);
	}
} rnd;

//Timer control
#define Now() chrono::high_resolution_clock::now()
struct Stopwatch {
	chrono::high_resolution_clock::time_point c_time, c_timeout;
	void Start(uint64_t us) { c_time = Now(); c_timeout = c_time + chrono::microseconds(us); }
	void setTimeout(uint64_t us) { c_timeout = c_time + chrono::microseconds(us); }
	inline bool Timeout() { return Now() > c_timeout; }
	long long EllapsedMilliseconds() { return chrono::duration_cast<chrono::milliseconds>(Now() - c_time).count(); }
} stopwatch;

//la clase juego:
Game* Game::s_pInstance = 0;

Game::Game()
{
	m_pRenderer = NULL;
	m_pWindow = NULL;
}

Game::~Game()
{

}

SDL_Window* g_pWindow = 0;
SDL_Renderer* g_pRenderer = 0;

bool Game::init(const char* title, int xpos, int ypos, int width,
	int height, bool fullscreen)
{
	// almacenar el alto y ancho del juego.
	m_gameWidth = width;
	m_gameHeight = height;

	// attempt to initialize SDL
	if (SDL_Init(SDL_INIT_EVERYTHING) == 0)
	{
		int flags = 0;
		if (fullscreen)
		{
			flags = SDL_WINDOW_FULLSCREEN;
		}

		std::cout << "SDL init success\n";
		// init the window
		m_pWindow = SDL_CreateWindow(title, xpos, ypos,
			width, height, flags);
		if (m_pWindow != 0) // window init success
		{
			std::cout << "window creation success\n";
			m_pRenderer = SDL_CreateRenderer(m_pWindow, -1, 0);
			if (m_pRenderer != 0) // renderer init success
			{
				std::cout << "renderer creation success\n";
				SDL_SetRenderDrawColor(m_pRenderer,
					255, 255, 255, 255);
			}
			else
			{
				std::cout << "renderer init fail\n";
				return false; // renderer init fail
			}
		}
		else
		{
			std::cout << "window init fail\n";
			return false; // window init fail
		}
	}
	else
	{
		std::cout << "SDL init fail\n";
		return false; // SDL init fail
	}
	if (TTF_Init() == 0)
	{
		std::cout << "sdl font initialization success\n";
	}
	else
	{
		std::cout << "sdl font init fail\n";
		return false;
	}

	std::cout << "init success\n";
	m_bRunning = true; // everything inited successfully, start the main loop

	//Joysticks
	InputHandler::Instance()->initialiseJoysticks();

	//load images, sounds, music and fonts
	//AssetsManager::Instance()->loadAssets();
	AssetsManager::Instance()->loadAssetsJson(); //ahora con formato json
	Mix_Volume(-1, 16); //adjust sound/music volume for all channels

	/*p = new player();
	p->settings("chicken", Vector2D(4, 175), Vector2D(0,0), 58, 58, 0, 0, 0, 0.0, 0);
	entities.push_back(p);*/

	//car creation
	/*c1 = new car();
	c2 = new car();
	c3 = new car();
	c4 = new car();
	c1->settings("car1", Vector2D(70, 0), Vector2D(0, 1), 74, 126, 0, 0, 0, 0.0, 0);
	c2->settings("car2", Vector2D(160, 0), Vector2D(0, 2), 56, 126, 0, 0, 0, 0.0, 0);
	c3->settings("car3", Vector2D(239, 400), Vector2D(0, -1), 73, 109, 0, 0, 0, 0.0, 0);
	c4->settings("car4", Vector2D(329, 400), Vector2D(0, -3), 56, 126, 0, 0, 0, 0.0, 0);
	entities.push_back(c1);
	entities.push_back(c2);
	entities.push_back(c3);
	entities.push_back(c4);*/

	/* b1 = new Button();
	b2 = new Button();
	b1->buttonSettings("button", Vector2D(0,0), Vector2D(0,0), 100,50, 0,0,0,0.0,0, "botón 1", "font", true);
	b2->buttonSettings("button", Vector2D(200,200), Vector2D(0,0), 100,50, 0,0,0,0.0,0, "botón 2 extendido", "font", true);
	entities.push_back(b1);
	entities.push_back(b2); */

	std::vector<std::string> themes = {"NPC", "Creature", "Leveled Creature", "Spellmaking", "Enchanting", "Alchemy", "Leveled Item", "Activator", "Apparatus", "Armor", 
							"Body Part", "Book", "Clothing", "Container", "Door", "Ingredient", "Light", "Lockpick", "Misc Item", "Probe", "Repair Item", "Static", "Weapon",
							"Cell", "Game Settings", "Global", "Class", "Faction", "Race", "Sound", "Skill", "Magic Effects", 
							"Script", "Region", "Birthsign", "Landscape Texture", "Landscape", "Path Grid", 
							"Sound Generator", "Spell", "Dialog"};

	int themex = 0;
	int themey = 0;
	int incry = 0;
	Button* b;
	for(int i=0;i<themes.size();i++){
		b = new Button();

		b->buttonSettings("button", Vector2D(themex,themey), Vector2D(0,0), 100,50, 0,0,0,0.0,0, themes[i], "font", true);
		themex += b->m_width + 5;
		if(themex > 520)
		{
			themex = 0;
			themey += b->m_height + 5;
		}

		entities.push_back(b);
	}

	showControl = new ShowControl;
	showControl->settings("showControl", Vector2D(0, 150), Vector2D(0, 0), 1, 1, 0, 0, 0, 0.0, 0);
	entities.push_back(showControl);
	
	state = GAME;

	//crear el archivo json
	/*nlohmann::json j;

	j["fnt"]["font"] = "sansation.ttf";
	j["ico"]["lchicken"] = "henway.ico";
	j["ico"]["lhead"] = "henway_sm.ico";
	j["img"]["car1"] = "car1.png";
	j["img"]["car2"] = "car2.png";
	j["img"]["car3"] = "car3.png";
	j["img"]["car4"] = "car4.png";
	j["img"]["chicken"] = "chicken.png";
	j["img"]["chickenhead"] = "chickenhead.png";
	j["img"]["highway"] = "highway.png";
	j["mus"]["music"] = "music.ogg";
	j["snd"]["bok"] = "bok.wav";
	j["snd"]["carhorn1"] = "carhorn1.wav";
	j["snd"]["carhorn2"] = "carhorn2.wav";
	j["snd"]["celebrate"] = "celebrate.wav";
	j["snd"]["gameover"] = "gameover.wav";
	j["snd"]["squish"] = "squish.wav";

	std::ofstream o("assets.json");
	o << std::setw(4) << j << std::endl;*/

	Stopwatch st;
	st.Start(0);
	readESM("c:/JuegosEstudio/Morrowind/Data Files/morrowind.esm");
	std::cout << "Time file read: " << st.EllapsedMilliseconds() << std::endl;
	//68759 tiempo tienda

	return true;
}

void Game::render()
{
	SDL_SetRenderDrawColor(Game::Instance()->getRenderer(), 255, 255, 255, 0);
	SDL_RenderClear(m_pRenderer); // clear the renderer to the draw color
	SDL_SetRenderDrawColor(Game::Instance()->getRenderer(), 0, 0, 0, 0);

	//AssetsManager::Instance()->draw("highway", 0, 0, 465, 400, m_pRenderer, SDL_FLIP_NONE);

		if (state == MENU)
		{
			//AssetsManager::Instance()->Text("Menu , press S to play", "font", 100, 100, SDL_Color({ 0,0,0,0 }), getRenderer());

			////Show hi scores
			/*int y = 350;
			AssetsManager::Instance()->Text("HiScores", "font", 580 - 50, y, SDL_Color({ 255,255,255,0 }), getRenderer());
			for (int i = 0; i < vhiscores.size(); i++) {
				y += 30;
				AssetsManager::Instance()->Text(std::to_string(vhiscores[i]), "font", 580, y, SDL_Color({ 255,255,255,0 }), getRenderer());
			}*/
		}

		if (state == GAME)
		{
			for (auto i : entities)
				i->draw();

			if (Game::Instance()->lastButtonClicked == "NPC")
			{
				AssetsManager::Instance()->Text("NPC", "font", 5, 150, SDL_Color({ 0,0,0,0 }), getRenderer());
				if (showControl->tag != "NPC")
				{
					showControl->reset();
					
					showControl->tag = "NPC";
				}
			}

			if (Game::Instance()->lastButtonClicked == "Creature")
			{
				AssetsManager::Instance()->Text("Creature", "font", 5, 150, SDL_Color({ 0,0,0,0 }), getRenderer());
				if (showControl->tag != "Creature")
				{
					showControl->reset();

					showControl->tag = "Creature";
				}
			}

			if (Game::Instance()->lastButtonClicked == "Leveled Creature")
			{
				AssetsManager::Instance()->Text("Leveled Creature", "font", 5, 150, SDL_Color({ 0,0,0,0 }), getRenderer());
				if (showControl->tag != "Leveled Creature")
				{
					showControl->reset();

					showControl->tag = "Leveled Creature";
				}
			}

			if (Game::Instance()->lastButtonClicked == "Spellmaking")
			{
				AssetsManager::Instance()->Text("Spellmaking", "font", 5, 150, SDL_Color({ 0,0,0,0 }), getRenderer());
				if (showControl->tag != "Spellmaking")
				{
					showControl->reset();

					showControl->tag = "Spellmaking";
				}
			}

			if (Game::Instance()->lastButtonClicked == "Enchanting")
			{
				AssetsManager::Instance()->Text("Enchanting", "font", 5, 150, SDL_Color({ 0,0,0,0 }), getRenderer());
				if (showControl->tag != "Enchanting")
				{
					showControl->reset();

					showControl->tag = "Enchanting";
				}
			}

			if (Game::Instance()->lastButtonClicked == "Alchemy")
			{
				AssetsManager::Instance()->Text("Alchemy", "font", 5, 150, SDL_Color({ 0,0,0,0 }), getRenderer());
				if (showControl->tag != "Alchemy")
				{
					showControl->reset();

					showControl->tag = "Alchemy";
				}
			}

			if (Game::Instance()->lastButtonClicked == "Leveled Item")
			{
				AssetsManager::Instance()->Text("Leveled Item", "font", 5, 150, SDL_Color({ 0,0,0,0 }), getRenderer());
				if (showControl->tag != "Leveled Item")
				{
					showControl->reset();

					showControl->tag = "Leveled Item";
				}
			}

			if (Game::Instance()->lastButtonClicked == "Activator")
			{
				AssetsManager::Instance()->Text("Activator", "font", 5, 150, SDL_Color({ 0,0,0,0 }), getRenderer());
				if (showControl->tag != "Activator")
				{
					showControl->reset();

					showControl->tag = "Activator";
				}
			}

			if (Game::Instance()->lastButtonClicked == "Apparatus")
			{
				AssetsManager::Instance()->Text("Apparatus", "font", 5, 150, SDL_Color({ 0,0,0,0 }), getRenderer());
				if (showControl->tag != "Apparatus")
				{
					showControl->reset();

					showControl->tag = "Apparatus";
				}
			}

			if (Game::Instance()->lastButtonClicked == "Armor")
			{
				//AssetsManager::Instance()->Text("Armor", "font", 5, 150, SDL_Color({ 0,0,0,0 }), getRenderer());
				if (showControl->tag != "Armor")
				{
					showControl->reset();

					//pass headers to the showControl
					std::vector< std::string > h = { "Name", "Full name", "Model", "Type", "Weight", "Value", "Health", "Enchantment Points", "Armour", "Icon Filename", "Body Part-Male Armor name-Female Armor name", "Enchantment", "Script" };
					showControl->setHeaders(h);

					//pass data to the showControl
					std::vector< std::vector< std::string > > d;
					for (auto x : varmo) {
						std::vector< std::string > temp;
						temp.push_back(x.name);
						temp.push_back(x.fullName);
						temp.push_back(x.model);
						temp.push_back(to_string(x.ad.type));
						temp.push_back(to_string(x.ad.weight));
						temp.push_back(to_string(x.ad.value));
						temp.push_back(to_string(x.ad.health));
						temp.push_back(to_string(x.ad.enchantPts));
						temp.push_back(to_string(x.ad.armour));
						temp.push_back(x.icon);
						std::string tempText = "";
						for (int j = 0; j < x.bp.size(); j++) {
							tempText += to_string(x.bp[j].bodyPartIndex) + "-" + x.bp[j].maleArmorName + "-" + x.bp[j].femaleArmorName;
							if (j != x.bp.size() - 1) tempText += ", ";
						}
						temp.push_back(tempText);
						temp.push_back(x.enchantment);
						temp.push_back(x.script);

						d.push_back(temp);
					}
					showControl->setData(d);

					showControl->tag = "Armor";
				}
			}

			if (Game::Instance()->lastButtonClicked == "Body Part")
			{
				AssetsManager::Instance()->Text("Body Part", "font", 5, 150, SDL_Color({ 0,0,0,0 }), getRenderer());
				if (showControl->tag != "Body Part")
				{
					showControl->reset();

					showControl->tag = "Body Part";
				}
			}

			if (Game::Instance()->lastButtonClicked == "Book")
			{
				AssetsManager::Instance()->Text("Book", "font", 5, 150, SDL_Color({ 0,0,0,0 }), getRenderer());
				if (showControl->tag != "Book")
				{
					showControl->reset();

					showControl->tag = "Book";
				}
			}

			if (Game::Instance()->lastButtonClicked == "Clothing")
			{
				AssetsManager::Instance()->Text("Clothing", "font", 5, 150, SDL_Color({ 0,0,0,0 }), getRenderer());
				if (showControl->tag != "Clothing")
				{
					showControl->reset();

					showControl->tag = "Clothing";
				}
			}

			if (Game::Instance()->lastButtonClicked == "Container")
			{
				AssetsManager::Instance()->Text("Container", "font", 5, 150, SDL_Color({ 0,0,0,0 }), getRenderer());
				if (showControl->tag != "Container")
				{
					showControl->reset();

					showControl->tag = "Container";
				}
			}

			if (Game::Instance()->lastButtonClicked == "Door")
			{
				AssetsManager::Instance()->Text("Door", "font", 5, 150, SDL_Color({ 0,0,0,0 }), getRenderer());
				if (showControl->tag != "Door")
				{
					showControl->reset();

					showControl->tag = "Door";
				}
			}

			if (Game::Instance()->lastButtonClicked == "Ingredient")
			{
				AssetsManager::Instance()->Text("Ingredient", "font", 5, 150, SDL_Color({ 0,0,0,0 }), getRenderer());
				if (showControl->tag != "Ingredient")
				{
					showControl->reset();

					showControl->tag = "Ingredient";
				}
			}

			if (Game::Instance()->lastButtonClicked == "Light")
			{
				AssetsManager::Instance()->Text("Light", "font", 5, 150, SDL_Color({ 0,0,0,0 }), getRenderer());
				if (showControl->tag != "Light")
				{
					showControl->reset();

					showControl->tag = "Light";
				}
			}

			if (Game::Instance()->lastButtonClicked == "Lockpick")
			{
				AssetsManager::Instance()->Text("Lockpick", "font", 5, 150, SDL_Color({ 0,0,0,0 }), getRenderer());
				if (showControl->tag != "Lockpick")
				{
					showControl->reset();

					showControl->tag = "Lockpick";
				}
			}

			if (Game::Instance()->lastButtonClicked == "Misc Item")
			{
				AssetsManager::Instance()->Text("Misc Item", "font", 5, 150, SDL_Color({ 0,0,0,0 }), getRenderer());
				if (showControl->tag != "Misc Item")
				{
					showControl->reset();

					showControl->tag = "Misc Item";
				}
			}

			if (Game::Instance()->lastButtonClicked == "Probe")
			{
				AssetsManager::Instance()->Text("Probe", "font", 5, 150, SDL_Color({ 0,0,0,0 }), getRenderer());
				if (showControl->tag != "Probe")
				{
					showControl->reset();

					showControl->tag = "Probe";
				}
			}

			if (Game::Instance()->lastButtonClicked == "Repair Item")
			{
				AssetsManager::Instance()->Text("Repair Item", "font", 5, 150, SDL_Color({ 0,0,0,0 }), getRenderer());
				if (showControl->tag != "Repair Item")
				{
					showControl->reset();

					showControl->tag = "Repair Item";
				}
			}

			if (Game::Instance()->lastButtonClicked == "Static")
			{
				AssetsManager::Instance()->Text("Static", "font", 5, 150, SDL_Color({ 0,0,0,0 }), getRenderer());
				if (showControl->tag != "Static")
				{
					showControl->reset();

					showControl->tag = "Static";
				}
			}

			if (Game::Instance()->lastButtonClicked == "Weapon")
			{
				AssetsManager::Instance()->Text("Weapon", "font", 5, 150, SDL_Color({ 0,0,0,0 }), getRenderer());
				if (showControl->tag != "Weapon")
				{
					showControl->reset();

					showControl->tag = "Weapon";
				}
			}

			if (Game::Instance()->lastButtonClicked == "Cell")
			{
				AssetsManager::Instance()->Text("Cell", "font", 5, 150, SDL_Color({ 0,0,0,0 }), getRenderer());
				if (showControl->tag != "Cell")
				{
					showControl->reset();

					showControl->tag = "Cell";
				}
			}

			if (Game::Instance()->lastButtonClicked == "Game Settings")
			{
				AssetsManager::Instance()->Text("Game Settings", "font", 5, 150, SDL_Color({ 0,0,0,0 }), getRenderer());
				if (showControl->tag != "Game Settings")
				{
					showControl->reset();

					showControl->tag = "Game Settings";
				}
			}

			if (Game::Instance()->lastButtonClicked == "Global")
			{
				AssetsManager::Instance()->Text("Global", "font", 5, 150, SDL_Color({ 0,0,0,0 }), getRenderer());
				if (showControl->tag != "Global")
				{
					showControl->reset();

					showControl->tag = "Global";
				}
			}

			if (Game::Instance()->lastButtonClicked == "Class")
			{
				AssetsManager::Instance()->Text("Class", "font", 5, 150, SDL_Color({ 0,0,0,0 }), getRenderer());
				if (showControl->tag != "Class")
				{
					showControl->reset();

					showControl->tag = "Class";
				}
			}

			if (Game::Instance()->lastButtonClicked == "Faction")
			{
				AssetsManager::Instance()->Text("Faction", "font", 5, 150, SDL_Color({ 0,0,0,0 }), getRenderer());
				if (showControl->tag != "Faction")
				{
					showControl->reset();

					showControl->tag = "Faction";
				}
			}

			if (Game::Instance()->lastButtonClicked == "Race")
			{
				AssetsManager::Instance()->Text("Race", "font", 5, 150, SDL_Color({ 0,0,0,0 }), getRenderer());
				if (showControl->tag != "Race")
				{
					showControl->reset();

					showControl->tag = "Race";
				}
			}

			if (Game::Instance()->lastButtonClicked == "Sound")
			{
				AssetsManager::Instance()->Text("Sound", "font", 5, 150, SDL_Color({ 0,0,0,0 }), getRenderer());
				if (showControl->tag != "Sound")
				{
					showControl->reset();

					showControl->tag = "Sound";
				}
			}

			if (Game::Instance()->lastButtonClicked == "Skill")
			{
				AssetsManager::Instance()->Text("Skill", "font", 5, 150, SDL_Color({ 0,0,0,0 }), getRenderer());
				if (showControl->tag != "Skill")
				{
					showControl->reset();

					showControl->tag = "Skill";
				}
			}

			if (Game::Instance()->lastButtonClicked == "Magic Effects")
			{
				AssetsManager::Instance()->Text("Magic Effects", "font", 5, 150, SDL_Color({ 0,0,0,0 }), getRenderer());
				if (showControl->tag != "Magic Effects")
				{
					showControl->reset();

					showControl->tag = "Magic Effects";
				}
			}

			if (Game::Instance()->lastButtonClicked == "Script")
			{
				AssetsManager::Instance()->Text("Script", "font", 5, 150, SDL_Color({ 0,0,0,0 }), getRenderer());
				if (showControl->tag != "Script")
				{
					showControl->reset();

					showControl->tag = "Script";
				}
			}

			if (Game::Instance()->lastButtonClicked == "Region")
			{
				AssetsManager::Instance()->Text("Region", "font", 5, 150, SDL_Color({ 0,0,0,0 }), getRenderer());
				if (showControl->tag != "Region")
				{
					showControl->reset();

					showControl->tag = "Region";
				}
			}

			if (Game::Instance()->lastButtonClicked == "Birthsign")
			{
				//AssetsManager::Instance()->Text("Birthsign", "font", 5, 150, SDL_Color({ 0,0,0,0 }), getRenderer());
				//birthSignVisualizer.show();
				if (showControl->tag != "Birthsign")
				{
					showControl->reset();

					//pass headers to the showControl
					std::vector< std::string > h = {"Name", "Full name", "Texture Filename", "Description", "Spells"};
					showControl->setHeaders(h);

					//pass data to the showControl
					std::vector< std::vector< std::string > > d;
					for(auto x : vbsgn){
						std::vector< std::string > temp;
						temp.push_back(x.name);
						temp.push_back(x.fullName);
						temp.push_back(x.textureFileName);
						temp.push_back(x.description);
						std::string tempText = "";
						for (int j = 0; j < x.spell_ability.size(); j++) {
							tempText += x.spell_ability[j];
							if (j != x.spell_ability.size() - 1) tempText += ", ";
						}
						temp.push_back(tempText);

						d.push_back(temp);
					}
					showControl->setData(d);
					
					showControl->tag = "Birthsign";
				}
			}

			if (Game::Instance()->lastButtonClicked == "Landscape Texture")
			{
				AssetsManager::Instance()->Text("Landscape Texture", "font", 5, 150, SDL_Color({ 0,0,0,0 }), getRenderer());
				if (showControl->tag != "Landscape Texture")
				{
					showControl->reset();

					showControl->tag = "Landscape Texture";
				}
			}

			if (Game::Instance()->lastButtonClicked == "Landscape")
			{
				AssetsManager::Instance()->Text("Landscape", "font", 5, 150, SDL_Color({ 0,0,0,0 }), getRenderer());
				if (showControl->tag != "Landscape")
				{
					showControl->reset();

					showControl->tag = "Landscape";
				}
			}

			if (Game::Instance()->lastButtonClicked == "Path Grid")
			{
				AssetsManager::Instance()->Text("Path Grid", "font", 5, 150, SDL_Color({ 0,0,0,0 }), getRenderer());
				if (showControl->tag != "Path Grid")
				{
					showControl->reset();

					showControl->tag = "Path Grid";
				}
			}

			if (Game::Instance()->lastButtonClicked == "Sound Generator")
			{
				AssetsManager::Instance()->Text("Sound Generator", "font", 5, 150, SDL_Color({ 0,0,0,0 }), getRenderer());
				if (showControl->tag != "Sound Generator")
				{
					showControl->reset();

					showControl->tag = "Sound Generator";
				}
			}

			if (Game::Instance()->lastButtonClicked == "Spell")
			{
				AssetsManager::Instance()->Text("Spell", "font", 5, 150, SDL_Color({ 0,0,0,0 }), getRenderer());
				if (showControl->tag != "Spell")
				{
					showControl->reset();

					showControl->tag = "Spell";
				}
			}

			if (Game::Instance()->lastButtonClicked == "Dialog")
			{
				AssetsManager::Instance()->Text("Dialog", "font", 5, 150, SDL_Color({ 0,0,0,0 }), getRenderer());
				if (showControl->tag != "Dialog")
				{
					showControl->reset();

					showControl->tag = "Dialog";
				}
			}

			mousepos.m_x = InputHandler::Instance()->getMousePosition()->m_x;
			mousepos.m_y = InputHandler::Instance()->getMousePosition()->m_y;
			AssetsManager::Instance()->Text(std::to_string(mousepos.m_x) + " " + std::to_string(mousepos.m_y), "font", mousepos.m_x, mousepos.m_y - 15, SDL_Color({ 0,0,0,0 }), getRenderer());
		}

		if (state == END_GAME)
		{
			//AssetsManager::Instance()->Text("End Game press space", "font", 100, 100, SDL_Color({ 0,0,0,0 }), Game::Instance()->getRenderer());
		}

	SDL_RenderPresent(m_pRenderer); // draw to the screen
}

void Game::quit()
{
	m_bRunning = false;
}

void Game::clean()
{
	std::cout << "cleaning game\n";
	for(auto x : entities) delete(x);
	InputHandler::Instance()->clean();
	AssetsManager::Instance()->clearFonts();
	TTF_Quit();
	SDL_DestroyWindow(m_pWindow);
	SDL_DestroyRenderer(m_pRenderer);
	Game::Instance()->m_bRunning = false;
	SDL_Quit();
}

void Game::handleEvents()
{
	InputHandler::Instance()->update();

	//HandleKeys
	if (state == MENU)
	{
		if (InputHandler::Instance()->isKeyDown(SDL_SCANCODE_S))
		{
			state = GAME;
			/*lives = 3;
			score = 0;*/
			//AssetsManager::Instance()->playMusic("music", 1);
		}
	}

	if (state == GAME)
	{
		if (InputHandler::Instance()->getMouseButtonState(LEFT))
		{
			mouseClicked = true;
			mousepos.m_x = InputHandler::Instance()->getMousePosition()->m_x;
			mousepos.m_y = InputHandler::Instance()->getMousePosition()->m_y;
		}
		
		for (auto i = entities.begin(); i != entities.end(); i++)
		{
			Entity *e = *i;

			e->handleEvents();
		}

		InputHandler::Instance()->setMouseButtonStatesToFalse();
		Game::Instance()->mouseClicked = false;
	}

	if (state == END_GAME)
	{
		if (InputHandler::Instance()->isKeyDown(SDL_SCANCODE_SPACE)) state = MENU;
	}

}

bool Game::isCollide(Entity *a, Entity *b)
{
	return (b->m_position.m_x - a->m_position.m_x)*(b->m_position.m_x - a->m_position.m_x) +
		(b->m_position.m_y - a->m_position.m_y)*(b->m_position.m_y - a->m_position.m_y) <
		(a->m_radius + b->m_radius)*(a->m_radius + b->m_radius);
}

bool Game::isCollideRect(Entity *a, Entity * b) {
	if (a->m_position.m_x < b->m_position.m_x + b->m_width &&
		a->m_position.m_x + a->m_width > b->m_position.m_x &&
		a->m_position.m_y < b->m_position.m_y + b->m_height &&
		a->m_height + a->m_position.m_y > b->m_position.m_y) {
		return true;
	}
	return false;
}

void Game::update()
{
	if (state == GAME)
	{

		//play some random car horns
		/*if (rnd.getRndInt(0, 100) == 0)
		{
			if (rnd.getRndInt(0, 1) == 0)
				AssetsManager::Instance()->playSound("carhorn1", 0);
			else
				AssetsManager::Instance()->playSound("carhorn2", 0);
		}*/

		// See if the chicken made it across
		//if (p->m_position.m_x > 400.0)
		//{
		//	// Play a sound for the chicken making it safely across
		//	//AssetsManager::Instance()->playSound("celebrate", 0);

		//	// Move the chicken back to the start and add to the score
		//	p->m_position.m_x = 4; p->m_position.m_y = 175;
		//	score += 150;
		//}

		for (auto a : entities)
		{
			for (auto b : entities)
			{
				//if (a->m_name == "player" && b->m_name == "car")
				//	if (isCollideRect(a, b))
				//	{
				//		lives--;
				//		if (lives <= 0)
				//		{
				//			//AssetsManager::Instance()->playSound("gameover", 0);
				//			state = END_GAME;
				//		}
				//		else
				//		{
				//			//AssetsManager::Instance()->playSound("squish", 0);
				//		}

				//		//relocate the chicken
				//		p->m_position.m_x = 4; p->m_position.m_y = 175;
				//		p->m_velocity.m_x = 0;
				//		p->m_velocity.m_y = 0;
				//	}
			}
		}

		for (auto i = entities.begin(); i != entities.end(); i++)
		{
			Entity *e = *i;

			e->update();
		}
	}

}

void Game::UpdateHiScores(int newscore)
{
	//new score to the end
	vhiscores.push_back(newscore);
	//sort
	sort(vhiscores.rbegin(), vhiscores.rend());
	//remove the last
	vhiscores.pop_back();
}

void Game::ReadHiScores()
{
	std::ifstream in("hiscores.dat");
	if (in.good())
	{
		std::string str;
		getline(in, str);
		std::stringstream ss(str);
		int n;
		for (int i = 0; i < 5; i++)
		{
			ss >> n;
			vhiscores.push_back(n);
		}
		in.close();
	}
	else
	{
		//if file does not exist fill with 5 scores
		for (int i = 0; i < 5; i++)
		{
			vhiscores.push_back(0);
		}
	}
}

void Game::WriteHiScores()
{
	std::ofstream out("hiscores.dat");
	for (int i = 0; i < 5; i++)
	{
		out << vhiscores[i] << " ";
	}
	out.close();
}

const int FPS = 60;
const int DELAY_TIME = 1000.0f / FPS;

int main(int argc, char* args[])
{
	srand(time(NULL));

	Uint32 frameStart, frameTime;

	std::cout << "game init attempt...\n";
	if (Game::Instance()->init("Elder Scrolls Master (ESM) file reader", 100, 100, 1024, 768,
		false))
	{
		std::cout << "game init success!\n";
		while (Game::Instance()->running())
		{
			frameStart = SDL_GetTicks(); //tiempo inicial

			Game::Instance()->handleEvents();
			Game::Instance()->update();
			Game::Instance()->render();

			frameTime = SDL_GetTicks() - frameStart; //tiempo final - tiempo inicial

			if (frameTime < DELAY_TIME)
			{
				//con tiempo fijo el retraso es 1000 / 60 = 16,66
				//procesar handleEvents, update y render tarda 1, y hay que esperar 15
				//cout << "frameTime : " << frameTime << "  delay : " << (int)(DELAY_TIME - frameTime) << endl;
				SDL_Delay((int)(DELAY_TIME - frameTime)); //esperamos hasta completar los 60 fps
			}
		}
	}
	else
	{
		std::cout << "game init failure - " << SDL_GetError() << "\n";
		return -1;
	}
	std::cout << "game closing...\n";
	Game::Instance()->clean();
	return 0;
}