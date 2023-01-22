#pragma once
#include "Math.h"
#include "vector"

using namespace dae;

struct Vertex
{
	Vector3 Position{};
	Vector2 Uv{};
	Vector3 Normal{};
	Vector3 Tangent{};
};


struct Vertex_Out
{
	Vector4 Position{};
	Vector2 Uv{};
	Vector3 Normal{};
	Vector3 Tangent{};
	Vector3 ViewDirection{};
};

enum class PrimitiveTopology
{
	TriangleList,
	TriangleStrip
};