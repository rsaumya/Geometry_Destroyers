#pragma once

#include "Common.h"
#include "Components.h"

class Entity
{
	friend class EntityManager;
	std::string m_tag		= "default";
	bool		m_active    = true;
	size_t		m_id		= 0;

	Entity(const size_t id, const std::string& tag);

public:

	//Component pointers
	ptr<CTransform> cTransform;
	ptr<CShape> cShape;
	ptr<CCollision> cCollision;
	ptr<CInput> cInput;
	ptr<CScore> cScore;
	ptr<CLifespan> cLifespan;

	bool isActive() const;
	const std::string& tag() const;
	const size_t id() const;
	void destroy();
};

