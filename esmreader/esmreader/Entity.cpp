#include "Entity.h"
#include "InputHandler.h"
#include "game.h"

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
	std::pair<int, int> p;
	p = AssetsManager::Instance()->getTextSize(m_text, m_font, SDL_Color({ 190,34,12,0 }), Game::Instance()->getRenderer());
	m_width = p.first + 10;
	m_height = p.second + 10;
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
	Uint8 r, g, b, a;
	SDL_GetRenderDrawColor(Game::Instance()->getRenderer(), &r, &g, &b, &a);

	//draw headers
	SDL_SetRenderDrawColor(Game::Instance()->getRenderer(), 211, 211, 211, 0); //light gray
	SDL_Rect* rect = new SDL_Rect();
	rect->x = m_position.m_x;
	rect->y = m_position.m_y;
	rect->w = totalWidth;
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
	for (int i = 0; i < data.size(); i++) {
		for (int j = 0; j < data[i].size(); j++) {
			AssetsManager::Instance()->Text(data[i][j], "font", xhead, ydata, SDL_Color({ 0,0,0,0 }), Game::Instance()->getRenderer());
			xhead += sizes[j];
		}
		xhead = m_position.m_x;
		ydata += 20;
	}

	SDL_SetRenderDrawColor(Game::Instance()->getRenderer(), r, g, b, a);
	SDL_RenderDrawLine(Game::Instance()->getRenderer(), m_position.m_x, m_position.m_y - 1, m_position.m_x + 1024, m_position.m_y - 1);
	//AssetsManager::Instance()->Text(m_text, "font", m_position.m_x + 5, m_position.m_y + 5, cl, Game::Instance()->getRenderer());
}

void ShowControl::update() {

}

void ShowControl::handleEvents() {

}

void ShowControl::setData(std::vector< std::vector<std::string> >& d) {
	data = d;
	//resize the columns to fit the biggest data
	int col = 0;
	for (int i = 0; i < sizes.size(); i++) {
		int maxSize = 0;
		int index = 0;
		for (int j = 0; j < d.size(); j++) {
			if (d[j][col].size() > maxSize)
			{
				maxSize = d[j][col].size();
				index = j;
			}
		}
		std::pair<int, int> p = AssetsManager::Instance()->getTextSize(d[index][col], "font", SDL_Color({ 0,0,0,0 }), Game::Instance()->getRenderer());
		if (col == 1) AssetsManager::Instance()->Text(d[index][col], "font", 800, 700, SDL_Color({ 0,0,0,0 }), Game::Instance()->getRenderer());
		sizes[i] = p.first + 5;
		col++;
	}
}