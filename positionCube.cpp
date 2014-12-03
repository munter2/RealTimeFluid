#include "Aluminum/Shapes.hpp"

using glm::vec3;

namespace aluminum {

	void addCube(MeshData &m, float d, vec3 center) {

		float x = center.x;
		float y = center.y;
		float z = center.z;

		float s = d/2.f;

		std::cout << "\nPosition Cube:\t(" << x << "," << y << "," << z << ")";
		
		//8 vertices
		vec3 v0 = vec3(-s+x, -s+y, s+z);
		vec3 v1 = vec3(-s+x, s+y, s+z);
		vec3 v2 = vec3(s+x, -s+y, s+z);
		vec3 v3 = vec3(s+x, s+y, s+z);
		vec3 v4 = vec3(-s+x, -s+y, -s+z);
		vec3 v5 = vec3(-s+x, s+y, -s+z);
		vec3 v6 = vec3(s+x, -s+y, -s+z);
		vec3 v7 = vec3(s+x, s+y, -s+z);
		
		//6 normals
		vec3 n0 = vec3(0, 0, -1.0f);
		vec3 n1 = vec3(0, 0, +1.0f);
		vec3 n2 = vec3(0, -1.0f, 0);
		vec3 n3 = vec3(0, +1.0f, 0);
		vec3 n4 = vec3(-1.0f, 0, 0);
		vec3 n5 = vec3(+1.0f, 0, 0);
		
		//6 colors
		/*
		vec3 c0 = vec3(1.0,0.0,0.0);
		vec3 c1 = vec3(0.0,1.0,0.0);
		vec3 c2 = vec3(0.0,0.0,1.0);
		vec3 c3 = vec3(1.0,1.0,0.0);
		vec3 c4 = vec3(1.0,0.0,1.0);
		vec3 c5 = vec3(0.0,1.0,1.0);
		*/

		vec3 c0 = vec3(.3,.3,.3);
		vec3 c1 = c0;
		vec3 c2 = c0;
		vec3 c3 = c0;
		vec3 c4 = c0;
		vec3 c5 = c0;
		
		if(x > 0) c0 = vec3(1,0,0);
		if(x < 0) c1 = vec3(0,1,0);
		if(y > 0) c2 = vec3(0,0,1);
		if(y < 0) c3 = vec3(0,1,1);
		if(z > 0) c4 = vec3(1,0,1);
		if(z < 0) c5 = vec3(1,1,0);
		
		
		//right
		m.vertex(v2);
		m.normal(n4);
		m.color(c4);
		
		m.vertex(v3);
		m.normal(n4);
		m.color(c4);
		
		m.vertex(v6);
		m.normal(n4);
		m.color(c4);
		
		m.vertex(v6);
		m.normal(n4);
		m.color(c4);
		
		m.vertex(v3);
		m.normal(n4);
		m.color(c4);
		
		m.vertex(v7);
		m.normal(n4);
		m.color(c4);
		
		//left
		m.vertex(v4);
		m.normal(n5);
		m.color(c5);
		
		m.vertex(v5);
		m.normal(n5);
		m.color(c5);
		
		m.vertex(v0);
		m.normal(n5);
		m.color(c5);
		
		m.vertex(v0);
		m.normal(n5);
		m.color(c5);
		
		m.vertex(v5);
		m.normal(n5);
		m.color(c5);
		
		m.vertex(v1);
		m.normal(n5);
		m.color(c5);
		
		//front
		m.vertex(v0);
		m.normal(n0);
		m.color(c0);
		
		m.vertex(v1);
		m.normal(n0);
		m.color(c0);
		
		m.vertex(v2);
		m.normal(n0);
		m.color(c0);
		
		m.vertex(v2);
		m.normal(n0);
		m.color(c0);
		
		m.vertex(v1);
		m.normal(n0);
		m.color(c0);
		
		m.vertex(v3);
		m.normal(n0);
		m.color(c0);
		
		
		//back
		m.vertex(v4);
		m.normal(n1);
		m.color(c1);
		
		m.vertex(v5);
		m.normal(n1);
		m.color(c1);
		
		m.vertex(v6);
		m.normal(n1);
		m.color(c1);
		
		m.vertex(v6);
		m.normal(n1);
		m.color(c1);
		
		m.vertex(v5);
		m.normal(n1);
		m.color(c1);
		
		m.vertex(v7);
		m.normal(n1);
		m.color(c1);
		
		
		//top
		m.vertex(v1);
		m.normal(n2);
		m.color(c2);
		
		m.vertex(v5);
		m.normal(n2);
		m.color(c2);
		
		m.vertex(v3);
		m.normal(n2);
		m.color(c2);
		
		m.vertex(v3);
		m.normal(n2);
		m.color(c2);
		
		m.vertex(v5);
		m.normal(n2);
		m.color(c2);
		
		m.vertex(v7);
		m.normal(n2);
		m.color(c2);
		
		
		//bottom
		m.vertex(v0);
		m.normal(n3);
		m.color(c3);
		
		m.vertex(v4);
		m.normal(n3);
		m.color(c3);
		
		m.vertex(v2);
		m.normal(n3);
		m.color(c3);
		
		m.vertex(v2);
		m.normal(n3);
		m.color(c3);
		
		m.vertex(v4);
		m.normal(n3);
		m.color(c3);
		
		m.vertex(v6);
		m.normal(n3);
		m.color(c3);
	}
}

