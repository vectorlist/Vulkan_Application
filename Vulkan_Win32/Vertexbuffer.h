
#pragma once

#include <vulkan/vulkan.h>
#include <array>
#include <vec2f.h>
#include <vec3f.h>
#include <color.h>

//#define VK_DEFINE_HANDLE(object) typedef struct object##_T* object;
#define CLASS_TO_PTR(object) typedef class object##_T* object; 
CLASS_TO_PTR(Vertex)

#define VERTEX_NULL_HANDLE  VK_NULL_HANDLE;

class Vertex_T
{
public:
	vec2f pos;
	Color color;

	//static Vertex_T tertex;
	static VkVertexInputBindingDescription getBindingDescribtion();

	static std::array<VkVertexInputAttributeDescription,2>
		getAttributeDescribtions();

	//Vertex& operator()(const Vertex_T &other)
	//{
	//	//this* = other;
	//	return (Vertex_T)*this;
	//}
	static Vertex vtCreateVertexBuffer(const vec2f& pos, const Color &color);
};



