#ifndef X_CONTROLLERDEVICEEVENT_H
#define X_CONTROLLERDEVICEEVENT_H

#include "../System.h"
#include "EventDelegate.h"

#define X_CONTROLLER_ADDED SDL_CONTROLLERDEVICEADDED
#define X_CONTROLLER_REMOVED SDL_CONTROLLERDEVICEREMOVED
#define X_CONTROLLER_MAPPED SDL_CONTROLLERDEVICEREMAPPED

namespace X {

    namespace Events {

        /**
         * A class to distribute controller device events.
         */
        class ControllerDeviceEvent {

        public:
            explicit ControllerDeviceEvent(SDL_ControllerDeviceEvent event) {

                type = event.type;
                device = event.which;

            }

            /**
             * The type of the event. See {@link ControllerDeviceEvent.h} for more.
             */
            uint32_t type;

            /**
             * The device ID of the game controller.
             */
            int32_t device;

        };

    }

}


#endif