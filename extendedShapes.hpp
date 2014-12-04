#ifndef EXTENDEDSHAPES_H
#define EXTENDEDSHAPES_H

#include "Aluminum/MeshData.hpp"

namespace aluminum {

	using glm::vec3;

	void addCube(MeshData&, float, vec3);
	void addRect(MeshData&, float, float, float, vec3);

}


#endif // EXTENDEDSHAPES_H
