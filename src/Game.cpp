#include "Game.h"
#include<cmath>


Game::Game(const std::string& config)
{
	init(config);
}

void Game::init(const std::string& path)
{
	
	std::fstream configFile("config.txt");
	if (configFile.fail())
	{
		std::cerr << "Error: Could not open file!\n";
		exit(EXIT_FAILURE);
	}

	std::string word;
	while (configFile >> word)
	{
		if (word == "Window")
		{
			configFile >> m_windowConfig.W >> m_windowConfig.H >> m_windowConfig.FL >> m_windowConfig.FS;
		}
		if (word == "Font")
		{
			configFile >> m_fontConfig.F >> m_fontConfig.fontSize >> m_fontConfig.fontR >> m_fontConfig.fontG >> m_fontConfig.fontB;
		}
		if (word == "Player")
		{
			configFile >> m_playerConfig.SR >> m_playerConfig.CR >> m_playerConfig.S >> m_playerConfig.FR >> m_playerConfig.FG >> m_playerConfig.FB;
			configFile >> m_playerConfig.OR >> m_playerConfig.OG >> m_playerConfig.OB >> m_playerConfig.OT >> m_playerConfig.V;
		}
		if (word=="Enemy")
		{
			configFile >> m_enemyConfig.SR >> m_enemyConfig.CR >> m_enemyConfig.SMIN >> m_enemyConfig.SMAX >> m_enemyConfig.OR >> m_enemyConfig.OG >> m_enemyConfig.OB;
			configFile >> m_enemyConfig.OT >> m_enemyConfig.VMIN >> m_enemyConfig.VMAX >> m_enemyConfig.L >> m_enemyConfig.SI;
		}
		if (word == "Bullet")
		{
			configFile >> m_bulletConfig.SR >> m_bulletConfig.CR >> m_bulletConfig.S >> m_bulletConfig.FR >> m_bulletConfig.FG >> m_bulletConfig.FB;
			configFile >> m_bulletConfig.OR >> m_bulletConfig.OG >> m_bulletConfig.OB >> m_bulletConfig.OT >> m_bulletConfig.V >> m_bulletConfig.L;
		}
	}

	
	if (m_windowConfig.FS) 
	{
		m_window.create(sf::VideoMode::getFullscreenModes()[0], "Fullscreen ECS game", sf::Style::Fullscreen);
	}
	else { m_window.create(sf::VideoMode(m_windowConfig.W, m_windowConfig.H), "ECS game",sf::Style::Default); }
	m_window.setVerticalSyncEnabled(true);
	m_window.setFramerateLimit(m_windowConfig.FL);

	// Load font
	if (!m_font.loadFromFile(m_fontConfig.F))
	{
		std::cerr << "Error: Font file not found or couldn't be loaded: " << m_fontConfig.F << std::endl;
		exit(EXIT_FAILURE);
	}


	//Load Background
	if (!m_texture.loadFromFile("galaxy.png"))
	{
		std::cout << "Unable to open jpeg file!\n";
		exit(EXIT_FAILURE);
	}

	m_sprite.setTexture(m_texture);
	float scaleX = m_window.getView().getSize().x / m_texture.getSize().x;
	float scaleY = m_window.getView().getSize().y / m_texture.getSize().y;
	m_sprite.setScale(scaleX, scaleY);

	spawnPlayer();
}

void Game::spawnPlayer()
{
	
	Vec2 position(m_window.getView().getSize().x / 2, m_window.getView().getSize().y / 2);

	//player spawn
	auto entity = m_entities.addEntity("player");
	entity->cTransform = std::make_shared<CTransform>(position, Vec2(m_playerConfig.S,m_playerConfig.S), 0.0f);
	entity->cShape = std::make_shared<CShape>(m_playerConfig.SR, m_playerConfig.V, sf::Color(m_playerConfig.FR,m_playerConfig.FG,m_playerConfig.FB), sf::Color(m_playerConfig.OR, m_playerConfig.OG,m_playerConfig.OB ),m_playerConfig.OT);
	entity->cInput = std::make_shared<CInput>();
	entity->cCollision = std::make_shared<CCollision>(m_playerConfig.CR);
	m_player = entity;
}

void Game::spawnEnemy()
{
	//random position
	Vec2 position(m_enemyConfig.SR+ std::rand() % (m_windowConfig.W-m_enemyConfig.SR),m_enemyConfig.SR+ std::rand() % (m_windowConfig.H-m_enemyConfig.SR));
	
	//random velocity
	int sMax = m_enemyConfig.SMAX, sMin = m_enemyConfig.SMIN;
	Vec2 vel(sMin + std::srand() % (sMax-sMin+1), sMin + std::srand() % (sMax - sMin + 1));
	
	//random points or vertices
	int diff = m_enemyConfig.VMAX - m_enemyConfig.VMIN+1;
	int points = m_enemyConfig.VMIN + std::rand() % diff;
	
	//random color
	int r = std::rand() % 255;
	int g = std::rand() % 255;
	int b = std::rand() % 255;
	
	//entity creation
	auto entity = m_entities.addEntity("enemy");
	entity->cTransform = std::make_shared<CTransform>(position, vel, 0.0f);
	entity->cShape = std::make_shared<CShape>(m_enemyConfig.SR,points, sf::Color(r,g,b), sf::Color(m_enemyConfig.OR,m_enemyConfig.OG,m_enemyConfig.OB), m_enemyConfig.OT);		
	entity->cCollision = std::make_shared<CCollision>(m_enemyConfig.CR);
	entity->cScore = std::make_shared<CScore>(points * 100);
}

void Game::spawnSmallEnemies(std::shared_ptr<Entity>& e)
{
	int vertices = e->cShape->circle.getPointCount();
	float a = (2* acos(-1)) / vertices;
	float smallRadius = e->cShape->circle.getRadius()/2.0f;
	float collisionRadius = e->cCollision->radius / 2;

	for (int i = 0; i < vertices; i++)
	{	
		float angle = a * i;
		Vec2 vel(cos(angle)* e->cTransform->velocity.x, sin(angle)* e->cTransform->velocity.y);
		auto smallEnemy = m_entities.addEntity("small");
		smallEnemy->cShape = e->cShape;
		smallEnemy->cShape->circle.setRadius(smallRadius);
		smallEnemy->cTransform = std::make_shared<CTransform>(e->cTransform->pos,vel,0.0f);
		smallEnemy->cLifespan = std::make_shared<CLifespan>(m_enemyConfig.L);
		smallEnemy->cCollision = std::make_shared<CCollision>(smallRadius);
		smallEnemy->cScore = std::make_shared<CScore>(collisionRadius);
		//std::cout << "angle: " << angle << "small radius 1: " << smallEnemy->cShape->circle.getRadius()<<std::endl;
	}
}

void Game::spawnBullet(std::shared_ptr<Entity>& e, const Vec2& target) {
	auto entity = m_entities.addEntity("bullet");
	Vec2 position(e->cShape->circle.getPosition().x, e->cShape->circle.getPosition().y);

	// Calculate direction vector towards the mouse pointer
	Vec2 direction = target - position;
	float magnitude = e->cTransform->pos.dist(target);
	if (magnitude != 0) { // Avoid division by zero
		direction.x /= magnitude;
		direction.y /= magnitude;
	}

	// Set the velocity vector based on the normalized direction
	Vec2 vel = direction * Vec2(m_bulletConfig.S,m_bulletConfig.S);
	float angle = std::atan2f(direction.y, direction.x);

	entity->cTransform = std::make_shared<CTransform>(position, vel, angle);
	entity->cShape = std::make_shared<CShape>(m_bulletConfig.SR, m_bulletConfig.V, 
												sf::Color(m_bulletConfig.FR, m_bulletConfig.FG, m_bulletConfig.FB,255),
												sf::Color(m_bulletConfig.OR, m_bulletConfig.OG, m_bulletConfig.OB,255),
												m_bulletConfig.OT);

	entity->cLifespan = std::make_shared<CLifespan>(m_bulletConfig.L);
	entity->cCollision = std::make_shared<CCollision>(m_bulletConfig.CR);
}

void Game::spawnSpecialWeapon(std::shared_ptr<Entity>& e)
{
	for (auto enemy : m_entities.getEntities("enemy"))
	{
		Vec2 pos(enemy->cShape->circle.getPosition().x, enemy->cShape->circle.getPosition().y);
		spawnBullet(e, pos);
	}
}

void Game::sMovement()
{
	Vec2 playerVelocity(0.0f, 0.0f);
	//restricting player within window
	if (m_player->cInput->left && m_player->cTransform->pos.x - m_playerConfig.SR > 0)
	{
		playerVelocity.x -= m_playerConfig.S;
	}
	if (m_player->cInput->right && m_player->cTransform->pos.x + m_playerConfig.SR < m_window.getSize().x)
	{
		playerVelocity.x += m_playerConfig.S;
	}
	if (m_player->cInput->down && m_player->cTransform->pos.y + m_playerConfig.SR < m_window.getSize().y)
	{
		playerVelocity.y += m_playerConfig.S;
	}
	if (m_player->cInput->up && m_player->cTransform->pos.y - m_playerConfig.SR > 0)
	{
		playerVelocity.y -= m_playerConfig.S;
	}
	m_player->cTransform->velocity = playerVelocity;

	for (auto e : m_entities.getEntities())
	{
		e->cTransform->pos += e->cTransform->velocity;
	}
}

void Game::sLifespan()
{
	for (auto e : m_entities.getEntities())
	{
		if (!e->cLifespan) { continue; }

		if (e->cLifespan->remaining > 0)
		{
			e->cLifespan->remaining--; 
			if (e->isActive())
			{
				auto color = m_player->cShape->circle.getFillColor();
				int newAlphaColor = 255;
				sf::Color newColor(color.r, color.g, color.b, newAlphaColor);
				m_player->cShape->circle.setFillColor(newColor);
			}

		}
		else
		{
			e->destroy();
		}

	}
}

void Game::sCollision()
{

	//collision between player and enemy
	for (auto p : m_entities.getEntities("player"))
	{
		for (auto e : m_entities.getEntities("enemy"))
		{
			float dist = p->cTransform->pos.dist(e->cTransform->pos);
			float distanceBetweenRadii = p->cShape->circle.getRadius() + e->cShape->circle.getRadius();
			if (distanceBetweenRadii > dist)
			{
				m_score = 0;
				p->destroy();
			}
		}
	}

	//collision between bullet and enemy
	for (auto b : m_entities.getEntities("bullet"))
	{
		for (auto e : m_entities.getEntities("enemy"))
		{
			float bulletDistance = b->cTransform->pos.dist(e->cTransform->pos);
			if (b->cCollision->radius + e->cCollision->radius > bulletDistance)
			{
				spawnSmallEnemies(e);
				m_score += e->cScore->score;
				e->destroy();
				b->destroy();
			}
		}
	}

	// Collision between enemy and walls
	wallCollision("enemy");

	//Collision between small enemy and walls
	wallCollision("small");

	//Collision between small enemy and bullet
	for (auto b : m_entities.getEntities("bullet"))
	{
		for (auto s : m_entities.getEntities("small"))
		{
			float bulletDistance = b->cTransform->pos.dist(s->cTransform->pos);
			if (b->cCollision->radius + s->cCollision->radius > bulletDistance)
			{
				m_score += s->cScore->score;
				s->destroy();
				b->destroy();
			}
		}
	}


}

void Game::wallCollision(std::string s)
{
	for (auto e : m_entities.getEntities(s)) 
	{
		if (e->cTransform->pos.x - e->cShape->circle.getRadius() < 0 ||
			e->cTransform->pos.x + e->cShape->circle.getRadius() > m_window.getSize().x) 
		{
			// Collision with horizontal walls (left or right edges)
			e->cTransform->velocity.x = -e->cTransform->velocity.x;
		}

		if (e->cTransform->pos.y - e->cShape->circle.getRadius() < 0 ||
			e->cTransform->pos.y + e->cShape->circle.getRadius() > m_window.getSize().y) 
		{
			// Collision with vertical walls (top or bottom edges)
			e->cTransform->velocity.y = -e->cTransform->velocity.y;
		}
	}
}

void Game::sEnemySpawner()
{
	m_lastEnemySpawnTime++;
	if (m_lastEnemySpawnTime >= m_enemyConfig.SI)
	{
		spawnEnemy();
		m_lastEnemySpawnTime = 0;
	}
}

void Game::sRender()
{
	m_window.clear();
	m_window.draw(m_sprite);
	//Set text properties
	m_text.setFont(m_font);
	std::string str = "SCORE: " + std::to_string(m_score);
	if (m_countdown <= m_currentFrame) { str += "\nWEAPON STATUS: READY"; }
	else { str += "\nWEAPON STATUS: NOT READY"; }
	//std::cout << str;
	m_text.setString(str);
	m_text.setCharacterSize(m_fontConfig.fontSize);
	m_text.setFillColor(sf::Color(m_fontConfig.fontR, m_fontConfig.fontG, m_fontConfig.fontB));
	m_text.setStyle(sf::Text::Bold);
	m_window.draw(m_text);

	//Draw Player
	if (!m_player->isActive()) { spawnPlayer(); }

	// Draw enemies
	for (auto e : m_entities.getEntities("enemy"))
	{
		e->cShape->circle.setPosition(e->cTransform->pos.x, e->cTransform->pos.y);
		e->cTransform->angle += 1.0f;
		e->cShape->circle.setRotation(e->cTransform->angle);
		m_window.draw(e->cShape->circle);
	}

	// Draw bullets and update their positions
	for (auto e : m_entities.getEntities("bullet"))
	{
		e->cShape->circle.setPosition(e->cTransform->pos.x, e->cTransform->pos.y);
		// Calculate alpha based on remaining lifespan
		float alpha = static_cast<float>(e->cLifespan->remaining) / static_cast<float>(m_bulletConfig.L) * 255.0f;

		// Apply alpha to fill color and outline color
		sf::Color fillColor = e->cShape->circle.getFillColor();
		sf::Color outlineColor = e->cShape->circle.getOutlineColor();
		fillColor.a = static_cast<sf::Uint8>(alpha);
		outlineColor.a = static_cast<sf::Uint8>(alpha);

		// Set the new colors
		e->cShape->circle.setFillColor(fillColor);
		e->cShape->circle.setOutlineColor(outlineColor);

		m_window.draw(e->cShape->circle);
	}

	// Draw small enemies
	for (auto s:m_entities.getEntities("small"))
	{
		s->cShape->circle.setPosition(s->cTransform->pos.x, s->cTransform->pos.y);

		float alpha = 180;
		// Apply alpha to fill color and outline color
		sf::Color fillColor = s->cShape->circle.getFillColor();
		sf::Color outlineColor = s->cShape->circle.getOutlineColor();
		fillColor.a = static_cast<sf::Uint8>(alpha);
		outlineColor.a = static_cast<sf::Uint8>(alpha);

		// Set the new colors
		s->cShape->circle.setFillColor(fillColor);
		s->cShape->circle.setOutlineColor(outlineColor);

		s->cTransform->angle += 1.0f;
		s->cShape->circle.setRotation(s->cTransform->angle);
		m_window.draw(s->cShape->circle);
	}

	// Draw player
	m_player->cShape->circle.setPosition(m_player->cTransform->pos.x, m_player->cTransform->pos.y);
	m_player->cTransform->angle += 1.0f;
	m_player->cShape->circle.setRotation(m_player->cTransform->angle);
	m_window.draw(m_player->cShape->circle);

	m_window.display();
}


void Game::sUserInput()
{
	sf::Event event;
	while (m_window.pollEvent(event))
	{
		if (event.type == sf::Event::Closed || (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape))
		{
			m_running = false;
		}
		if (event.type == sf::Event::Resized)
		{
			
			sf::FloatRect visArea(0, 0, event.size.width, event.size.height);
			
			// Get the size of the resized window
			sf::Vector2u newSize(event.size.width, event.size.height);

			// Calculate the scale factors to fit the background image to the window size
			float scaleX = static_cast<float>(newSize.x) / m_texture.getSize().x;
			float scaleY = static_cast<float>(newSize.y) / m_texture.getSize().y;

			// Set the scale of the background sprite to cover the entire window
			m_sprite.setScale(scaleX, scaleY);

			//player position
			float playerScaleX = newSize.x / m_window.getSize().x;
			float playerScaleY = newSize.y /m_window.getSize().y;
			m_player->cShape->circle.setPosition(playerScaleX* m_player->cTransform->pos.x, playerScaleY*m_player->cTransform->pos.y);
			
			
			m_window.setView(sf::View(visArea));
		}
		if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::P)
		{
			m_paused = true;
		}
		if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::R)
		{
			m_paused = false;
		}
		if (event.type == sf::Event::KeyPressed) {
			switch (event.key.code)
			{
			case sf::Keyboard::W:
				m_player->cInput->up = true;
				break;
			case sf::Keyboard::S:
				m_player->cInput->down = true;
				break;
			case sf::Keyboard::A:
				m_player->cInput->left = true;
				break;
			case sf::Keyboard::D:
				m_player->cInput->right = true;
				break;
			default:
				break;
			}
		}
		if (event.type == sf::Event::KeyReleased) {
			switch (event.key.code)
			{
			case sf::Keyboard::W:
				m_player->cInput->up = false;
				break;
			case sf::Keyboard::S:
				m_player->cInput->down = false;
			case sf::Keyboard::A:
				m_player->cInput->left = false;
				break;
			case sf::Keyboard::D:
				m_player->cInput->right = false;
				break;
			default:
				break;
			}
		}
		if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left)
		{
			if (m_player->isActive()) { spawnBullet(m_player, Vec2(sf::Mouse::getPosition(m_window).x, sf::Mouse::getPosition(m_window).y)); }
		}
		if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Right) 
		{

			if (m_countdown < m_currentFrame && m_player->isActive())
			{
				spawnSpecialWeapon(m_player);
				m_countdown = m_currentFrame + 600;
			}
		}
	}
}

void Game::run()
{

	while (m_running)
	{

		m_entities.update();
		sUserInput();
		sRender();
		if (!m_paused)
		{
			sEnemySpawner();
			sMovement();
			sCollision();
			
			sLifespan();
		}
		

		
		m_currentFrame++;
	}
}

