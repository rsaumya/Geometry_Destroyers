#pragma once

#include "Entity.h"
#include "EntityManager.h"

#include <SFML/Graphics.hpp>

//W :width, H :Height, FL: Frame limit, FS: fullscreen
struct windowConfig { int W, H, FL; bool FS; };
//F: font file
struct fontConfig { std::string F ="Fonts/arial.ttf"; int fontSize, fontR, fontG, fontB; };
//SR: shape radius, CR: collision radius, S: speed, (FR,FG,FB):fill color
// (OR,OG,OB):outline color,OT:outline thickness,V:shape vertices, SI :spawn interval
struct PlayerConfig { int SR, CR, FR, FG, FB, OR, OG, OB, OT, V; float S; };
struct EnemyConfig { int SR, CR, OR, OG, OB, OT, VMIN, VMAX, L, SI; float SMIN, SMAX; };
struct BulletConfig { int SR, CR, FR, FG, FB, OR, OG, OB, OT, V, L; float S; };

class Game
{
	sf::RenderWindow m_window;
	sf::Texture m_texture;
	sf::Sprite m_sprite;
	EntityManager m_entities;
	sf::Font m_font;
	sf::Text m_text;
	PlayerConfig m_playerConfig;
	EnemyConfig m_enemyConfig;
	BulletConfig m_bulletConfig;
	windowConfig m_windowConfig;
	fontConfig m_fontConfig;
	int m_score = 0;
	int m_currentFrame = 0;
	int m_lastEnemySpawnTime = 0;
	bool m_paused = false;
	bool m_running = true;
	int m_countdown = INT_MIN;

	std::shared_ptr<Entity> m_player;

	void init(const std::string& config); //initialize the GameState with config file path
	void setPaused(bool paused); //pause the game

	void sMovement();
	void sUserInput();
	void sLifespan();
	void sRender();
	void sEnemySpawner();
	void sCollision();

	void spawnPlayer();
	void spawnEnemy();
	void wallCollision(std::string s);
	void spawnSmallEnemies(std::shared_ptr<Entity>& entity);
	void spawnBullet(std::shared_ptr<Entity>& entity, const Vec2& mousePos);
	void spawnSpecialWeapon(std::shared_ptr<Entity>& e);
	

public:

	Game(const std::string& config);

	void run();
};

