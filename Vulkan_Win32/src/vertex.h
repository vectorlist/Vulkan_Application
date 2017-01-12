
#pragma once

#include <vulkan/vulkan.h>
#include <array>
#include <vec2f.h>
#include <vec3f.h>
#include <color.h>

class Vertex
{
public:
	Vertex() : pos(), color(), st() {}
	Vertex(const vec3f &pos, const vec3f &color, const vec2f &st = vec2f())
		: pos(pos), color(color), st(st) {}

	vec3f pos;
	vec3f color;
	vec2f st;

	static VkVertexInputBindingDescription getBindingDescribtion();

	static std::array<VkVertexInputAttributeDescription, 3>
		getAttributeDescribtions();

	bool operator==(const Vertex &other) const {
		return pos == other.pos && color == other.color && st == other.st;
	}
};


inline void hash_combine(size_t &seed, size_t hash)
{
	hash += 0x9e3779b9 + (seed << 6) + (seed >> 2);
	seed ^= hash;
}

namespace std
{
	template<>
	struct hash<vec3f>
	{
		inline size_t operator()(vec3f const &v) const
		{
			size_t seed = 0;
			hash<float> hasher;
			hash_combine(seed, hasher(v.x));
			hash_combine(seed, hasher(v.y));
			hash_combine(seed, hasher(v.z));
			return seed;
		}
	};

	template<>
	struct hash<vec2f>
	{
		inline size_t operator()(vec2f const &v) const
		{
			size_t seed = 0;
			hash<float> hasher;
			hash_combine(seed, hasher(v.x));
			hash_combine(seed, hasher(v.y));
			return seed;
		}
	};

	template<>
	struct hash<Vertex>
	{
		inline size_t operator()(Vertex const & vertex) const
		{
			return (
				(hash<vec3f>()(vertex.pos) ^ 
				(hash<vec3f>()(vertex.color) << 1)) >> 1) ^ 
				(hash<vec2f>()(vertex.st) << 1);
		}
	};
}
