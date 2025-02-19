#include "ModelCylinder.hpp"

#include "Maths/Maths.hpp"
#include "Resources/Resources.hpp"
#include "Models/Vertex3d.hpp"

namespace acid {
bool ModelCylinder::registered = Register("cylinder");

std::shared_ptr<ModelCylinder> ModelCylinder::Create(const Node &node) {
	if (auto resource = Resources::Get()->Find(node)) {
		return std::dynamic_pointer_cast<ModelCylinder>(resource);
	}

	auto result = std::make_shared<ModelCylinder>(0.0f, 0.0f);
	Resources::Get()->Add(node, std::dynamic_pointer_cast<Resource>(result));
	node >> *result;
	result->Load();
	return result;
}

std::shared_ptr<ModelCylinder> ModelCylinder::Create(float radiusBase, float radiusTop, float height, uint32_t slices, uint32_t stacks) {
	ModelCylinder temp(radiusBase, radiusTop, height, slices, stacks, false);
	Node node;
	node << temp;
	return Create(node);
}

ModelCylinder::ModelCylinder(float radiusBase, float radiusTop, float height, uint32_t slices, uint32_t stacks, bool load) :
	m_radiusBase(radiusBase),
	m_radiusTop(radiusTop),
	m_height(height),
	m_slices(slices),
	m_stacks(stacks) {
	if (load) {
		Load();
	}
}

const Node &operator>>(const Node &node, ModelCylinder &model) {
	node["radiusBase"].Get(model.m_radiusBase);
	node["radiusTop"].Get(model.m_radiusTop);
	node["height"].Get(model.m_height);
	node["slices"].Get(model.m_slices);
	node["stacks"].Get(model.m_stacks);
	return node;
}

Node &operator<<(Node &node, const ModelCylinder &model) {
	node["radiusBase"].Set(model.m_radiusBase);
	node["radiusTop"].Set(model.m_radiusTop);
	node["height"].Set(model.m_height);
	node["slices"].Set(model.m_slices);
	node["stacks"].Set(model.m_stacks);
	return node;
}

void ModelCylinder::Load() {
	if (m_radiusBase == 0.0f && m_radiusTop == 0.0f) {
		return;
	}

	std::vector<Vertex3d> vertices;
	std::vector<uint32_t> indices;
	vertices.reserve((m_slices + 1) * (m_stacks + 1));
	indices.reserve(m_slices * m_stacks * 6);

	for (uint32_t i = 0; i < m_slices + 1; i++) {
		auto iDivSlices = static_cast<float>(i) / static_cast<float>(m_slices);
		auto alpha = (i == 0 || i == m_slices) ? 0.0f : iDivSlices * 2.0f * Maths::Pi<float>;
		auto xDir = std::cos(alpha);
		auto zDir = std::sin(alpha);

		for (uint32_t j = 0; j < m_stacks + 1; j++) {
			auto jDivStacks = static_cast<float>(j) / static_cast<float>(m_stacks);
			auto radius = m_radiusBase * (1.0f - jDivStacks) + m_radiusTop * jDivStacks;

			Vector3f position(xDir * radius, jDivStacks * m_height - (m_height / 2.0f), zDir * radius);
			Vector2f uvs(1.0f - iDivSlices, 1.0f - jDivStacks);
			Vector3f normal(xDir, 0.0f, zDir);
			vertices.emplace_back(Vertex3d(position, uvs, normal));
		}
	}

	for (uint32_t i = 0; i < m_slices; i++) {
		for (uint32_t j = 0; j < m_stacks; j++) {
			auto first = j + ((m_stacks + 1) * i);
			auto second = j + ((m_stacks + 1) * (i + 1));

			indices.emplace_back(first + 1);
			indices.emplace_back(second + 1);
			indices.emplace_back(first);
			indices.emplace_back(second + 1);
			indices.emplace_back(second);
			indices.emplace_back(first);
		}
	}

	Initialize(vertices, indices);
}
}
