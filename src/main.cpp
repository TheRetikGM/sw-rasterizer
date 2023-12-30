#include <imguiwrapper.hpp> // Requires C++20
#include <iostream>
#include <swrast.h>

using namespace swrast;

struct MainProgram {
  ObjectHandle<VertexArray> vao;
  ObjectHandle<Framebuffer> fb;
  glm::uvec2 vp_size = { 400, 400 };

  void OnCreate() {
    State::Init(vp_size);
    VertexBuffer::Data vbo_data = {
        -0.5f, 0.0f, 0.0f,    1.0f, 0.0f, 0.0f,
         0.5f, 0.0f, 0.0f,    0.0f, 1.0f, 0.0f,
         0.0f, 0.7f, 0.0f,    0.0f, 0.0f, 1.0f,
    };
    auto vbo = State::CreateObject(VertexBuffer(std::move(vbo_data)));
    auto ibo = State::CreateObject(IndexBuffer({ 1, 2, 3 }));
    vao = State::CreateObject(VertexArray({
      { vbo, 3, AttributeType::Float32, 6 * sizeof(float), 0 },
      { vbo, 3, AttributeType::Float32, 6 * sizeof(float), 3 * sizeof(float) },
    }, ibo));
    fb = State::CreateObject(Framebuffer::CreateBasic(vp_size));
  }

  void OnDestroy() {
    State::Destroy();
  }

  void OnUpdate(float) {
    ImGui::DockSpaceOverViewport();
    ImGui::Begin("Rasterized image");
    // Draw into image.
    ImGui::End();
  }
};

int main() {
  ImWrap::ContextDefinition def;
  def.window_width = 800;
  def.window_height = 600;
  def.window_title = "PGR - SW rasterizer";
  def.window_hints[GLFW_RESIZABLE] = GLFW_FALSE;
  def.imgui_multiviewport = false; // Allow ImGui windows to be dragged out of the main window.
  def.imgui_theme = ImWrap::ImGuiTheme::dark;

  try {
    MainProgram prg;
    auto context = ImWrap::Context::Create(def);
    ImWrap::run(context, prg);
    ImWrap::Context::Destroy(context);
  } catch (const swrast::Exception& e) {
    std::cerr << "[\033[31m!! EXCEPTION !!\033[0m] " << e.what() << std::endl;
    return 1;
  } catch (const std::exception& e) {
    std::cerr << "[\033[31m!! EXCEPTION !!\033[0m] " << e.what() << std::endl;
    return 1;
  }

  return 0;
}
