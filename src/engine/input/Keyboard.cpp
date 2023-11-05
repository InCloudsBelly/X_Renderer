#include "Keyboard.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

namespace X {

    namespace Input {

        KeyboardHandler::KeyboardHandler() {

            RegisterEvent();

        }

        KeyboardHandler::KeyboardHandler(const KeyboardHandler& that) {

            RegisterEvent();

            DeepCopy(that);

        }

        KeyboardHandler::KeyboardHandler(Camera* camera, float speed, float reactivity) :
                speed(speed), reactivity(reactivity) {

            RegisterEvent();

            location = camera->location;
            
        }

        KeyboardHandler::~KeyboardHandler() {

            Events::EventManager::KeyboardEventDelegate.Unsubscribe(eventHandle);

        }

        KeyboardHandler& KeyboardHandler::operator=(const KeyboardHandler& that) {

            if (this != &that) {

                DeepCopy(that);

            }

            return *this;

        }

        void KeyboardHandler::Update(Camera* camera, float deltaTime) {

            location += camera->direction * movement.x * deltaTime * speed;
            location += camera->right * movement.y * deltaTime * speed;
            location += glm::cross(camera->right, camera->direction) * movement.z * deltaTime * speed;
            float progress = glm::clamp(reactivity * deltaTime, 0.0f, 1.0f);

            camera->location = glm::mix(camera->location, location, progress);

        }

        void KeyboardHandler::Reset(Camera* camera) {

            location = camera->location;

        }

        void KeyboardHandler::RegisterEvent() {

            auto keyboardEventHandler = std::bind(&KeyboardHandler::KeyboardEventHandler, this, std::placeholders::_1);
            eventHandle = Events::EventManager::KeyboardEventDelegate.Subscribe(keyboardEventHandler);

        }

        void KeyboardHandler::KeyboardEventHandler(Events::KeyboardEvent event) {

            if (event.keyCode == X_KEY_W && event.state == X_BUTTON_PRESSED && !event.repeat) {
                movement.x += 1.0f;
            }

            if (event.keyCode == X_KEY_W && event.state == X_BUTTON_RELEASED) {
                movement.x -= 1.0f;
            }

            if (event.keyCode == X_KEY_S && event.state == X_BUTTON_PRESSED && !event.repeat) {
                movement.x -= 1.0f;
            }

            if (event.keyCode == X_KEY_S && event.state == X_BUTTON_RELEASED) {
                movement.x += 1.0f;
            }

            if (event.keyCode == X_KEY_D && event.state == X_BUTTON_PRESSED && !event.repeat) {
                movement.y += 1.0f;
            }

            if (event.keyCode == X_KEY_D && event.state == X_BUTTON_RELEASED) {
                movement.y -= 1.0f;
            }

            if (event.keyCode == X_KEY_A && event.state == X_BUTTON_PRESSED && !event.repeat) {
                movement.y -= 1.0f;
            }

            if (event.keyCode == X_KEY_A && event.state == X_BUTTON_RELEASED) {
                movement.y += 1.0f;
            }

            if (event.keyCode == X_KEY_Q && event.state == X_BUTTON_PRESSED && !event.repeat) {
                movement.z -= 1.0f;
            }

            if (event.keyCode == X_KEY_Q && event.state == X_BUTTON_RELEASED) {
                movement.z += 1.0f;
            }

            if (event.keyCode == X_KEY_E && event.state == X_BUTTON_PRESSED && !event.repeat) {
                movement.z += 1.0f;
            }

            if (event.keyCode == X_KEY_E && event.state == X_BUTTON_RELEASED) {
                movement.z -= 1.0f;
            }

        }

        void KeyboardHandler::DeepCopy(const KeyboardHandler& that) {

            speed = that.speed;
            reactivity = that.reactivity;

            location = that.location;
            movement = that.movement;

        }

    }

}