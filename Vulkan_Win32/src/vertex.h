
#pragma once

#include <vulkan/vulkan.h>
#include <array>
#include <vec2f.h>
#include <vec3f.h>
#include <color.h>

//#define CLASS_TO_PTR(object) typedef class object##_T* object; 
//CLASS_TO_PTR(Vertex)
//
//#define VERTEX_NULL_HANDLE  VK_NULL_HANDLE;

class Vertex
{
public:
	Vertex() : pos(),color(),st(){}
	Vertex(const vec2f &pos, const Color &color, const vec2f &st = vec2f())
		: pos(pos), color(color) ,st(st){}
	vec2f pos;
	Color color;
	vec2f st;

	//static Vertex_T tertex;
	static VkVertexInputBindingDescription getBindingDescribtion();

	static std::array<VkVertexInputAttributeDescription,3>
		getAttributeDescribtions();

};



