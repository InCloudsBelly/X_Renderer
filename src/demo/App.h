#ifndef X_APP_H
#define X_APP_H

#include <EngineInstance.h>
#include <input/Mouse.h>
#include <input/Keyboard.h>
#include <input/Controller.h>
#include <input/Touch.h>
#include <loader/ModelLoader.h>
#include <ImguiExtension/ImguiWrapper.h>

#include <renderer/PathTracingRenderer.h>

#define WINDOW_FLAGS X_WINDOW_RESIZABLE | X_WINDOW_HIGH_DPI

class App : public X::EngineInstance {

    template<class T>
    using Ref = X::Ref<T>;

public:
    App() : EngineInstance("X3 Engine Demo", 1920, 1080, WINDOW_FLAGS) {}

    virtual void LoadContent() final;

    virtual void UnloadContent() final;

    virtual void Update(float deltaTime) final;

    virtual void Render(float deltaTime) final;

private:
    enum SceneSelection {
        CORNELL = 0,
        SPONZA,
        SANMIGUEL,
        NEWSPONZA,
        BISTRO,
        MEDIEVAL,
        PICAPICA,
        SUBWAY,
        MATERIALS,
        FOREST
    };

    void DisplayLoadingScreen(float deltaTime);

    bool IsSceneAvailable(SceneSelection selection);
    bool LoadScene();
    void UnloadScene();
    void CheckLoadScene();

    void SetResolution(int32_t width, int32_t height);

    void CopyActors(Ref<X::Scene::Scene> otherScene);

    SceneSelection sceneSelection = SPONZA;

    X::Renderer::PathTracerRenderTarget pathTraceTarget;
    X::RenderTarget renderTarget;
    X::Viewport viewport;

    X::Font font;

    X::Camera camera;

    Ref<X::Scene::Scene> scene;
    Ref<X::Lighting::DirectionalLight> directionalLight;
    std::vector<Ref<X::Lighting::PointLight>> pointLights;

    std::vector<X::ResourceHandle<X::Mesh::Mesh>> meshes;
    std::vector<X::Actor::MovableMeshActor> actors;

    X::Lighting::EnvironmentProbe probe;

    X::Input::MouseHandler mouseHandler;
    X::Input::KeyboardHandler keyboardHandler;

    Ref<X::Texture::Texture2D> loadingTexture;

    bool renderUI = true;
    bool renderEnvProbe = true;
    bool spheresVisible = false;

    bool rotateCamera = false;
    bool moveCamera = false;
    float rotateCameraSpeed = 0.01f;
    float moveCameraSpeed = 0.1f;

    int32_t windowWidth = 1920;
    int32_t windowHeight = 1080;

    float cameraSpeed = 7.0f;

    bool loadingComplete = false;
    bool sceneReload = false;

    ImguiWrapper imguiWrapper;

};

#endif