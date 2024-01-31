#include "ren_utils/AvgSampler.hpp"
#include "render/render.h"
#include "camera.h"
#include <glm/ext/matrix_clip_space.hpp>
#include <imguiwrapper.hpp> // Requires C++20
#include <iostream>
#include <swrast.h>
#include <glm/gtc/matrix_transform.hpp>
#include <imwidgets/imwidgets.h>

using namespace swrast;
using namespace ImWidgets;

void vertex_shader(VertexShader* vs) {
  auto aPos = vs->Attribute<glm::vec3>(0).value().get();
  auto aColor = vs->Attribute<glm::vec3>(1).value().get();
  auto& color = vs->Out<glm::vec3>("color"_sid);
  auto mvp = vs->Uniform<glm::mat4>("mvp"_sid).value().get();

  color = aColor;
  vs->m_Position = mvp * glm::vec4(aPos, 1.0f);
}

void fragment_shader(FragmentShader* fs) {
  auto& color = fs->In<glm::vec3>("color"_sid);

  fs->m_FragColor = glm::vec4(color, 1.0f);
}

struct MainProgram {
  ObjectHandle<VertexArray> vao;
  ObjectHandle<VertexArray> axis_vao;
  ObjectHandle<Framebuffer> fb;
  ObjectHandle<Program> prg;
  glm::uvec2 vp_size = { 400, 400 };

  // Gui utilitities
  float dt = 0.0f;    // Last delta time
  GuiLogger* logger;
  ren_utils::AvgSampler<float> fps_sampler = ren_utils::AvgSampler<float>(144, [this]{ return 1.0f / dt; }, ren_utils::SampleMode::CONTINUOUS);
  FpsPlot<float> fps_plot = FpsPlot<float>(fps_sampler);

  // GPU texture used for drawing the resulting image in ImGui window.
  GLuint fb_texture;

  Camera camera{};
  GLFWwindow* window;

  // UI variables
  bool rotate_cube = false;

  MainProgram(GLFWwindow* glfw_window) : window(glfw_window) {
    logger = ren_utils::LogEmitter::AddListener<GuiLogger>(100, true);
  }

  void OnCreate() {
    State::Init(vp_size);
    VertexBuffer::Data vbo_data = {
      -0.5f,  0.5f,  0.5f,    0.0f, 0.0f, 0.0f,
       0.5f,  0.5f,  0.5f,    1.0f, 0.0f, 0.0f,
      -0.5f, -0.5f,  0.5f,    0.0f, 1.0f, 0.0f,
       0.5f, -0.5f,  0.5f,    0.0f, 0.0f, 1.0f,

      -0.5f,  0.5f, -0.5f,    1.0f, 1.0f, 0.0f,
       0.5f,  0.5f, -0.5f,    1.0f, 0.0f, 1.0f,
      -0.5f, -0.5f, -0.5f,    0.0f, 1.0f, 1.0f,
       0.5f, -0.5f, -0.5f,    1.0f, 1.0f, 1.0f,
    };
    auto vbo = State::CreateObject(VertexBuffer(std::move(vbo_data)));
    auto ibo = State::CreateObject(IndexBuffer({    // Cube
      0, 1, 2, 2, 1, 3,     // Near
      1, 5, 3, 3, 5, 7,     // Right
      5, 4, 7, 7, 4, 6,     // Far
      4, 0, 6, 6, 0, 2,     // Left
      4, 1, 0, 4, 5, 1,     // Top
      6, 2, 3, 6, 3, 7,     // Bottom
    }));
    vao = State::CreateObject(VertexArray({
      { vbo, AttributeType::Vec3, 6 * sizeof(float), 0 },
      { vbo, AttributeType::Vec3, 6 * sizeof(float), 3 * sizeof(float) },
    }, ibo));
    fb = State::CreateObject(Framebuffer::CreateBasic(vp_size));

    prg = State::CreateObject(Program({
      .vertex_shader = State::CreateObject(VertexShader(vertex_shader)),
      .fragment_shader = State::CreateObject(FragmentShader(fragment_shader)),
    }));

    // Setup axis lines
    auto axis_lines_vbo = State::CreateObject(VertexBuffer({
      0.0f, 0.0f, 0.0f,   1.0f, 0.0f, 0.0f,
      10.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,

      0.0f, 0.0f, 0.0f,   0.0f, 1.0f, 0.0f,
      0.0f, 10.0f, 0.0f, 0.0f, 1.0f, 0.0f,

      0.0f, 0.0f, 0.0f,   0.0f, 0.0f, 1.0f,
      0.0f, 0.0f, 10.0f, 0.0f, 0.0f, 1.0f,
    }));

    axis_vao = State::CreateObject(VertexArray({
      { axis_lines_vbo, AttributeType::Vec3, 6 * sizeof(float), 0 },
      { axis_lines_vbo, AttributeType::Vec3, 6 * sizeof(float), 3 * sizeof(float) },
    }));

    State::m_DepthTest = true;

    // Init opengl gpu texture used for imgui image drawing.
    glGenTextures(1, &fb_texture);
    glBindTexture(GL_TEXTURE_2D, fb_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, vp_size.x, vp_size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  }

  void OnDestroy() {
    State::Destroy();
  }

  void DrawGui() {
    logger->Draw();
    ImGui::Begin("Control panel");
    ImGui::SeparatorText("Info");
    fps_plot.DrawPlot();
    ImGui::SeparatorText("Controls");
    if (ImGui::Checkbox("Depth test", &State::m_DepthTest))
      LOG_S(strfmt("Depth test: %s", State::m_DepthTest ? "on" : "off"));
    static bool culling = false;
    static int cull_face_current = 2;
    if (ImGui::Checkbox("Culling", &culling))
        LOG_S(strfmt("Cull face: %s", culling ? (cull_face_current == 1 ? "CW" : "CCW") : "None"));
    if (!culling)
      State::SetCullFace(CullFace::None);
    else {
      ImGui::Indent();
      if (ImGui::SliderInt("Method", &cull_face_current, 1, 2, cull_face_current == 1 ? "Clockwise" : "Counter clockwise"))
        LOG_S(strfmt("Cull face: %s", cull_face_current == 1 ? "CW" : "CCW"));
      State::SetCullFace(CullFace(cull_face_current));
      ImGui::Unindent();
    }
    ImGui::Checkbox("Wireframe", &State::m_WriteFrame);
    ImGui::Separator();
    ImGui::Checkbox("Rotate cube", &rotate_cube);
    if (ImGui::Button("Reset camera"))
      camera.Reset();
    ImGui::End();
  }

  void OnUpdate(float dt) {
    static bool capture_input = false;
    if (ImGui::IsKeyPressed(ImGuiKey_F2, false)) {
      capture_input = !capture_input;
      glfwSetInputMode(window, GLFW_CURSOR, capture_input ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
      LOG_S(strfmt("Capture input: %s", capture_input ? "yes" : "no"));
    }
    if (capture_input)
      camera.Update(dt);

    this->dt = dt;
    ImGui::DockSpaceOverViewport();
    fps_sampler.Sample();
    DrawGui();

    // Transform
    glm::mat4 model(1.0f);
    if (rotate_cube)
      model = glm::rotate(model, (float)ImGui::GetTime(), glm::vec3( 0.8f, 0.5f, 0.1f ));
    else {
      model = glm::rotate(model, glm::radians(50.0f), glm::vec3( 0.8f, 0.5f, 0.1f ));
    }
    model = glm::scale(model, glm::vec3(1.0f));
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), 1.0f, 0.1f, 20.0f);
    // glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 0.0f, 5.0f), { 0.0f, 0.0f, -1.0f }, { 0.0f, 1.0f, 0.0f });
    glm::mat4 mvp = projection * camera.m_ViewMatrix * model;

    fb->Use();
    State::Clear(Colors::Gray);
    prg->Use();

    prg->SetUniform("mvp"_sid, mvp);
    vao->Use();
    State::DrawIndexed(Primitive::Triangles, vao->GetIndexBuffer()->data.size());

    // prg->SetUniform("mvp"_sid, projection * camera.m_ViewMatrix);
    // axis_vao->Use();
    // State::DrawArrays(Primitive::Lines, 0, 6);

    // Copy data from rendered framebuffer and display it in ImGui window.
    ImGui::Begin("Rasterized image");
    {
      glBindTexture(GL_TEXTURE_2D, fb_texture);
      uint8_t* p_data = State::GetActiveFramebuffer()->GetColorAttach(0)->Get().GetPixel({0, 0});
      assert(p_data != nullptr);
      glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, vp_size.x, vp_size.y, GL_RGBA, GL_UNSIGNED_BYTE, p_data);
      ImGui::Image((void*)(intptr_t)fb_texture, ImVec2(vp_size.x, vp_size.y), ImVec2(0, 1), ImVec2(1, 0));
    }
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
    auto context = ImWrap::Context::Create(def);
    MainProgram prg(context->m_Window);
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
