#ifndef Camera_hpp
#define Camera_hpp
#include "glm/glm.hpp"
#include <memory>
namespace RenderEngine {
	class Camera
	{
	public:
		typedef std::shared_ptr<Camera> Ptr;
	public:
		glm::vec3 position;
		glm::vec3 target;
		Camera()
			:position(0, 0, 0)
			, target(0, 0, 0) {
			esLogMessage("[reander]Camera");
		}
		~Camera()
		{
			esLogMessage("[reander]~Camera");
		}
	};
}
#endif