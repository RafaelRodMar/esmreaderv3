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
				//AssetsManager::Instance()->Text("NPC", "font", 5, 150, SDL_Color({ 0,0,0,0 }), getRenderer());
				if (showControl->tag != "NPC")
				{
					//ready headers for the showControl
					std::vector< std::string > h = { "Name", "Full name", "Model", "Script", "Race", "ClassName", "Faction" };

					//pass data to the showControl
					std::vector< std::vector< std::string > > d;
					for (auto x : vnpc_) {
						std::vector< std::string > temp;
						temp.push_back(x.name);
						temp.push_back(x.fullName);
						temp.push_back(x.model);
						temp.push_back(x.script);
						temp.push_back(x.race);
						temp.push_back(x.className);
						temp.push_back(x.faction);
						d.push_back(temp);
					}
					showControl->setData(h, d);
		
					showControl->tag = "NPC";
				}
			}

			if (Game::Instance()->lastButtonClicked == "Creature")
			{
				//AssetsManager::Instance()->Text("Creature", "font", 5, 150, SDL_Color({ 0,0,0,0 }), getRenderer());
				if (showControl->tag != "Creature")
				{
					//ready headers for the showControl
					std::vector< std::string > h = { "Name", "Full name", "Model", "Script", "Sound", "Spells" };

					//pass data to the showControl
					std::vector< std::vector< std::string > > d;
					for (auto x : vcrea) {
						std::vector< std::string > temp;
						temp.push_back(x.name);
						temp.push_back(x.fullName);
						temp.push_back(x.model);
						temp.push_back(x.script);
						temp.push_back(x.sound);
						std::string aux;
						for (int i = 0; i < x.spells.size(); i++) {
							aux += x.spells[i] + ",";
						}
						temp.push_back(aux);

						d.push_back(temp);
					}
					showControl->setData(h, d);

					showControl->tag = "Creature";
				}
			}

			if (Game::Instance()->lastButtonClicked == "Leveled Creature")
			{
				//AssetsManager::Instance()->Text("Leveled Creature", "font", 5, 150, SDL_Color({ 0,0,0,0 }), getRenderer());
				if (showControl->tag != "Leveled Creature")
				{
					//ready headers for the showControl
					std::vector< std::string > h = { "Name", "Count", "Items" };

					//pass data to the showControl
					std::vector< std::vector< std::string > > d;
					for (auto x : vlevc) {
						std::vector< std::string > temp;
						temp.push_back(x.name);
						temp.push_back(to_string(x.count));
						std::string aux = "";
						for (int i = 0; i < x.item.size(); i++) {
							aux += x.item[i].first;
							aux += " (" + to_string(x.item[i].second) + " )";
						}
						temp.push_back(aux);

						d.push_back(temp);
					}
					showControl->setData(h, d);

					showControl->tag = "Leveled Creature";
				}
			}

			if (Game::Instance()->lastButtonClicked == "Spellmaking")
			{
				//AssetsManager::Instance()->Text("Spellmaking", "font", 5, 150, SDL_Color({ 0,0,0,0 }), getRenderer());
				if (showControl->tag != "Spellmaking")
				{
					//ready headers for the showControl
					std::vector< std::string > h = { "Name", "Full name", "Type", "Spell Cost", "Área-duration-range" };

					//pass data to the showControl
					std::vector< std::vector< std::string > > d;
					for (auto x : vspel) {
						std::vector< std::string > temp;
						temp.push_back(x.name);
						temp.push_back(x.fullName);
						temp.push_back(to_string(x.sd.type));
						temp.push_back(to_string(x.sd.spellCost));
						std::string aux = "";
						for (int i = 0; i < x.ed.size(); i++) {
							aux += to_string(x.ed[i].area) + " ";
							aux += to_string(x.ed[i].duration) + " ";
							aux += to_string(x.ed[i].range);
						}
						temp.push_back(aux);

						d.push_back(temp);
					}
					showControl->setData(h, d);

					showControl->tag = "Spellmaking";
				}
			}

			if (Game::Instance()->lastButtonClicked == "Enchanting")
			{
				//AssetsManager::Instance()->Text("Enchanting", "font", 5, 150, SDL_Color({ 0,0,0,0 }), getRenderer());
				if (showControl->tag != "Enchanting")
				{
					//ready headers for the showControl
					std::vector< std::string > h = { "Name", "Type", "Enchantment Cost", "Charge", "Área-duration-range" };

					//pass data to the showControl
					std::vector< std::vector< std::string > > d;
					for (auto x : vench) {
						std::vector< std::string > temp;
						temp.push_back(x.name);
						temp.push_back(to_string(x.ed.type));
						temp.push_back(to_string(x.ed.enchantmentCost));
						temp.push_back(to_string(x.ed.charge));
						std::string aux = "";
						for (int i = 0; i < x.enchantments.size(); i++) {
							aux += to_string(x.enchantments[i].area) + " ";
							aux += to_string(x.enchantments[i].duration) + " ";
							aux += to_string(x.enchantments[i].range);
						}
						temp.push_back(aux);

						d.push_back(temp);
					}
					showControl->setData(h, d);

					showControl->tag = "Enchanting";
				}
			}

			if (Game::Instance()->lastButtonClicked == "Alchemy")
			{
				//AssetsManager::Instance()->Text("Alchemy", "font", 5, 150, SDL_Color({ 0,0,0,0 }), getRenderer());
				if (showControl->tag != "Alchemy")
				{
					//ready headers for the showControl
					std::vector< std::string > h = { "Name", "Full name", "Model", "Script", "Icon", "Flags", "Weight", "Value" };

					//pass data to the showControl
					std::vector< std::vector< std::string > > d;
					for (auto x : valch) {
						std::vector< std::string > temp;
						temp.push_back(x.name);
						temp.push_back(x.fullName);
						temp.push_back(x.model);
						temp.push_back(x.script);
						temp.push_back(x.icon);
						temp.push_back(to_string(x.ad.flags));
						temp.push_back(to_string(x.ad.weight));
						temp.push_back(to_string(x.ad.value));

						d.push_back(temp);
					}
					showControl->setData(h, d);
					showControl->iconColumn = 4;

					showControl->tag = "Alchemy";
				}
			}

			if (Game::Instance()->lastButtonClicked == "Leveled Item")
			{
				//AssetsManager::Instance()->Text("Leveled Item", "font", 5, 150, SDL_Color({ 0,0,0,0 }), getRenderer());
				if (showControl->tag != "Leveled Item")
				{
					//ready headers for the showControl
					std::vector< std::string > h = { "Name", "Count", "Items" };

					//pass data to the showControl
					std::vector< std::vector< std::string > > d;
					for (auto x : vlevi) {
						std::vector< std::string > temp;
						temp.push_back(x.name);
						temp.push_back(to_string(x.count));
						std::string aux = "";
						for (int i = 0; i < x.item.size(); i++) {
							aux += x.item[i].first;
							aux += " (" + to_string(x.item[i].second) + " )";
						}
						temp.push_back(aux);

						d.push_back(temp);
					}
					showControl->setData(h, d);

					showControl->tag = "Leveled Item";
				}
			}

			if (Game::Instance()->lastButtonClicked == "Activator")
			{
				//AssetsManager::Instance()->Text("Activator", "font", 5, 150, SDL_Color({ 0,0,0,0 }), getRenderer());
				if (showControl->tag != "Activator")
				{
					//ready headers for the showControl
					std::vector< std::string > h = { "Name", "Full name", "Model", "Script" };

					//pass data to the showControl
					std::vector< std::vector< std::string > > d;
					for (auto x : vacti) {
						std::vector< std::string > temp;
						temp.push_back(x.name);
						temp.push_back(x.fullName);
						temp.push_back(x.model);
						temp.push_back(x.script);

						d.push_back(temp);
					}
					showControl->setData(h, d);

					showControl->tag = "Activator";
				}
			}

			if (Game::Instance()->lastButtonClicked == "Apparatus")
			{
				//AssetsManager::Instance()->Text("Apparatus", "font", 5, 150, SDL_Color({ 0,0,0,0 }), getRenderer());
				if (showControl->tag != "Apparatus")
				{
					//ready headers for the showControl
					std::vector< std::string > h = { "Name", "Full name", "Model", "Script", "Icon", "Type", "Weight", "Value", "Quality" };

					//pass data to the showControl
					std::vector< std::vector< std::string > > d;
					for (auto x : vappa) {
						std::vector< std::string > temp;
						temp.push_back(x.name);
						temp.push_back(x.fullName);
						temp.push_back(x.model);
						temp.push_back(x.script);
						temp.push_back(x.icon);
						temp.push_back(to_string(x.ad.type));
						temp.push_back(to_string(x.ad.weight));
						temp.push_back(to_string(x.ad.value));
						temp.push_back(to_string(x.ad.quality));

						d.push_back(temp);
					}
					showControl->setData(h, d);
					showControl->iconColumn = 4;

					showControl->tag = "Apparatus";
				}
			}

			if (Game::Instance()->lastButtonClicked == "Armor")
			{
				//AssetsManager::Instance()->Text("Armor", "font", 5, 150, SDL_Color({ 0,0,0,0 }), getRenderer());
				if (showControl->tag != "Armor")
				{
					//ready headers for the showControl
					std::vector< std::string > h = { "Name", "Full name", "Model", "Type", "Weight", "Value", "Health", "Enchantment Points", "Armour", "Icon Filename", "Body Part-Male Armor name-Female Armor name", "Enchantment", "Script" };

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
					showControl->setData(h,d);
					showControl->iconColumn = 9;

					showControl->tag = "Armor";
				}
			}

			if (Game::Instance()->lastButtonClicked == "Body Part")
			{
				//AssetsManager::Instance()->Text("Body Part", "font", 5, 150, SDL_Color({ 0,0,0,0 }), getRenderer());
				if (showControl->tag != "Body Part")
				{
					//ready headers for the showControl
					std::vector< std::string > h = { "Name", "Full name", "Model" };

					//pass data to the showControl
					std::vector< std::vector< std::string > > d;
					for (auto x : vbody) {
						std::vector< std::string > temp;
						temp.push_back(x.name);
						temp.push_back(x.fullName);
						temp.push_back(x.model);

						d.push_back(temp);
					}
					showControl->setData(h, d);

					showControl->tag = "Body Part";
				}
			}

			if (Game::Instance()->lastButtonClicked == "Book")
			{
				//AssetsManager::Instance()->Text("Book", "font", 5, 150, SDL_Color({ 0,0,0,0 }), getRenderer());
				if (showControl->tag != "Book")
				{
					//ready headers for the showControl
					std::vector< std::string > h = { "Name", "Full name", "Model", "Icon", "Enchantment", "Script", "Weight", "Value", "SkillId", "Scroll", "Enchantment Points", "Text" };
					//pass data to the showControl
					std::vector< std::vector< std::string > > d;
					for (auto x : vbook) {
						std::vector< std::string > temp;
						temp.push_back(x.name);
						temp.push_back(x.fullName);
						temp.push_back(x.model);
						temp.push_back(x.icon);
						temp.push_back(x.enchantment);
						temp.push_back(x.script);
						temp.push_back(to_string(x.bd.weight));
						temp.push_back(to_string(x.bd.value));
						temp.push_back(to_string(x.bd.skillID));
						temp.push_back(to_string(x.bd.scroll));
						temp.push_back(to_string(x.bd.enchantPts));
						std::string tempstr = "";
						for (int i = 0; i < std::min(100,(int)x.text.size()); i++) {
							if (x.text[i] >= 'a' && x.text[i] <= 'z') tempstr += x.text[i];
							if (x.text[i] >= 'A' && x.text[i] <= 'Z') tempstr += x.text[i];
							if (x.text[i] >= '0' && x.text[i] <= '9') tempstr += x.text[i];
						}
						temp.push_back(tempstr);
						//remove line breaks for text converter
						temp.back().erase(std::remove(temp.back().begin(), temp.back().end(), '\n'), temp.back().end());
						temp.back().erase(std::remove(temp.back().begin(), temp.back().end(), '\r'), temp.back().end());
						/*temp.back().erase(std::remove_if(temp.back().begin(), temp.back().end(),
							[](auto const& c) -> bool { return !std::isalnum(c); }), temp.back().end());*/
												
						d.push_back(temp);
					}
					showControl->setData(h, d);
					showControl->iconColumn = 3;

					showControl->tag = "Book";
				}
			}

			if (Game::Instance()->lastButtonClicked == "Clothing")
			{
				//AssetsManager::Instance()->Text("Clothing", "font", 5, 150, SDL_Color({ 0,0,0,0 }), getRenderer());
				if (showControl->tag != "Clothing")
				{
					//ready headers for the showControl
					std::vector< std::string > h = { "Name", "Full name", "Model", "Icon", "Enchantment", "Script", "Weight", "Value", "Type", "Enchantment Points", "Body part--male name--female name" };
					//pass data to the showControl
					std::vector< std::vector< std::string > > d;
					for (auto x : vclot) {
						std::vector< std::string > temp;
						temp.push_back(x.name);
						temp.push_back(x.fullName);
						temp.push_back(x.model);
						temp.push_back(x.icon);
						temp.push_back(x.enchantment);
						temp.push_back(x.script);
						temp.push_back(to_string(x.cd.weight));
						temp.push_back(to_string(x.cd.value));
						temp.push_back(to_string(x.cd.type));
						temp.push_back(to_string(x.cd.enchantmentPts));
						std::string tempText = "";
						for (int j = 0; j < x.bp.size(); j++) {
							tempText += x.bp[j].bodyPartIndex + "--";
							tempText += x.bp[j].maleBodyPartName + "--";
							tempText += x.bp[j].femaleBodyPartName;
							if (j != x.bp.size() - 1) tempText += ", ";
						}
						temp.push_back(tempText);

						d.push_back(temp);
					}
					showControl->setData(h, d);
					showControl->iconColumn = 3;

					showControl->tag = "Clothing";
				}
			}

			if (Game::Instance()->lastButtonClicked == "Container")
			{
				AssetsManager::Instance()->Text("Container", "font", 5, 150, SDL_Color({ 0,0,0,0 }), getRenderer());
				if (showControl->tag != "Container")
				{
					//ready headers for the showControl
					std::vector< std::string > h = { "Name", "Full name", "Model", "Script", "Weight", "Items" };

					//pass data to the showControl
					std::vector< std::vector< std::string > > d;
					for (auto x : vcont) {
						std::vector< std::string > temp;
						temp.push_back(x.name);
						temp.push_back(x.fullName);
						temp.push_back(x.model);
						temp.push_back(x.script);
						temp.push_back(to_string(x.weight));
						std::string aux = "";
						for (int i = 0; i < x.items.size(); i++) {
							aux += x.items[i].name;
							aux += " (" + to_string(x.items[i].count) + ") ";
						}
						temp.push_back(aux);

						d.push_back(temp);
					}
					showControl->setData(h, d);

					showControl->tag = "Container";
				}
			}

			if (Game::Instance()->lastButtonClicked == "Door")
			{
				//AssetsManager::Instance()->Text("Door", "font", 5, 150, SDL_Color({ 0,0,0,0 }), getRenderer());
				if (showControl->tag != "Door")
				{
					//ready headers for the showControl
					std::vector< std::string > h = { "Name", "Full name", "Model", "Script", "Open Sound", "Close Sound" };

					//pass data to the showControl
					std::vector< std::vector< std::string > > d;
					for (auto x : vdoor) {
						std::vector< std::string > temp;
						temp.push_back(x.name);
						temp.push_back(x.fullName);
						temp.push_back(x.model);
						temp.push_back(x.script);
						temp.push_back(x.openSound);
						temp.push_back(x.closeSound);

						d.push_back(temp);
					}
					showControl->setData(h, d);

					showControl->tag = "Door";
				}
			}

			if (Game::Instance()->lastButtonClicked == "Ingredient")
			{
				AssetsManager::Instance()->Text("Ingredient", "font", 5, 150, SDL_Color({ 0,0,0,0 }), getRenderer());
				if (showControl->tag != "Ingredient")
				{
					//ready headers for the showControl
					std::vector< std::string > h = { "Name", "Full name", "Model", "Icon", "Script", "Value", "Weight" };

					//pass data to the showControl
					std::vector< std::vector< std::string > > d;
					for (auto x : vingr) {
						std::vector< std::string > temp;
						temp.push_back(x.name);
						temp.push_back(x.fullName);
						temp.push_back(x.model);
						temp.push_back(x.icon);
						temp.push_back(x.script);
						temp.push_back(to_string(x.id.value));
						temp.push_back(to_string(x.id.weight));

						d.push_back(temp);
					}
					showControl->setData(h, d);
					showControl->iconColumn = 3;

					showControl->tag = "Ingredient";
				}
			}

			if (Game::Instance()->lastButtonClicked == "Light")
			{
				//AssetsManager::Instance()->Text("Light", "font", 5, 150, SDL_Color({ 0,0,0,0 }), getRenderer());
				if (showControl->tag != "Light")
				{
					//ready headers for the showControl
					std::vector< std::string > h = { "Name", "Full name", "Model", "Icon", "Script", "Value", "Uses" };

					//pass data to the showControl
					std::vector< std::vector< std::string > > d;
					for (auto x : vligh) {
						std::vector< std::string > temp;
						temp.push_back(x.name);
						temp.push_back(x.fullName);
						temp.push_back(x.model);
						temp.push_back(x.icon);
						temp.push_back(x.script);
						temp.push_back(to_string(x.ld.value));
						temp.push_back(to_string(x.ld.weight));

						d.push_back(temp);
					}
					showControl->setData(h, d);
					showControl->iconColumn = 3;

					showControl->tag = "Light";
				}
			}

			if (Game::Instance()->lastButtonClicked == "Lockpick")
			{
				//AssetsManager::Instance()->Text("Lockpick", "font", 5, 150, SDL_Color({ 0,0,0,0 }), getRenderer());
				if (showControl->tag != "Lockpick")
				{
					//ready headers for the showControl
					std::vector< std::string > h = { "Name", "Full name", "Model", "Icon", "Script", "Quality", "Value", "Uses", "Weight" };

					//pass data to the showControl
					std::vector< std::vector< std::string > > d;
					for (auto x : vlock) {
						std::vector< std::string > temp;
						temp.push_back(x.name);
						temp.push_back(x.fullName);
						temp.push_back(x.model);
						temp.push_back(x.icon);
						temp.push_back(x.script);
						temp.push_back(to_string(x.ld.quality));
						temp.push_back(to_string(x.ld.value));
						temp.push_back(to_string(x.ld.uses));
						temp.push_back(to_string(x.ld.weight));

						d.push_back(temp);
					}
					showControl->setData(h, d);
					showControl->iconColumn = 3;

					showControl->tag = "Lockpick";
				}
			}

			if (Game::Instance()->lastButtonClicked == "Misc Item")
			{
				//AssetsManager::Instance()->Text("Misc Item", "font", 5, 150, SDL_Color({ 0,0,0,0 }), getRenderer());
				if (showControl->tag != "Misc Item")
				{
					//ready headers for the showControl
					std::vector< std::string > h = { "Name", "Full name", "Model", "Icon", "Script", "Enchantment", "Value", "Weight" };

					//pass data to the showControl
					std::vector< std::vector< std::string > > d;
					for (auto x : vmisc) {
						std::vector< std::string > temp;
						temp.push_back(x.name);
						temp.push_back(x.fullName);
						temp.push_back(x.model);
						temp.push_back(x.icon);
						temp.push_back(x.script);
						temp.push_back(x.enchantment);
						temp.push_back(to_string(x.mid.value));
						temp.push_back(to_string(x.mid.weight));

						d.push_back(temp);
					}
					showControl->setData(h, d);
					showControl->iconColumn = 3;

					showControl->tag = "Misc Item";
				}
			}

			if (Game::Instance()->lastButtonClicked == "Probe")
			{
				//AssetsManager::Instance()->Text("Probe", "font", 5, 150, SDL_Color({ 0,0,0,0 }), getRenderer());
				if (showControl->tag != "Probe")
				{
					//ready headers for the showControl
					std::vector< std::string > h = { "Name", "Full name", "Model", "Icon", "Script", "Quality", "Uses", "Value", "Weight" };

					//pass data to the showControl
					std::vector< std::vector< std::string > > d;
					for (auto x : vprob) {
						std::vector< std::string > temp;
						temp.push_back(x.name);
						temp.push_back(x.fullName);
						temp.push_back(x.model);
						temp.push_back(x.icon);
						temp.push_back(x.script);
						temp.push_back(to_string(x.pd.quality));
						temp.push_back(to_string(x.pd.uses));
						temp.push_back(to_string(x.pd.value));
						temp.push_back(to_string(x.pd.weight));

						d.push_back(temp);
					}
					showControl->setData(h, d);
					showControl->iconColumn = 3;

					showControl->tag = "Probe";
				}
			}

			if (Game::Instance()->lastButtonClicked == "Repair Item")
			{
				//AssetsManager::Instance()->Text("Repair Item", "font", 5, 150, SDL_Color({ 0,0,0,0 }), getRenderer());
				if (showControl->tag != "Repair Item")
				{
					//ready headers for the showControl
					std::vector< std::string > h = { "Name", "Full name", "Model", "Icon", "Script", "Quality", "Uses", "Value", "Weight" };

					//pass data to the showControl
					std::vector< std::vector< std::string > > d;
					for (auto x : vrepa) {
						std::vector< std::string > temp;
						temp.push_back(x.name);
						temp.push_back(x.fullName);
						temp.push_back(x.model);
						temp.push_back(x.icon);
						temp.push_back(x.script);
						temp.push_back(to_string(x.rd.quality));
						temp.push_back(to_string(x.rd.uses));
						temp.push_back(to_string(x.rd.value));
						temp.push_back(to_string(x.rd.weight));

						d.push_back(temp);
					}
					showControl->setData(h, d);
					showControl->iconColumn = 3;

					showControl->tag = "Repair Item";
				}
			}

			if (Game::Instance()->lastButtonClicked == "Static")
			{
				//AssetsManager::Instance()->Text("Static", "font", 5, 150, SDL_Color({ 0,0,0,0 }), getRenderer());
				if (showControl->tag != "Static")
				{
					//ready headers for the showControl
					std::vector< std::string > h = { "Name", "Model" };

					//pass data to the showControl
					std::vector< std::vector< std::string > > d;
					for (auto x : vstat) {
						std::vector< std::string > temp;
						temp.push_back(x.name);
						temp.push_back(x.model);

						d.push_back(temp);
					}
					showControl->setData(h, d);

					showControl->tag = "Static";
				}
			}

			if (Game::Instance()->lastButtonClicked == "Weapon")
			{
				//AssetsManager::Instance()->Text("Weapon", "font", 5, 150, SDL_Color({ 0,0,0,0 }), getRenderer());
				if (showControl->tag != "Weapon")
				{
					//ready headers for the showControl
					std::vector< std::string > h = { "Name", "Full name", "Model", "Icon", "Script", "Enchantment", "Type", "Value", "Weight" };

					//pass data to the showControl
					std::vector< std::vector< std::string > > d;
					for (auto x : vweap) {
						std::vector< std::string > temp;
						temp.push_back(x.name);
						temp.push_back(x.fullName);
						temp.push_back(x.model);
						temp.push_back(x.icon);
						temp.push_back(x.script);
						temp.push_back(x.enchantment);
						temp.push_back(to_string(x.wd.type));
						temp.push_back(to_string(x.wd.value));
						temp.push_back(to_string(x.wd.weight));

						d.push_back(temp);
					}
					showControl->setData(h, d);
					showControl->iconColumn = 3;

					showControl->tag = "Weapon";
				}
			}

			if (Game::Instance()->lastButtonClicked == "Cell")
			{
				//AssetsManager::Instance()->Text("Cell", "font", 5, 150, SDL_Color({ 0,0,0,0 }), getRenderer());
				int maxX = -100;
				int maxY = -100;
				if (showControl->tag != "Cell")
				{
					//ready headers for the showControl
					std::vector< std::string > h = { "Name", "Region name", "Flags (1=interior)", "Position", "Persistent Refs" };

					//pass data to the showControl
					std::vector< std::vector< std::string > > d;
					for (auto x : vcell) {
						std::vector< std::string > temp;
						temp.push_back(x.name);
						temp.push_back(x.regionName);
						temp.push_back(to_string(x.cd.flags));
						temp.push_back(to_string(x.cd.gridX));
						temp.back() += "," + to_string(x.cd.gridY);
						if (x.cd.flags & (1 << 0)) temp.back() += " interior ";
						if (x.cd.flags & (1 << 1)) temp.back() += " has water ";
						if (x.cd.flags & (1 << 2)) temp.back() += " illegal to sleep here ";
						if (x.cd.gridX > maxX) maxX = x.cd.gridX;
						if (x.cd.gridY > maxY) maxY = x.cd.gridY;
						std::string tempText = "";
						for (int j = 0; j < x.persistentRefs.size(); j++) {
							tempText += x.persistentRefs[j].name;
							if (j != x.persistentRefs.size() - 1) tempText += ", ";
						}
						temp.push_back(tempText);

						d.push_back(temp);
					}
					showControl->setData(h, d);
					std::cout << "maxX: " << maxX <<
						"maxY: " << maxY << std::endl;

					showControl->tag = "Cell";
				}
			}

			if (Game::Instance()->lastButtonClicked == "Game Settings")
			{
				//AssetsManager::Instance()->Text("Game Settings", "font", 5, 150, SDL_Color({ 0,0,0,0 }), getRenderer());
				if (showControl->tag != "Game Settings")
				{
					//ready headers for the showControl
					std::vector< std::string > h = { "Name", "Int value", "Float value", "String value" };

					//pass data to the showControl
					std::vector< std::vector< std::string > > d;
					for (auto x : vgmst) {
						std::vector< std::string > temp;
						temp.push_back(x.name);
						temp.push_back(to_string(x.intValue));
						temp.push_back(to_string(x.floatValue));
						temp.push_back(x.stringValue);

						d.push_back(temp);
					}
					showControl->setData(h, d);

					showControl->tag = "Game Settings";
				}
			}

			if (Game::Instance()->lastButtonClicked == "Global")
			{
				//AssetsManager::Instance()->Text("Global", "font", 5, 150, SDL_Color({ 0,0,0,0 }), getRenderer());
				if (showControl->tag != "Global")
				{
					//ready headers for the showControl
					std::vector< std::string > h = { "Name", "Type", "Value" };

					//pass data to the showControl
					std::vector< std::vector< std::string > > d;
					for (auto x : vglob) {
						std::vector< std::string > temp;
						temp.push_back(x.name);
						temp.push_back(x.type);
						temp.push_back(to_string(x.value));

						d.push_back(temp);
					}
					showControl->setData(h, d);

					showControl->tag = "Global";
				}
			}

			if (Game::Instance()->lastButtonClicked == "Class")
			{
				//AssetsManager::Instance()->Text("Class", "font", 5, 150, SDL_Color({ 0,0,0,0 }), getRenderer());
				if (showControl->tag != "Class")
				{
					//ready headers for the showControl
					std::vector< std::string > h = { "Name", "Full name", "Description" };

					//pass data to the showControl
					std::vector< std::vector< std::string > > d;
					for (auto x : vclas) {
						std::vector< std::string > temp;
						temp.push_back(x.name);
						temp.push_back(x.fullName);
						temp.push_back(x.description);

						d.push_back(temp);
					}
					showControl->setData(h, d);

					showControl->tag = "Class";
				}
			}

			if (Game::Instance()->lastButtonClicked == "Faction")
			{
				//AssetsManager::Instance()->Text("Faction", "font", 5, 150, SDL_Color({ 0,0,0,0 }), getRenderer());
				if (showControl->tag != "Faction")
				{
					//ready headers for the showControl
					std::vector< std::string > h = { "Name", "Full name", "Rank names" };

					//pass data to the showControl
					std::vector< std::vector< std::string > > d;
					for (auto x : vfact) {
						std::vector< std::string > temp;
						temp.push_back(x.name);
						temp.push_back(x.fullName);
						std::string aux = "";
						for (int i = 0; i < x.rankName.size(); i++) {
							aux += x.rankName[i] + "--";
						}
						temp.push_back(aux);

						d.push_back(temp);
					}
					showControl->setData(h, d);

					showControl->tag = "Faction";
				}
			}

			if (Game::Instance()->lastButtonClicked == "Race")
			{
				//AssetsManager::Instance()->Text("Race", "font", 5, 150, SDL_Color({ 0,0,0,0 }), getRenderer());
				if (showControl->tag != "Race")
				{
					//ready headers for the showControl
					std::vector< std::string > h = { "Name", "Full name", "Description", "Special" };

					//pass data to the showControl
					std::vector< std::vector< std::string > > d;
					for (auto x : vrace) {
						std::vector< std::string > temp;
						temp.push_back(x.name);
						temp.push_back(x.fullName);
						temp.push_back(x.description);
						std::string aux = "";
						for (int i = 0; i < x.special.size(); i++) {
							aux += x.special[i] + "--";
						}
						temp.push_back(aux);

						d.push_back(temp);
					}
					showControl->setData(h, d);

					showControl->tag = "Race";
				}
			}

			if (Game::Instance()->lastButtonClicked == "Sound")
			{
				//AssetsManager::Instance()->Text("Sound", "font", 5, 150, SDL_Color({ 0,0,0,0 }), getRenderer());
				if (showControl->tag != "Sound")
				{
					//pass headers to the showControl
					std::vector< std::string > h = { "Name", "File name"};

					//pass data to the showControl
					std::vector< std::vector< std::string > > d;
					for (auto x : vsoun) {
						std::vector< std::string > temp;
						temp.push_back(x.name);
						temp.push_back(x.fullName);
						d.push_back(temp);
					}
					showControl->setData(h, d);
					showControl->soundColumn = 1;

					showControl->tag = "Sound";
				}
			}

			if (Game::Instance()->lastButtonClicked == "Skill")
			{
				//AssetsManager::Instance()->Text("Skill", "font", 5, 150, SDL_Color({ 0,0,0,0 }), getRenderer());
				if (showControl->tag != "Skill")
				{
					//ready headers for the showControl
					std::vector< std::string > h = { "Index", "description", "Atribute", "Specialization" };

					//pass data to the showControl
					std::vector< std::vector< std::string > > d;
					for (auto x : vskil) {
						std::vector< std::string > temp;
						temp.push_back(to_string(x.index));
						temp.push_back(x.description);
						temp.push_back(to_string(x.sd.attribute));
						temp.push_back(to_string(x.sd.specialization));

						d.push_back(temp);
					}
					showControl->setData(h, d);

					showControl->tag = "Skill";
				}
			}

			if (Game::Instance()->lastButtonClicked == "Magic Effects")
			{
				//AssetsManager::Instance()->Text("Magic Effects", "font", 5, 150, SDL_Color({ 0,0,0,0 }), getRenderer());
				if (showControl->tag != "Magic Effects")
				{
					//ready headers for the showControl
					std::vector< std::string > h = { "Index", "description", "Icon", "Cost", "Visual (área, bolt, casting)", "Sound (área, bolt, casting)" };

					//pass data to the showControl
					std::vector< std::vector< std::string > > d;
					for (auto x : vmgef) {
						std::vector< std::string > temp;
						temp.push_back(to_string(x.index));
						temp.push_back(x.description);
						temp.push_back(x.iconTexture);
						temp.push_back(to_string(x.md.baseCost));
						temp.push_back(x.areaVisual);
						temp.back() += " " + x.boltVisual + " " + x.castingVisual;
						temp.push_back(x.areaSound);
						temp.back() += " " + x.boltSound + " " + x.castSound;
						d.push_back(temp);
					}
					showControl->setData(h, d);
					showControl->iconColumn = 2;

					showControl->tag = "Magic Effects";
				}
			}

			if (Game::Instance()->lastButtonClicked == "Script")
			{
				//AssetsManager::Instance()->Text("Script", "font", 5, 150, SDL_Color({ 0,0,0,0 }), getRenderer());
				if (showControl->tag != "Script")
				{
					//ready headers for the showControl
					std::vector< std::string > h = { "Name", "Local Vars", "Text" };

					//pass data to the showControl
					std::vector< std::vector< std::string > > d;
					for (auto x : vscpt) {
						std::vector< std::string > temp;
						temp.push_back(x.sh.name);
						std::string aux = "";
						for (int i = 0; i < x.localVars.size(); i++) {
							aux += x.localVars[i] + ",";
						}
						temp.push_back(aux);
						temp.push_back(x.textScpt);
						d.push_back(temp);
					}
					showControl->setData(h, d);

					showControl->tag = "Script";
				}
			}

			if (Game::Instance()->lastButtonClicked == "Region")
			{
				//AssetsManager::Instance()->Text("Region", "font", 5, 150, SDL_Color({ 0,0,0,0 }), getRenderer());
				if (showControl->tag != "Region")
				{
					//ready headers for the showControl
					std::vector< std::string > h = { "Name", "Full name", "Sleep Creature" };

					//pass data to the showControl
					std::vector< std::vector< std::string > > d;
					for (auto x : vregn) {
						std::vector< std::string > temp;
						temp.push_back(x.name);
						temp.push_back(x.fullName);
						temp.push_back(x.sleepCreature);
						d.push_back(temp);
					}
					showControl->setData(h, d);

					showControl->tag = "Region";
				}
			}

			if (Game::Instance()->lastButtonClicked == "Birthsign")
			{
				//AssetsManager::Instance()->Text("Birthsign", "font", 5, 150, SDL_Color({ 0,0,0,0 }), getRenderer());
				//birthSignVisualizer.show();
				if (showControl->tag != "Birthsign")
				{
					//pass headers to the showControl
					std::vector< std::string > h = {"Name", "Full name", "Texture Filename", "Description", "Spells"};

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
					showControl->setData(h,d);
					showControl->textureColumn = 2;
					
					showControl->tag = "Birthsign";
				}
			}

			if (Game::Instance()->lastButtonClicked == "Landscape Texture")
			{
				//AssetsManager::Instance()->Text("Landscape Texture", "font", 5, 150, SDL_Color({ 0,0,0,0 }), getRenderer());
				if (showControl->tag != "Landscape Texture")
				{
					//pass headers to the showControl
					std::vector< std::string > h = { "Name", "Index", "Texture Filename" };

					//pass data to the showControl
					std::vector< std::vector< std::string > > d;
					for (auto x : vltex) {
						std::vector< std::string > temp;
						temp.push_back(x.name);
						temp.push_back(to_string(x.index));
						temp.push_back(x.filename);
						d.push_back(temp);
					}
					showControl->setData(h, d);
					showControl->textureColumn = 2;

					showControl->tag = "Landscape Texture";
				}
			}

			if (Game::Instance()->lastButtonClicked == "Landscape")
			{
				//AssetsManager::Instance()->Text("Landscape", "font", 5, 150, SDL_Color({ 0,0,0,0 }), getRenderer());
				if (showControl->tag != "Landscape")
				{
					//pass headers to the showControl
					std::vector< std::string > h = { "Coord x", "Coord y" };

					//pass data to the showControl
					std::vector< std::vector< std::string > > d;
					for (auto x : vland) {
						std::vector< std::string > temp;
						temp.push_back(to_string(x.coord.x));
						temp.push_back(to_string(x.coord.y));
						d.push_back(temp);
					}
					showControl->setData(h, d);

					showControl->tag = "Landscape";
				}
			}

			if (Game::Instance()->lastButtonClicked == "Path Grid")
			{
				//AssetsManager::Instance()->Text("Path Grid", "font", 5, 150, SDL_Color({ 0,0,0,0 }), getRenderer());
				if (showControl->tag != "Path Grid")
				{
					//pass headers to the showControl
					std::vector< std::string > h = { "Name" };

					//pass data to the showControl
					std::vector< std::vector< std::string > > d;
					for (auto x : vpgrd) {
						std::vector< std::string > temp;
						temp.push_back(x.name);
						d.push_back(temp);
					}
					showControl->setData(h, d);

					showControl->tag = "Path Grid";
				}
			}

			if (Game::Instance()->lastButtonClicked == "Sound Generator")
			{
				//AssetsManager::Instance()->Text("Sound Generator", "font", 5, 150, SDL_Color({ 0,0,0,0 }), getRenderer());
				if (showControl->tag != "Sound Generator")
				{
					//pass headers to the showControl
					std::vector< std::string > h = { "Name", "Creature Name", "Sound id", "Sound type" };

					//pass data to the showControl
					std::vector< std::vector< std::string > > d;
					for (auto x : vsndg) {
						std::vector< std::string > temp;
						temp.push_back(x.name);
						temp.push_back(x.creatureName);
						temp.push_back(x.soundID);
						temp.push_back(to_string(x.soundTypeData));
						d.push_back(temp);
					}
					showControl->setData(h, d);

					showControl->tag = "Sound Generator";
				}
			}

			if (Game::Instance()->lastButtonClicked == "Spell")
			{
				//AssetsManager::Instance()->Text("Spell", "font", 5, 150, SDL_Color({ 0,0,0,0 }), getRenderer());
				if (showControl->tag != "Spell")
				{
					//ready headers for the showControl
					std::vector< std::string > h = { "Name", "Full name", "Type", "Spell Cost", "Área-duration-range" };

					//pass data to the showControl
					std::vector< std::vector< std::string > > d;
					for (auto x : vspel) {
						std::vector< std::string > temp;
						temp.push_back(x.name);
						temp.push_back(x.fullName);
						temp.push_back(to_string(x.sd.type));
						temp.push_back(to_string(x.sd.spellCost));
						std::string aux = "";
						for (int i = 0; i < x.ed.size(); i++) {
							aux += to_string(x.ed[i].area) + " ";
							aux += to_string(x.ed[i].duration) + " ";
							aux += to_string(x.ed[i].range);
						}
						temp.push_back(aux);

						d.push_back(temp);
					}
					showControl->setData(h, d);

					showControl->tag = "Spell";
				}
			}

			if (Game::Instance()->lastButtonClicked == "Dialog")
			{
				//AssetsManager::Instance()->Text("Dialog", "font", 5, 150, SDL_Color({ 0,0,0,0 }), getRenderer());
				if (showControl->tag != "Dialog")
				{
					//ready headers for the showControl
					std::vector< std::string > h = { "Name", "Dialogue type" };

					//pass data to the showControl
					std::vector< std::vector< std::string > > d;
					for (auto x : vdial) {
						std::vector< std::string > temp;
						temp.push_back(x.name);
						temp.push_back(to_string(x.dialogueType));

						d.push_back(temp);
					}
					showControl->setData(h, d);

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
	AssetsManager::Instance()->clearAll();
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