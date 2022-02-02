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
	if(Game::Instance()->mouseClicked)
	{
		Vector2D* v = InputHandler::Instance()->getMousePosition();
		if ( v->getX() > m_position.m_x &&  v->getX() < m_position.m_x + m_width && v->getY() > m_position.m_y && v->getY() < m_position.m_y + m_height )
		{
			AssetsManager::Instance()->playSound("bok",0);
			InputHandler::Instance()->setMouseButtonStatesToFalse();
			Game::Instance()->mouseClicked = false;
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

	SDL_RenderDrawLine(Game::Instance()->getRenderer(), m_position.m_x, m_position.m_y, m_position.m_x + m_width, m_position.m_y);
	SDL_RenderDrawLine(Game::Instance()->getRenderer(), m_position.m_x, m_position.m_y, m_position.m_x , m_position.m_y + m_height);
	SDL_RenderDrawLine(Game::Instance()->getRenderer(), m_position.m_x + m_width, m_position.m_y, m_position.m_x + m_width, m_position.m_y + m_height);
	SDL_RenderDrawLine(Game::Instance()->getRenderer(), m_position.m_x, m_position.m_y + m_height, m_position.m_x + m_width, m_position.m_y + m_height);
	AssetsManager::Instance()->Text(m_text, "font", m_position.m_x, m_position.m_y, SDL_Color({190,34,12,0}), Game::Instance()->getRenderer());
}

void Button::autoSize()
{
	std::pair<int, int> p;
	p = AssetsManager::Instance()->getTextSize(m_text, m_font, SDL_Color({ 190,34,12,0 }), Game::Instance()->getRenderer());
	m_width = p.first;
	m_height = p.second;
}

void Button::buttonSettings(const string &Texture, Vector2D pos, Vector2D vel, int Width, int Height, int nFrames, 
						int row, int cframe, double Angle, int radius, std::string text, std::string font, bool autosize){
	m_text = text;
	m_font = font;
	settings(Texture, pos,vel, Width, Height, nFrames, row, cframe, Angle, radius);
	if( autosize == true) autoSize();
}