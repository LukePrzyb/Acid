#include "ModelRectangle.hpp"

#include "Resources/Resources.hpp"
#include "Models/Vertex3d.hpp"

namespace acid {
bool ModelRectangle::registered = Register("rectangle");

std::shared_ptr<ModelRectangle> ModelRectangle::Create(const Node &node) {
	if (auto resource = Resources::Get()->Find(node)) {
		return std::dynamic_pointer_cast<ModelRectangle>(resource);
	}

	auto result = std::make_shared<ModelRectangle>(0.0f, 0.0f);
	Resources::Get()->Add(node, std::dynamic_pointer_cast<Resource>(result));
	node >> *result;
	result->Load();
	return result;
}

std::shared_ptr<ModelRectangle> ModelRectangle::Create(float min, float max) {
	ModelRectangle temp(min, max, false);
	Node node;
	node << temp;
	return Create(node);
}

ModelRectangle::ModelRectangle(float min, float max, bool load) :
	m_min(min),
	m_max(max) {
	if (load) {
		Load();
	}
}

const Node &operator>>(const Node &node, ModelRectangle &model) {
	node["min"].Get(model.m_min);
	node["max"].Get(model.m_max);
	return node;
}

Node &operator<<(Node &node, const ModelRectangle &model) {
	node["min"].Set(model.m_min);
	node["max"].Set(model.m_max);
	return node;
}

void ModelRectangle::Load() {
	if (m_min == m_max) {
		return;
	}

	std::vector<Vertex3d> vertices = {
		{{m_min, m_min, 0.0f}, {0.0f, 0.0f}, {}},
		{{m_max, m_min, 0.0f}, {1.0f, 0.0f}, {}},
		{{m_max, m_max, 0.0f}, {1.0f, 1.0f}, {}},
		{{m_min, m_max, 0.0f}, {0.0f, 1.0f}, {}}
	};
	static std::vector<uint32_t> indices = {
		0, 1, 2,
		2, 3, 0
	};

	Initialize(vertices, indices);
}
}
