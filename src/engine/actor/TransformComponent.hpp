#ifndef X_TRANSFOMCOMPONENT_H
#define X_TRANSFOMCOMPONENT_H

#include "common/MathHelper.h"

namespace X {

	namespace Actor {


		struct TransformComponent
		{
			glm::vec3 Translation = { 0.0f, 0.0f, 0.0f };
			glm::vec3 Scale = { 1.0f, 1.0f, 1.0f };
		private:
			// These are private so that you are forced to set them via
			// SetRotation() or SetRotationEuler()
			// This avoids situation where one of them gets set and the other is forgotten.
			//
			// Why do we need both a quat and Euler angle representation for rotation?
			// Because Euler suffers from gimbal lock -> rotations should be stored as quaternions.
			//
			// BUT: quaternions are confusing, and humans like to work with Euler angles.
			// We cannot store just the quaternions and translate to/from Euler because the conversion
			// Euler -> quat -> Euler is not invariant.
			//
			// It's also sometimes useful to be able to store rotations > 360 degrees which
			// quats do not support.
			//
			// Accordingly, we store Euler for "editor" stuff that humans work with, 
			// and quats for everything else.  The two are maintained in-sync via the SetRotation()
			// methods.
			glm::vec3 RotationEuler = { 0.0f, 0.0f, 0.0f };
			glm::quat Rotation = { 1.0f, 0.0f, 0.0f, 0.0f };

		public:
			TransformComponent() = default;
			TransformComponent(const TransformComponent& other) = default;
			TransformComponent(const glm::vec3& translation)
				: Translation(translation) {}

			glm::mat4 GetTransform() const
			{
				return glm::translate(glm::mat4(1.0f), Translation)
					* glm::toMat4(Rotation)
					* glm::scale(glm::mat4(1.0f), Scale);
			}

			void SetTransform(const glm::mat4& transform)
			{
				Helper::Math::DecomposeTransform(transform, Translation, Rotation, Scale);
				RotationEuler = glm::eulerAngles(Rotation);
			}

			glm::vec3 GetRotationEuler() const
			{
				return RotationEuler;
			}

			void SetRotationEuler(const glm::vec3& euler)
			{
				RotationEuler = euler;
				Rotation = glm::quat(RotationEuler);
			}

			glm::quat GetRotation() const
			{
				return Rotation;
			}

			void SetRotation(const glm::quat& quat)
			{
				Rotation = quat;
				RotationEuler = glm::eulerAngles(Rotation);
			}

		};
	}
}

#endif