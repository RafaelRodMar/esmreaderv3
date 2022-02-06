#include "esmreader.h"
#include "game.h"
#include "AssetsManager.h"

class BirthSign {
public:
	void show() {
		int xpos = 5;
		int ypos = 150;
		//show header
		AssetsManager::Instance()->Text("NAME", "font", 5, ypos, SDL_Color({ 0,0,0,0 }), Game::Instance()->getRenderer());
		AssetsManager::Instance()->Text("FULL NAME", "font", 120, ypos, SDL_Color({ 0,0,0,0 }), Game::Instance()->getRenderer());
		AssetsManager::Instance()->Text("TEXTURE FILE NAME", "font", 220, ypos, SDL_Color({ 0,0,0,0 }), Game::Instance()->getRenderer());
		AssetsManager::Instance()->Text("DESCRIPTION", "font", 400, ypos, SDL_Color({ 0,0,0,0 }), Game::Instance()->getRenderer());
		AssetsManager::Instance()->Text("SPELLS", "font", 750, ypos, SDL_Color({ 0,0,0,0 }), Game::Instance()->getRenderer());
		ypos += 15;
		for (int i = 0; i < vbsgn.size();i++) {
			AssetsManager::Instance()->Text(vbsgn[i].name, "font", 5, ypos, SDL_Color({ 0,0,0,0 }), Game::Instance()->getRenderer());
			AssetsManager::Instance()->Text(vbsgn[i].fullName, "font", 120, ypos, SDL_Color({ 0,0,0,0 }), Game::Instance()->getRenderer());
			AssetsManager::Instance()->Text(vbsgn[i].textureFileName, "font", 220, ypos, SDL_Color({ 0,0,0,0 }), Game::Instance()->getRenderer());
			AssetsManager::Instance()->Text(vbsgn[i].description, "font", 400, ypos, SDL_Color({ 0,0,0,0 }), Game::Instance()->getRenderer());
			int tempxpos = 750;
			std::string tempText = "";
			for (int j = 0; j < vbsgn[i].spell_ability.size(); j++) {
				tempText += vbsgn[i].spell_ability[j];
				if (j != vbsgn[i].spell_ability.size() - 1) tempText += ", ";
			}
			AssetsManager::Instance()->Text(tempText, "font", tempxpos, ypos, SDL_Color({ 0,0,0,0 }), Game::Instance()->getRenderer());
			ypos += 15;
		}
	}
}birthSignVisualizer;