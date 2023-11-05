#ifndef X_DROPEVENT_H
#define X_DROPEVENT_H

#include "../System.h"
#include "EventDelegate.h"

#include <string>

#define X_FILEDROP SDL_DROPFILE
#define X_TEXTDROP SDL_DROPTEXT
#define X_BEGINDROP SDL_DROPBEGIN
#define X_COMPLETEDROP SDL_DROPCOMPLETE

namespace X {

    namespace Events {

        /**
         * A class to distribute drop events.
         */
        class DropEvent {

        public:
            explicit DropEvent(SDL_DropEvent event) {

                windowID = event.windowID;
                if (event.file)
                    file = std::string(event.file);
                type = event.type;

            }

            /**
             * The ID of the window the event occurred in.
             */
            uint32_t windowID;

            /**
             * The path to the file. Is empty for X_BEGINDROP and X_COMPLETEDROP.
             */
            std::string file;

            /**
             * The type of the drop event. See {@link DropEvent.h}
             */
            uint32_t type;

        };

    }

}


#endif