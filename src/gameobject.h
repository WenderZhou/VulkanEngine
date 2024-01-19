#pragma once

#include "model.h"

#include <glm/gtc/matrix_transform.hpp>

#include <memory>
#include <unordered_map>

namespace VulkanEngine
{

struct TransformComponent
{
	glm::vec3 translation{};
	glm::vec3 scale{ 1.0f, 1.0f, 1.0f };
	glm::vec3 rotation;

	// Matrix correspond to translate * Ry * Rx * Rz * scale transformation
	glm::mat4 mat4();
	glm::mat3 normalMatrix();
};

struct PointLightComponent
{
	float lightIntensity = 1.0f;
};

class GameObject
{
public:
	using id_t = unsigned int;
	using Map = std::unordered_map<id_t, GameObject>;

	static GameObject createGameObject()
	{
		static id_t currentId = 0;
		return GameObject{ currentId++ };
	}

	static GameObject createPointLight(float intensity, float radius, glm::vec3 color);

	GameObject(const GameObject&) = delete;
	GameObject& operator=(const GameObject&) = delete;
	GameObject(GameObject&&) = default;
	GameObject& operator=(GameObject&&) = default;

	id_t getId() { return id; }

	glm::vec3 color{};
	TransformComponent transform{};

	std::shared_ptr<Model> pModel{};
	std::unique_ptr<PointLightComponent> pPointLightComponent;

private:
	GameObject(id_t id) :id{ id } {}

	id_t id;
};
}