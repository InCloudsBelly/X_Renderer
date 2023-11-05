#ifndef X_CONTROLLERAXISEVENT_H
#define X_CONTROLLERAXISEVENT_H

#include "../System.h"
#include "EventDelegate.h"

#define X_CONTROLLERAXIS_LEFTX         SDL_CONTROLLER_AXIS_LEFTX
#define X_CONTROLLERAXIS_LEFTY         SDL_CONTROLLER_AXIS_LEFTY
#define X_CONTROLLERAXIS_RIGHTX         SDL_CONTROLLER_AXIS_RIGHTX
#define X_CONTROLLERAXIS_RIGHTY         SDL_CONTROLLER_AXIS_RIGHTY
#define X_CONTROLLERAXIS_LEFTTRIGGER     SDL_CONTROLLER_AXIS_TRIGGERLEFT
#define X_CONTROLLERAXIS_RIGHTTRIGGER SDL_CONTROLLER_AXIS_TRIGGERRIGHT

namespace X {

    namespace Events {

        /**
          * A class to distribute controller axis events.
         */
        class ControllerAxisEvent {

        public:
            explicit ControllerAxisEvent(SDL_ControllerAxisEvent event) {

                axis = event.axis;
                value = event.value;
                device = event.which;

            }

            /**
             * The axis which was moved on the controller. See {@link ControllerAxisEvent.h} for more.
             */
            uint8_t axis;

            /**
             * The value of the axis. Ranges from -32767-32767.
             */
            int16_t value;

            /**
             * The device ID of the game controller.
             */
            int32_t device;

        };

    }

}


#endif