#pragma once
#include "../../BBox3D.h"
#include "../../Intersectable.h"



namespace PrimitiveLocators {
	static const float Epsilon = 0.000001f;
	class IPrimitive: public Intersectable {
	public:
		virtual vec3 position() const = 0;
		virtual AABB bounds() const = 0;
		// optional

	};
	template<class Primitive>
	class Base : public Intersectable {
	protected:
		Base() = default;
		std::vector<Primitive*> primitives;
	public:
		bool isEmpty() const {
			return primitives.size();
		}
	};
}