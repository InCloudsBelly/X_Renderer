#ifndef X_GRAPHICSSURFACE_H
#define X_GRAPHICSSURFACE_H

#include <volk.h>
#include <SDL.h>

namespace X {

    namespace Graphics {

        class Instance;

        class Surface {
        public:
            Surface(Instance* instance, SDL_Window* window, bool& success);

            ~Surface();

            VkSurfaceKHR GetNativeSurface() const;

            SDL_Window* GetNativeWindow() const;

        private:
            Instance* instance = nullptr;

            VkSurfaceKHR surface;
            SDL_Window* window = nullptr;

        };

    }

}

#endif
