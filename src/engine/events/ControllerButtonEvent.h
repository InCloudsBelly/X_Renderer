#ifndef X_CONTROLLERBUTTONEVENT_H
#define X_CONTROLLERBUTTONEVENT_H

#include "../System.h"
#include "EventDelegate.h"

#define X_CONTROLLERBUTTON_A                 SDL_CONTROLLER_BUTTON_A
#define X_CONTROLLERBUTTON_B                 SDL_CONTROLLER_BUTTON_B
#define X_CONTROLLERBUTTON_X                 SDL_CONTROLLER_BUTTON_X
#define X_CONTROLLERBUTTON_Y                 SDL_CONTROLLER_BUTTON_Y
#define X_CONTROLLERBUTTON_UP                 SDL_CONTROLLER_BUTTON_DPAD_UP
#define X_CONTROLLERBUTTON_DOWN             SDL_CONTROLLER_BUTTON_DPAD_DOWN
#define X_CONTROLLERBUTTON_LEFT             SDL_CONTROLLER_BUTTON_DPAD_LEFT
#define X_CONTROLLERBUTTON_RIGHT             SDL_CONTROLLER_BUTTON_DPAD_RIGHT
#define X_CONTROLLERBUTTON_LEFTSTICK         SDL_CONTROLLER_BUTTON_LEFTSTICK
#define X_CONTROLLERBUTTON_RIGHTSTICK         SDL_CONTROLLER_BUTTON_RIGHTSTICK
#define X_CONTROLLERBUTTON_LEFTSHOULDER     SDL_CONTROLLER_BUTTON_LEFTSHOULDER
#define X_CONTROLLERBUTTON_RIGHTSHOULDER     SDL_CONTROLLER_BUTTON_RIGHTSHOULDER
#define X_CONTROLLERBUTTON_BACK             SDL_CONTROLLER_BUTTON_BACK
#define X_CONTROLLERBUTTON_GUIDE             SDL_CONTROLLER_BUTTON_GUIDE
#define X_CONTROLLERBUTTON_START             SDL_CONTROLLER_BUTTON_START

namespace X {

    namespace Events {

        /**
         * A class to distribute controller button events.
         */
        class ControllerButtonEvent {

        public:
            explicit ControllerButtonEvent(SDL_ControllerButtonEvent event) {

                button = event.button;
                state = event.state;
                device = event.which;

            }

            /**
             * The button which was pressed on the controller. See {@link ControllerButtonEvent.h} for more.
             */
            uint8_t button;

            /**
             * The state of the button. Might be BUTTON_PRESSED or BUTTON_RELEASED.
             */
            uint8_t state;

            /**
             * The device ID of the game controller.
             */
            int32_t device;

        };

    }

}

#endif