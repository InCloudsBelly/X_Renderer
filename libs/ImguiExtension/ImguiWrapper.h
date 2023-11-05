#ifndef X_IMGUIWRAPPER_H
#define X_IMGUIWRAPPER_H

// ImguiExtension includes
#include <imgui.h>
#include <imgui_internal.h>

// X3 engine includes
#include <Window.h>
#include <graphics/Descriptor.h>

class ImguiWrapper {

public:
    ImguiWrapper() = default;

    void Load(X::Window* window);

    void Unload();

    void Update(X::Window* window, float deltaTime);

    void Render(bool clearSwapChain = false);

    void RecreateImGuiResources();

    VkDescriptorSet GetTextureDescriptorSet(const X::Texture::Texture2D& texture,
        VkImageLayout layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

private:
    void MouseMotionHandler(X::Events::MouseMotionEvent event);
    void MouseButtonHandler(X::Events::MouseButtonEvent event);
    void MouseWheelHandler(X::Events::MouseWheelEvent event);
    void KeyboardHandler(X::Events::KeyboardEvent event);
    void TextInputHandler(X::Events::TextInputEvent event);
    void WindowHandler(X::Events::WindowEvent event);

    ImGuiKey KeycodeToImGuiKey(int keyCode);
    void UpdateKeyModifiers(uint16_t keyModifier);

    void UpdateMouseCursor();

    SDL_Cursor* mouseCursors[ImGuiMouseCursor_COUNT];

    int32_t windowID = 0;
    int32_t mouseMotionID = 0;
    int32_t mouseButtonID = 0;
    int32_t mouseWheelID = 0;
    int32_t keyboardID = 0;
    int32_t textInputID = 0;

    X::Window* window = nullptr;
    X::Ref<X::Graphics::DescriptorPool> pool = nullptr;

    std::unordered_map<VkImageView, VkDescriptorSet> imageViewToDescriptorSetMap;

};

#endif