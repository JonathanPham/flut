#include "Renderer.hpp"

#include <Eigen/Core>
#include <Eigen/Geometry>
#include <iostream>
#define _USE_MATH_DEFINES
#include <math.h>

using namespace gl;

ansimproj::Renderer::Renderer()
  : BaseRenderer() {

  const auto &vert = core::Utils::loadFileText(RESOURCES_PATH "/simplePoint.vert");
  const auto &frag = core::Utils::loadFileText(RESOURCES_PATH "/simplePoint.frag");
  renderProgram_ = createVertFragShader(vert, frag);
  std::cout << "Vert/Frag Shader compiled: " << renderProgram_ << std::endl;

  // clang-format off
  const std::vector<float> data = {
    //   position    |      color
     0.5f,  0.5f,  0.5f, 1.0f, 1.0f, 1.0f,
     0.5f,  0.5f, -0.5f, 1.0f, 1.0f, 0.0f,
     0.5f, -0.5f,  0.5f, 1.0f, 0.0f, 1.0f,
     0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f,
    -0.5f,  0.5f,  0.5f, 0.0f, 1.0f, 1.0f,
    -0.5f,  0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
    -0.5f, -0.5f,  0.5f, 0.0f, 0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 0.0f
  };
  // clang-format on
  exampleBuffer_ = createBuffer(data);
  std::cout << "Uploaded buffer: " << exampleBuffer_ << std::endl;

  const auto &comp = core::Utils::loadFileText(RESOURCES_PATH "/simpleMod.comp");
  computeProgram_ = createComputeShader(comp);
  std::cout << "Compute Shader compiled: " << computeProgram_ << std::endl;

  vao_ = createVAO(exampleBuffer_);
  std::cout << "VAO created: " << vao_ << std::endl;

  glPointSize(10.0f);
}

ansimproj::Renderer::~Renderer() {
  deleteShader(renderProgram_);
  deleteShader(computeProgram_);
  deleteBuffer(exampleBuffer_);
  deleteVAO(vao_);
}

GLuint ansimproj::Renderer::createVAO(const GLuint &vbo) const {
  GLuint handle;
  glCreateVertexArrays(1, &handle);
  if (!handle) {
    throw std::runtime_error("Unable to create VAO.");
  }
  constexpr GLuint binding = 0;
  glVertexArrayVertexBuffer(handle, binding, vbo, 0, 6 * sizeof(float));

  constexpr GLuint posAttrIndex = 0;
  constexpr GLuint posAttrOffet = 0;
  glEnableVertexArrayAttrib(handle, posAttrIndex);
  glVertexArrayAttribFormat(handle, posAttrIndex, 3, GL_FLOAT, GL_FALSE, posAttrOffet);
  glVertexArrayAttribBinding(handle, posAttrIndex, binding);
  constexpr GLuint colAttrIndex = 1;
  constexpr GLuint colAttrOffet = 3 * sizeof(float);
  glEnableVertexArrayAttrib(handle, colAttrIndex);
  glVertexArrayAttribFormat(handle, colAttrIndex, 3, GL_FLOAT, GL_FALSE, colAttrOffet);
  glVertexArrayAttribBinding(handle, colAttrIndex, binding);
  return handle;
}

void ansimproj::Renderer::deleteVAO(GLuint handle) {
  glDeleteVertexArrays(1, &handle);
}

void ansimproj::Renderer::render(const ansimproj::core::Camera &camera) const {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glBindVertexArray(vao_);
  glUseProgram(renderProgram_);

  const auto &view = camera.view();
  const auto &projection = camera.projection();
  constexpr GLuint mvpLoc = 0;
  constexpr GLuint mvLoc = 1;
  const Eigen::Matrix4f mvp = projection * view;
  glProgramUniformMatrix4fv(renderProgram_, mvpLoc, 1, GL_FALSE, mvp.data());
  glProgramUniformMatrix4fv(renderProgram_, mvLoc, 1, GL_FALSE, view.data());
  glDrawArrays(GL_POINTS, 0, 8);
}

void ansimproj::Renderer::resize(std::uint32_t width, std::uint32_t height) {
  glViewport(0, 0, width, height);
}
