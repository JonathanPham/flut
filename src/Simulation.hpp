#pragma once

#include "core/BaseRenderer.hpp"

namespace ansimproj {

  class Simulation : public core::BaseRenderer {

  private:
    constexpr static std::uint32_t PARTICLE_COUNT = 131072;
    static_assert(!(PARTICLE_COUNT & (PARTICLE_COUNT - 1)),
      "PARTICLE_COUNT must be 2^N because of bitonic mergesort.");

  public:
    Simulation();

    ~Simulation();

    void render(const core::Camera &camera, float dt) override;

    void resize(std::uint32_t width, std::uint32_t height);

  private:
    ::gl::GLuint createVAO(const ::gl::GLuint &vbo) const;

    void deleteVAO(::gl::GLuint handle);

  private:
    ::gl::GLuint renderProgram_;
    ::gl::GLuint positionUpdateProgram_;
    ::gl::GLuint gridInsertProgram_;
    ::gl::GLuint gridSortProgram_;
    ::gl::GLuint gridIndexingProgram_;
    ::gl::GLuint position1_;
    ::gl::GLuint velocity1_;
    ::gl::GLuint velocity2_;
    ::gl::GLuint position2_;
    ::gl::GLuint gridPairs_;
    ::gl::GLuint gridIndices_;
    ::gl::GLuint vao_;
    bool swapTextures_;
  };
}
