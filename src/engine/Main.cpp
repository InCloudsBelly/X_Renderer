#include "Engine.h"
#include "EngineInstance.h"
#include "graphics/Instance.h"
#include "common/Path.h"

#if defined(X_OS_ANDROID) || defined(X_OS_MACOS) || defined(X_OS_LINUX)
#include <zconf.h>
#endif

#ifdef X_OS_WINDOWS
#include <direct.h>
#endif

#ifdef main
#undef main
#endif

int main(int argc, char* argv[]) {

    // Automatically change working directory to load
    // shaders properly.
    if (argc > 0) {
        auto workingDir = X::Common::Path::GetDirectory(argv[0]);
#ifdef X_OS_WINDOWS
        _chdir(workingDir.c_str());
#else
        chdir(workingDir.c_str());
#endif
    }

    X::Engine::Init(X::EngineInstance::engineConfig);

    auto graphicsInstance = X::Graphics::Instance::DefaultInstance;

    if (!graphicsInstance->isComplete) {
        X::Log::Error("Couldn't initialize graphics instance");
        X::Engine::Shutdown();
        return 0;
    }

    auto engineInstance = X::EngineInstance::GetInstance();
    if (!engineInstance) {
        X::Log::Warning("Shutdown of application");
        X::Engine::Shutdown();
        return 0;
    }

    auto graphicsDevice = graphicsInstance->GetGraphicsDevice();

    // We need to pass the command line arguments
    for (int32_t i = 0; i < argc; i++)
        engineInstance->args.push_back(std::string(argv[i]));

    bool quit = false;

    X::Events::EventManager::QuitEventDelegate.Subscribe(
        [&quit]() {
            quit = true;
    });

    engineInstance->LoadContent();

    while (!quit) {

        X::Engine::Update();
        
        auto deltaTime = X::Clock::GetDelta();

        engineInstance->Update();

        engineInstance->Update(deltaTime);
        engineInstance->Render(deltaTime);

        graphicsDevice->CompleteFrame();
        
    }

    engineInstance->UnloadContent();
    delete engineInstance;

    X::Engine::Shutdown();
    delete graphicsInstance;

    return 0;

}
