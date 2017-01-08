#include "vertex.h"

//float: VK_FORMAT_R32_SFLOAT
//vec2 : VK_FORMAT_R32G32_SFLOAT
//vec3 : VK_FORMAT_R32G32B32_SFLOAT
//vec4 : VK_FORMAT_R32G32B32A32_SFLOAT
//	
//
//ivec2: VK_FORMAT_R32G32_SINT, a 2 - component vector of 32 - bit signed integers
//uvec4 : VK_FORMAT_R32G32B32A32_UINT, a 4 - component vector of 32 - bit unsigned integers
//double : VK_FORMAT_R64_SFLOAT, a double - precision(64 - bit) float

VkVertexInputBindingDescription Vertex::getBindingDescribtion()
{
	VkVertexInputBindingDescription bindingDescribtion = {};
	bindingDescribtion.binding = 0;
	bindingDescribtion.stride = sizeof(Vertex);
	bindingDescribtion.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
	
	return bindingDescribtion;
}

std::array<VkVertexInputAttributeDescription, 2> Vertex::getAttributeDescribtions()
{
	std::array<VkVertexInputAttributeDescription, 2> attrDescibs = {};
	//pos
	attrDescibs[0].binding = 0;
	attrDescibs[0].location = 0;
	attrDescibs[0].format = VK_FORMAT_R32G32_SFLOAT;		//vec2
	attrDescibs[0].offset = offsetof(Vertex, pos);

	//color
	attrDescibs[1].binding = 0;
	attrDescibs[1].location = 1;							//set 1 as  a location
	attrDescibs[1].format = VK_FORMAT_R32G32B32_SFLOAT;		//vec3
	attrDescibs[1].offset = offsetof(Vertex, color);

	return attrDescibs;
}

