#ifndef X_AUDIODEVICEEVENT_H
#define X_AUDIODEVICEEVENT_H

#include "../System.h"
#include "EventDelegate.h"

#define X_AUDIODEVICE_ADDED SDL_AUDIODEVICEADDED
#define X_AUDIODEVICE_REMOVED SDL_AUDIODEVICEREMOVED

namespace X {

    namespace Events {

        /**
         * A class to distribute audio device events.
         */
        class AudioDeviceEvent {

        public:
            explicit AudioDeviceEvent(SDL_AudioDeviceEvent event) {

                type = event.type;
                device = event.which;

            }

            /**
             * The type of the event. See {@link AudioDeviceEvent.h} for more.
             */
            uint32_t type;

            /**
             * The device ID of the audio device.
             */
            uint32_t device;

        };

    }

}

#endif
