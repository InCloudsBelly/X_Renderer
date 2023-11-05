#ifndef X_WINDOWEVENT_H
#define X_WINDOWEVENT_H

#include "../System.h"
#include "EventDelegate.h"

#define X_WINDOWEVENT_SHOWN             SDL_WINDOWEVENT_SHOWN
#define X_WINDOWEVENT_HIDDEN             SDL_WINDOWEVENT_HIDDEN
#define X_WINDOWEVENT_EXPOSED             SDL_WINDOWEVENT_EXPOSED
#define X_WINDOWEVENT_MOVED             SDL_WINDOWEVENT_MOVED
#define X_WINDOWEVENT_RESIZED             SDL_WINDOWEVENT_RESIZED
#define X_WINDOWEVENT_MINIMIZED         SDL_WINDOWEVENT_MINIMIZED
#define X_WINDOWEVENT_MAXIMIZED         SDL_WINDOWEVENT_MAXIMIZED
#define X_WINDOWEVENT_RESTORED         SDL_WINDOWEVENT_RESTORED
#define X_WINDOWEVENT_MOUSE_ENTERED     SDL_WINDOWEVENT_ENTER
#define X_WINDOWEVENT_MOUSE_LEAVED     SDL_WINDOWEVENT_LEAVE
#define X_WINDOWEVENT_FOCUS_GAINED     SDL_WINDOWEVENT_FOCUS_GAINED
#define X_WINDOWEVENT_FOCUS_LOST         SDL_WINDOWEVENT_FOCUS_LOST
#define X_WINDOWEVENT_CLOSE             SDL_WINDOWEVENT_CLOSE

namespace X {

    namespace Events {

        /**
         * A class to distribute window events.
         */
        class WindowEvent {

        public:
            explicit WindowEvent(SDL_WindowEvent event) :
                data(event.data1, event.data2) {
                windowID = event.windowID;
                type = event.event;
            }

            /**
             * The ID of the window the event occurred in
             */
            uint32_t windowID;

            /**
             * The type of the window event. See {@link WindowEvent.h} for more
             */
            uint8_t type;

            /**
             * The data specific to the event. Represents screen space coordinates
             */
            ivec2 data;

        };

    }

}

#endif