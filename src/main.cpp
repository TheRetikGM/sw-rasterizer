#include "ren_utils/AvgSampler.hpp"
#include "ren_utils/RingBuffer.hpp"
#include "render/render.h"
#include <glm/ext/matrix_clip_space.hpp>
#include <imguiwrapper.hpp> // Requires C++20
#include <iostream>
#include <swrast.h>
#include <glm/gtc/matrix_transform.hpp>
#include <imwidgets/imwidgets.h>

using namespace swrast;

void print_vec(const char* name, const glm::vec3 vec) {
  printf("%s = [%f, %f, %f]\n", name, vec.x, vec.y, vec.z);
};
void print_vec(const char* name, const glm::vec2 vec) {
  printf("%s = [%f, %f]\n", name, vec.x, vec.y);
};

using namespace ImWidgets;


struct MainProgram {
  ObjectHandle<VertexArray> vao;
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

  MainProgram() {
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

    /// Vertex shader ///
    auto vs_func = [](VertexShader* vs) {
      // Vertex attributes
      auto aPos = vs->Attribute<glm::vec3>(0).value().get();
      auto aColor = vs->Attribute<glm::vec3>(1).value().get();
      // Uniforms
      auto mvp = vs->Uniform<glm::mat4>("mvp").value().get();
      // Computation
      vs->Out<glm::vec3>("color") = aColor;
      vs->m_Position = mvp * glm::vec4(aPos, 1.0f);
    };

    /// Fragment shader ///
    auto fs_func = [](FragmentShader* fs) {
      auto color = fs->In<glm::vec3>("color");
      fs->m_FragColor = glm::vec4(color, 1.0f);
    };

    prg = State::CreateObject(Program({
      .vertex_shader = State::CreateObject(VertexShader(vs_func)),
      .fragment_shader = State::CreateObject(FragmentShader(fs_func)),
    }));

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
    static bool depth_test = true;
    if (ImGui::Checkbox("Depth test", &depth_test))
      LOG_S(strfmt("Depth test: %s", depth_test ? "on" : "off"));
    State::SetDepthTest(depth_test);
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
    ImGui::End();
  }

  void OnUpdate(float dt) {
    this->dt = dt;
    ImGui::DockSpaceOverViewport();
    fps_sampler.Sample();

    // Transform
    glm::mat4 model(1.0f);
    model = glm::rotate(model, (float)ImGui::GetTime(), glm::vec3( 0.8f, 0.5f, 0.1f ));
    // model = glm::rotate(model, (float)glm::radians(45.0f), glm::vec3( 0.8f, 0.5f, 0.1f ));
    model = glm::scale(model, glm::vec3(1.0f));
    // Projection
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), 1.0f, 0.1f, 20.0f);
    // View
    glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 0.0f, 5.0f), { 0.0f, 0.0f, -1.0f }, { 0.0f, 1.0f, 0.0f });
    // MVP matrix
    glm::mat4 mvp = projection * view * model;

    fb->Use();
    State::Clear(Colors::Gray);
    prg->Use();
    prg->SetUniform("mvp", mvp);
    vao->Use();
    State::DrawIndexed(Primitive::Triangles, vao->GetIndexBuffer()->data.size());

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
