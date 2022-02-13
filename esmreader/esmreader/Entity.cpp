#include "Entity.h"
#include "InputHandler.h"
#include "game.h"
#include "Vector2D.h"
#include <algorithm>

float DEGTORAD = 0.017453f; //pi/180

void Entity::handleEvents() {
	//default function
}

void Entity::update()
{
	if (m_name == "explosion")
		m_currentFrame++;
	else
		m_currentFrame = int(((SDL_GetTicks() / (100)) % m_numFrames));
}

void Entity::draw()
{
	AssetsManager::Instance()->drawFrame(m_textureID, m_position.m_x, m_position.m_y, m_width, m_height, 
		m_currentRow, m_currentFrame, Game::Instance()->getRenderer(), m_angle, m_alpha, SDL_FLIP_NONE);
}

void  asteroid::update()
{
	m_position.m_x += m_velocity.m_x;
	m_position.m_y += m_velocity.m_y;

	if (m_position.m_x > Game::Instance()->getGameWidth()) m_position.m_x = 0;
	if (m_position.m_x < 0) m_position.m_x = Game::Instance()->getGameWidth();
	if (m_position.m_y > Game::Instance()->getGameHeight()) m_position.m_y = 0;
	if (m_position.m_y < 0) m_position.m_y = Game::Instance()->getGameHeight();

	m_currentFrame = int(((SDL_GetTicks() / (100)) % m_numFrames));
}

void bullet::update()
{
	m_velocity.m_x = cos(m_angle*DEGTORAD) * 6;
	m_velocity.m_y = sin(m_angle*DEGTORAD) * 6;
	// angle+=rand()%6-3;
	m_position.m_x += m_velocity.m_x;
	m_position.m_y += m_velocity.m_y;

	if (m_position.m_x > Game::Instance()->getGameWidth() || m_position.m_x<0 || m_position.m_y>Game::Instance()->getGameHeight() || m_position.m_y < 0) m_life = 0;

	m_currentFrame = int(((SDL_GetTicks() / (100)) % m_numFrames));
}

void bullet::draw()
{
	AssetsManager::Instance()->drawFrame(m_textureID, m_position.m_x, m_position.m_y, m_width, m_height,
		m_currentRow, m_currentFrame, Game::Instance()->getRenderer(), m_angle+90, m_alpha, SDL_FLIP_NONE);
}

void player::handleEvents(){
	if (InputHandler::Instance()->isKeyDown(SDL_SCANCODE_RIGHT)) m_velocity.m_x = 2;
	if (InputHandler::Instance()->isKeyDown(SDL_SCANCODE_LEFT)) m_velocity.m_x = -2;
	if (InputHandler::Instance()->isKeyDown(SDL_SCANCODE_UP)) m_velocity.m_y = -2;
	if (InputHandler::Instance()->isKeyDown(SDL_SCANCODE_DOWN)) m_velocity.m_y = 2;
}

void player::update()
{
	m_position += m_velocity;

	m_velocity = Vector2D(0, 0);
	if (m_position.m_x > Game::Instance()->getGameWidth()) m_position.m_x = Game::Instance()->getGameWidth();
	if (m_position.m_x < 0) m_position.m_x = 0;
	if (m_position.m_y > Game::Instance()->getGameHeight()) m_position.m_y = Game::Instance()->getGameHeight();
	if (m_position.m_y < 0) m_position.m_y = 0;
}

void player::draw()
{
	AssetsManager::Instance()->drawFrame(m_textureID, m_position.m_x, m_position.m_y, m_width, m_height, 
		m_currentRow, m_currentFrame, Game::Instance()->getRenderer(), m_angle, m_alpha, SDL_FLIP_NONE);

	if (m_shield)
	{
		AssetsManager::Instance()->drawFrame("shield", m_position.m_x, m_position.m_y, 40, 40, 0, 0, 
			Game::Instance()->getRenderer(), 0, m_alpha / 2, SDL_FLIP_NONE);
	}
}

void car::update()
{
	m_position.m_y += m_velocity.m_y;

	if (m_position.m_y < 0)
	{
		m_position.m_y = Game::Instance()->getGameHeight();
	}
	else
	{
		if (m_position.m_y > Game::Instance()->getGameHeight())
			m_position.m_y = 0;
	}
}

void Button::handleEvents(){
	/*if (InputHandler::Instance()->isKeyDown(SDL_SCANCODE_RIGHT)) m_velocity.m_x = 2;
	if (InputHandler::Instance()->isKeyDown(SDL_SCANCODE_LEFT)) m_velocity.m_x = -2;
	if (InputHandler::Instance()->isKeyDown(SDL_SCANCODE_UP)) m_velocity.m_y = -2;
	if (InputHandler::Instance()->isKeyDown(SDL_SCANCODE_DOWN)) m_velocity.m_y = 2;*/
	mouseOver = false;
	if(Game::Instance()->mouseClicked)
	{
		Vector2D* v = InputHandler::Instance()->getMousePosition();
		if ( v->getX() > m_position.m_x &&  v->getX() < m_position.m_x + m_width && v->getY() > m_position.m_y && v->getY() < m_position.m_y + m_height )
		{
			AssetsManager::Instance()->playSound("bok",0);
			InputHandler::Instance()->setMouseButtonStatesToFalse();
			Game::Instance()->mouseClicked = false;
			Game::Instance()->lastButtonClicked = m_text;
		}
	}
	else
	{
		Vector2D* v = InputHandler::Instance()->getMousePosition();
		if (v->getX() > m_position.m_x &&  v->getX() < m_position.m_x + m_width && v->getY() > m_position.m_y && v->getY() < m_position.m_y + m_height)
		{
			mouseOver = true;
		}
	}
}

void Button::update()
{
}

void Button::draw()
{
	/* AssetsManager::Instance()->drawFrame(m_textureID, m_position.m_x, m_position.m_y, m_width, m_height, 
		m_currentRow, m_currentFrame, Game::Instance()->getRenderer(), m_angle, m_alpha, SDL_FLIP_NONE); */
	Uint8 r, g, b, a;
	SDL_GetRenderDrawColor(Game::Instance()->getRenderer(), &r, &g, &b, &a);

	SDL_SetRenderDrawColor(Game::Instance()->getRenderer(), 128, 128, 128, 0);
	if (Game::Instance()->lastButtonClicked == m_text) SDL_SetRenderDrawColor(Game::Instance()->getRenderer(), 255, 234, 0, 0);
	SDL_Rect* rect = new SDL_Rect();
	rect->x = m_position.m_x;
	rect->y = m_position.m_y;
	rect->w = m_width;
	rect->h = m_height;
	SDL_RenderFillRect(Game::Instance()->getRenderer(), rect);
	delete(rect);

	SDL_SetRenderDrawColor(Game::Instance()->getRenderer(), r, g, b, a);
	SDL_RenderDrawLine(Game::Instance()->getRenderer(), m_position.m_x, m_position.m_y, m_position.m_x + m_width, m_position.m_y);
	SDL_RenderDrawLine(Game::Instance()->getRenderer(), m_position.m_x, m_position.m_y, m_position.m_x , m_position.m_y + m_height);
	SDL_RenderDrawLine(Game::Instance()->getRenderer(), m_position.m_x + m_width, m_position.m_y, m_position.m_x + m_width, m_position.m_y + m_height);
	SDL_RenderDrawLine(Game::Instance()->getRenderer(), m_position.m_x, m_position.m_y + m_height, m_position.m_x + m_width, m_position.m_y + m_height);
	SDL_Color cl = { 0,0,0,0 };
	if (mouseOver) cl = { 0,0,255,0 };
	AssetsManager::Instance()->Text(m_text, "font", m_position.m_x + 5, m_position.m_y + 5, cl, Game::Instance()->getRenderer());
}

void Button::autoSize()
{
	int width = 0, height = 0;
	TTF_SizeText(AssetsManager::Instance()->getFont("font"), m_text.c_str(), &width, &height);
	m_width = width + 10;
	m_height = height + 10;
}

void Button::buttonSettings(const string &Texture, Vector2D pos, Vector2D vel, int Width, int Height, int nFrames, 
						int row, int cframe, double Angle, int radius, std::string text, std::string font, bool autosize){
	m_text = text;
	m_font = font;
	settings(Texture, pos,vel, Width, Height, nFrames, row, cframe, Angle, radius);
	if( autosize == true) autoSize();
}

void ShowControl::draw()
{
	//draw headers
	SDL_SetRenderDrawColor(Game::Instance()->getRenderer(), 211, 211, 211, 0); //light gray
	SDL_Rect* rect = new SDL_Rect();
	rect->x = 0;
	rect->y = 150;
	rect->w = 1024;
	rect->h = 22;
	SDL_RenderFillRect(Game::Instance()->getRenderer(), rect);
	delete(rect);
	int xhead = m_position.m_x;
	for (int i = 0; i < headers.size();i++) {
		AssetsManager::Instance()->Text(headers[i], "font", xhead, m_position.m_y + 5, SDL_Color({ 0,0,0,0 }), Game::Instance()->getRenderer());
		xhead += sizes[i];
	}

	int ydata = m_position.m_y + 20;
	//draw data
	xhead = m_position.m_x;
	for (int i = dataFrom; i < data.size(); i++) {
		for (int j = 0; j < data[i].size(); j++) {
			SDL_Color cl = SDL_Color({ 0,0,0,0 });
			if (mouseOn)
			{
				if (mouseOver.m_y >= ydata && mouseOver.m_y < ydata + 20) cl = SDL_Color({ 0,0,255,0 });
			}
			if (selected != -1)
			{
				if (selected >= ydata && selected < ydata + 20)
				{
					cl = SDL_Color({ 255,0,0,0 });
					selectedIndex = i;
				}
			}
			AssetsManager::Instance()->Text(data[i][j], "font", xhead, ydata, cl, Game::Instance()->getRenderer());
			xhead += sizes[j];
			if (xhead > 1024) break;
		}
		xhead = m_position.m_x;
		ydata += 20;
		if (ydata > 768) break;
	}
	mouseOn = false;

	SDL_RenderDrawLine(Game::Instance()->getRenderer(), 0, 150 - 1, 1024, 150 - 1);

	//draw horizontal scroll bar if necessary
	if (showHScroll)
	{
		rect = new SDL_Rect();
		rect->x = 0;
		rect->y = 768 - 20;
		rect->w = 1024;
		rect->h = 20;
		SDL_RenderFillRect(Game::Instance()->getRenderer(), rect);
		delete(rect);
		AssetsManager::Instance()->draw("arrow-left", 0, 768-20, 20, 20, Game::Instance()->getRenderer(), SDL_FLIP_NONE);
		AssetsManager::Instance()->draw("arrow-right", 1024-40, 768-20, 20, 20, Game::Instance()->getRenderer(), SDL_FLIP_NONE);
	}
	//draw vertical scroll bar if necessary
	if (showVScroll)
	{
		rect = new SDL_Rect();
		rect->x = 1004;
		rect->y = 150;
		rect->w = 20;
		rect->h = 768-150;
		SDL_RenderFillRect(Game::Instance()->getRenderer(), rect);
		delete(rect);
		AssetsManager::Instance()->draw("arrow-up", 1004, 150, 20, 20, Game::Instance()->getRenderer(), SDL_FLIP_NONE);
		AssetsManager::Instance()->draw("arrow-down", 1004, 768-40, 20, 20, Game::Instance()->getRenderer(), SDL_FLIP_NONE);
	}

	//draw icon or texture if necessary
	if (selected != -1)
	{
		if (textureColumn != -1)
		{
			if (!AssetsManager::Instance()->textureMapContains(data[selectedIndex][0]))
			{
				AssetsManager::Instance()->clearFromTextureMap(textureId); //remove the later
				textureId = data[selectedIndex][0];
				std::string path = "C:/JuegosEstudio/Morrowind/Data Files/Textures/";
				std::string fullPath = path + data[selectedIndex][textureColumn];
				AssetsManager::Instance()->loadTexture(fullPath, textureId, Game::Instance()->getRenderer());
			}
			AssetsManager::Instance()->draw(textureId, 560, 0, 256, 150, Game::Instance()->getRenderer(), SDL_FLIP_NONE);
		}

		if (iconColumn != -1)
		{
			if (!AssetsManager::Instance()->textureMapContains(data[selectedIndex][0]))
			{
				AssetsManager::Instance()->clearFromTextureMap(iconId); //remove the later
				iconId = data[selectedIndex][0];
				std::string path = "C:/JuegosEstudio/Morrowind/Data Files/Icons/";
				std::string fullPath = path + data[selectedIndex][iconColumn];
				AssetsManager::Instance()->loadTexture(fullPath, iconId, Game::Instance()->getRenderer());
			}
			AssetsManager::Instance()->draw(iconId, 560, 0, 64, 64, Game::Instance()->getRenderer(), SDL_FLIP_NONE);
		}

		if (soundColumn != -1)
		{
			if (!AssetsManager::Instance()->soundMapContains(data[selectedIndex][0]))
			{
				AssetsManager::Instance()->clearSound(soundId); //remove the later
				soundId = data[selectedIndex][0];
				std::string path = "C:/JuegosEstudio/Morrowind/Data Files/Sound/";
				std::string fullPath = path + data[selectedIndex][soundColumn];
				AssetsManager::Instance()->loadSound(fullPath, soundId, SOUND_SFX);
				AssetsManager::Instance()->playSound(soundId, 0);
			}
		}
	}
}

void ShowControl::update() {

}

void ShowControl::handleEvents() {

	if (showVScroll)
	{
		//keyboard
		if (InputHandler::Instance()->isKeyDown(SDL_SCANCODE_DOWN))
		{
			if (dataFrom < data.size() - 25)
			{
				dataFrom++;
				if (selected != -1) selected -= 20;
			}
		}
		if (InputHandler::Instance()->isKeyDown(SDL_SCANCODE_UP))
		{
			if (dataFrom > 0)
			{
				dataFrom--;
				if (selected != -1) selected += 20;
			}
		}
		//mouse
		if (Game::Instance()->mouseClicked)
		{
			Vector2D* v = InputHandler::Instance()->getMousePosition();
			//arrow-up
			if (v->getX() > 1004 && v->getX() < 1004 + 20 && v->getY() > 150 && v->getY() < 150 + 20)
			{
				if (dataFrom > 0)
				{
					dataFrom--;
					if (selected != -1) selected += 20;
				}
			}
			//arrow-down
			if (v->getX() > 1004 && v->getX() < 1004 + 20 && v->getY() > 768 - 40 && v->getY() < 768 - 40 + 20)
			{
				if (dataFrom < data.size() - 25)
				{
					dataFrom++;
					if (selected != -1) selected -= 20;
				}
			}
		}
		//mouse wheel
		if (InputHandler::Instance()->isMouseWheelUp())
		{
			if (dataFrom > 0)
			{
				dataFrom--;
				if (selected != -1) selected += 20;
			}
		}
		if (InputHandler::Instance()->isMouseWheelDown())
		{
			if (dataFrom < data.size() - 25)
			{
				dataFrom++;
				if (selected != -1) selected -= 20;
			}
		}
		InputHandler::Instance()->setMouseWheelToFalse();
	}

	if (showHScroll)
	{
		//keyboard
		if (InputHandler::Instance()->isKeyDown(SDL_SCANCODE_LEFT))	if (m_position.m_x + totalWidth > 0) m_position.m_x -= 20;
		if (InputHandler::Instance()->isKeyDown(SDL_SCANCODE_RIGHT)) if(m_position.m_x < 1024) m_position.m_x += 20;
		//mouse
		//AssetsManager::Instance()->draw("arrow-left", 0, 768 - 20, 20, 20, Game::Instance()->getRenderer(), SDL_FLIP_NONE);
		//AssetsManager::Instance()->draw("arrow-right", 1024 - 40, 768 - 20, 20, 20, Game::Instance()->getRenderer(), SDL_FLIP_NONE);
		if (Game::Instance()->mouseClicked)
		{
			Vector2D* v = InputHandler::Instance()->getMousePosition();
			//arrow-left
			if (v->getX() > 0 && v->getX() < 20 && v->getY() > 768 - 20 && v->getY() < 768) if (m_position.m_x + totalWidth > 0) m_position.m_x -= 20;
			//arrow-right
			if (v->getX() > 1004 - 40 && v->getX() < 1024 - 40 + 20 && v->getY() > 768 - 20 && v->getY() < 768) if (m_position.m_x < 1024) m_position.m_x += 20;
		}
	}

	Vector2D* mpos = InputHandler::Instance()->getMousePosition();
	if (mpos->m_x > 0 && mpos->m_x < 1024 && mpos->m_y > 150 && mpos->m_y < 768)
	{
		mouseOver = *mpos;
		mouseOn = true;
		if (Game::Instance()->mouseClicked)
		{
			//avoid scroll arrows
			if(mouseOver.m_x < 1024 - 20 && mouseOver.m_y < 768 - 20)
				selected = mouseOver.m_y;
		}
	}

	//InputHandler::Instance()->setMouseButtonStatesToFalse();
	//Game::Instance()->mouseClicked = false;
}

void ShowControl::setData(std::vector<std::string>& h, std::vector< std::vector<std::string> >& d) {
	//reset values
	reset();

	//set headers
	headers = h;
	for (int i = 0; i < headers.size(); i++) {
		sizes.push_back(0); //columns default size
		totalWidth += 0;
	}

	//set data
	data = d;
	//resize the columns to fit the biggest data
	int col = 0;
	for (int i = 0; i < sizes.size(); i++) {
		int maxSize = headers[i].size();  //headers counts too.
		int index = -1;
		for (int j = 0; j < d.size(); j++) {
			if (d[j][col].size() > maxSize)
			{
				maxSize = d[j][col].size();
				index = j;
			}
		}

		int width = 0, height = 0;
		//check if the biggest is the header
		if (index == -1)
			TTF_SizeText(AssetsManager::Instance()->getFont("font"), headers[i].c_str(), &width, &height);
		else
			TTF_SizeText(AssetsManager::Instance()->getFont("font"), d[index][col].c_str(), &width, &height);

		sizes[i] = width + 25;
		col++;
	}

	//recalculate totalWidth
	totalWidth = 0;
	for (auto x : sizes) totalWidth += x;
	if (totalWidth > 1024) showHScroll = true;

	//set how many rows to show
	dataFrom = 0;
	dataTo = std::min(29, (int)d.size());
	if ((int)d.size() > 29) showVScroll = true;
}