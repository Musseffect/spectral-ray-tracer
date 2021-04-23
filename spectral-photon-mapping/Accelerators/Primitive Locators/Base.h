#pragma once
#include "../../BBox3D.h"
#include "../../Intersectable.h"



namespace PrimitiveLocators {
	class IPrimitive: public Intersectable {
	public:
		virtual vec3 position() const = 0;
		virtual AABB bounds() const = 0;
		// optional

	};
	template<class Primitive>
	class Base : public Intersectable {
		Base();
	protected:
		std::vector<Primitive*> _primitives;
	public:
		bool isEmpty() const {
			return _primitives.size();
		}
	};
}