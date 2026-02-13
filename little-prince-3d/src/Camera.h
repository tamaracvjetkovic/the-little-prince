#ifndef CAMERA_H
#define CAMERA_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace glm;

class Camera {
public:
    vec3 Position;
    vec3 Front;
    vec3 Up;
    vec3 Right;
    vec3 WorldUp;

    float Yaw;
    float Pitch;
    float Distance; // camera distance from target

    Camera(
        vec3 position = vec3(0.0f, 0.0f, 0.0f),
        vec3 up = vec3(0.0f, 1.0f, 0.0f),
        float yaw = -90.0f,
        float pitch = 0.0f
    ) : Front(vec3(0.0f, 0.0f, -1.0f)), Distance(5.0f) {
        Position = position;
        WorldUp = up;
        Yaw = yaw;
        Pitch = pitch;
        updateCameraVectors();
    }

    mat4 GetViewMatrix() {
        return lookAt(Position, Position + Front, Up);
    }

    // orbit around target
    void Follow(vec3 target, float orbitAngle, float height = 2.0f) {
        // Calculate position based on target, distance, and orientation
        // orbit angle is around vertical axis (planet normal)
        
        vec3 up = normalize(target);
        
        // find ref direction, same as in movement so it doesnt glitch
        vec3 ref = (abs(up.y) < 0.999f) ? vec3(0, 1, 0) : vec3(0, 0, 1);
        vec3 right = normalize(cross(up, ref));
        vec3 forward = cross(up, right);
        
        // character can rotate independently from camera
        float totalAngle = orbitAngle;
        
        // Rotate the base forward vector by totalAngle around the normal (up)
        vec3 dir = normalize(forward * cos(totalAngle) + right * sin(totalAngle));
        
        Position = target + up * height - dir * Distance;
        Front = normalize(target + up * (height * 0.5f) - Position);
        // look slightly above target
        Right = normalize(cross(Front, up));
        Up = normalize(cross(Right, Front));
    }

private:
    void updateCameraVectors() {
        vec3 front;
        front.x = cos(radians(Yaw)) * cos(radians(Pitch));
        front.y = sin(radians(Pitch));
        front.z = sin(radians(Yaw)) * cos(radians(Pitch));
        Front = normalize(front);
        Right = normalize(cross(Front, WorldUp));
        Up = normalize(cross(Right, Front));
    }
};
#endif
