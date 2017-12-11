#include "Simulation.hpp"

#include <Eigen/Core>
#include <Eigen/Geometry>
#include <iostream>

using namespace gl;

ansimproj::Simulation::Simulation()
  : BaseRenderer() {

  // Buffer with <uint particleId, uint voxelId>
  // representing the Uniform Grid mappings.
  std::vector<GLuint> gridPairsData;
  gridPairsData.resize(PARTICLE_COUNT * 2);
  gridPairs_ = createBuffer(gridPairsData, true);

  // Position 1 & 2 SSBO (pos+col)
  std::vector<float> positionData;
  const std::uint64_t dataSize = PARTICLE_COUNT * 6;
  positionData.reserve(dataSize);
  const std::uint64_t AXIS_COUNT = static_cast<std::uint64_t>(std::cbrt(PARTICLE_COUNT));
  assert((std::cbrt(PARTICLE_COUNT) - AXIS_COUNT) < 0.00001);
  for (std::uint64_t x = 0; x < AXIS_COUNT; ++x) {
    for (std::uint64_t y = 0; y < AXIS_COUNT; ++y) {
      for (std::uint64_t z = 0; z < AXIS_COUNT; ++z) {
        const float valX = -0.5f + (static_cast<float>(x) / AXIS_COUNT);
        const float valY = -0.5f + (static_cast<float>(y) / AXIS_COUNT);
        const float valZ = -0.5f + (static_cast<float>(z) / AXIS_COUNT);
        const float colX = static_cast<float>(x) / AXIS_COUNT;
        const float colY = static_cast<float>(y) / AXIS_COUNT;
        const float colZ = static_cast<float>(z) / AXIS_COUNT;
        positionData.push_back(valX);
        positionData.push_back(valY);
        positionData.push_back(valZ);
        positionData.push_back(colX);
        positionData.push_back(colY);
        positionData.push_back(colZ);
      }
    }
  }
  position1_ = createBuffer(positionData, true);
  position2_ = createBuffer(positionData, true);

  // Velocity SSBO
  std::vector<float> velocityData;
  velocityData.reserve(PARTICLE_COUNT * 3);
  for (std::uint64_t x = 0; x < AXIS_COUNT; ++x) {
    for (std::uint64_t y = 0; y < AXIS_COUNT; ++y) {
      for (std::uint64_t z = 0; z < AXIS_COUNT; ++z) {
        const float valX = -0.5f + (static_cast<float>(x) / AXIS_COUNT);
        const float valY = -0.5f + (static_cast<float>(y) / AXIS_COUNT);
        const float valZ = -0.5f + (static_cast<float>(z) / AXIS_COUNT);
        velocityData.push_back(0.1f * valX);
        velocityData.push_back(0.1f * valY);
        velocityData.push_back(0.1f * valZ);
      }
    }
  }
  velocity2_ = createBuffer(velocityData, true);
  std::cout << "Uploaded buffers: " << position1_ << ", " << position2_ << ", " << velocity2_
            << std::endl;

  // Uniform Grid Insert Shader
  const auto &comp1 = core::Utils::loadFileText(RESOURCES_PATH "/gridInsert.comp");
  gridInsertProgram_ = createComputeShader(comp1);
  std::cout << "Grid Insert Shader compiled: " << gridInsertProgram_ << std::endl;


  // Position Update Compute Shader
  const auto &comp2 = core::Utils::loadFileText(RESOURCES_PATH "/positionUpdate.comp");
  positionUpdateProgram_ = createComputeShader(comp2);
  std::cout << "Position Update Shader compiled: " << positionUpdateProgram_ << std::endl;

  // Render shader & VAO
  const auto &vert = core::Utils::loadFileText(RESOURCES_PATH "/simplePoint.vert");
  const auto &frag = core::Utils::loadFileText(RESOURCES_PATH "/simplePoint.frag");
  renderProgram_ = createVertFragShader(vert, frag);
  std::cout << "Render Shader compiled: " << renderProgram_ << std::endl;
  vao_ = createVAO(position1_);
  std::cout << "VAO created: " << vao_ << std::endl;

  glPointSize(5.0f);
}

ansimproj::Simulation::~Simulation() {
  deleteShader(renderProgram_);
  deleteShader(positionUpdateProgram_);
  deleteShader(gridInsertProgram_);
  deleteBuffer(position1_);
  deleteBuffer(position2_);
  deleteBuffer(velocity2_);
  deleteBuffer(gridPairs_);
  deleteVAO(vao_);
}

std::int32_t testSwap = 0; // FIXME: just for demonstration purposes.
void ansimproj::Simulation::render(const ansimproj::core::Camera &camera, float dt) const {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glBindVertexArray(vao_);

  //constexpr auto workGroupSize = 100;
  constexpr GLuint numWorkGroups = static_cast<GLuint>(std::ceil(std::sqrt(PARTICLE_COUNT)));// / workGroupSize;

  // Grid Insert Shader test
  glUseProgram(gridInsertProgram_);
  glProgramUniform3f(gridInsertProgram_, 0, 1.0f, 1.0f, 1.0f);
  glProgramUniform3f(gridInsertProgram_, 1, -0.5f, -0.5f, -0.5f);
  glProgramUniform3ui(gridInsertProgram_, 2, 10, 10, 10);
  glProgramUniform1ui(gridInsertProgram_, 3, PARTICLE_COUNT);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, position1_);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, gridPairs_);
  glDispatchCompute(10000, 1, 1);
  glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

  // Position Update Shader test
  glUseProgram(positionUpdateProgram_);
  glProgramUniform1f(positionUpdateProgram_, 0, dt);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, (testSwap % 2 == 0) ? 0 : 2, position1_);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, velocity2_);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, (testSwap % 2 == 0) ? 2 : 0, position2_);
  glDispatchCompute(10000, 1, 1);
  glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
  testSwap++;

  // Simple vert/frag rendering program
  glUseProgram(renderProgram_);
  const auto &view = camera.view();
  const auto &projection = camera.projection();
  constexpr GLuint mvpLoc = 0;
  constexpr GLuint mvLoc = 1;
  const Eigen::Matrix4f mvp = projection * view;
  glProgramUniformMatrix4fv(renderProgram_, mvpLoc, 1, GL_FALSE, mvp.data());
  glProgramUniformMatrix4fv(renderProgram_, mvLoc, 1, GL_FALSE, view.data());
  glDrawArrays(GL_POINTS, 0, PARTICLE_COUNT);
}

GLuint ansimproj::Simulation::createVAO(const GLuint &vbo) const {
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

void ansimproj::Simulation::deleteVAO(GLuint handle) {
  glDeleteVertexArrays(1, &handle);
}

void ansimproj::Simulation::resize(std::uint32_t width, std::uint32_t height) {
  glViewport(0, 0, width, height);
}
