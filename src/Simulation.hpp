#pragma once

#include "core/BaseRenderer.hpp"

namespace ansimproj {

  class Simulation : public core::BaseRenderer {

  private:
    constexpr static std::uint64_t PARTICLE_COUNT = 100 * 100 * 100;

  public:
    Simulation();

    ~Simulation();

    void render(const core::Camera &camera, float dt) const override;

    void resize(std::uint32_t width, std::uint32_t height);

  private:
    ::gl::GLuint createVAO(const ::gl::GLuint &vbo) const;

    void deleteVAO(::gl::GLuint handle);

  private:
    ::gl::GLuint renderProgram_;
    ::gl::GLuint positionUpdateProgram_;
    ::gl::GLuint gridInsertProgram_;
    ::gl::GLuint position1_;
    ::gl::GLuint velocity2_;
    ::gl::GLuint position2_;
    ::gl::GLuint gridPairs_;
    ::gl::GLuint vao_;
  };
}
