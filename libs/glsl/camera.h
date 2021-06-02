#ifndef CAMERA_H_INCLUDED
#define CAMERA_H_INCLUDED

#include "glm/glm.hpp"


namespace glsl {

  class Camera
  {
  public:
    Camera();
    ~Camera();

    glm::dvec3 eye() const;
    glm::dvec3 forward() const;
    glm::dvec3 side() const;
    glm::dvec3 up() const;
    glm::dmat3 R() const;           // Rotational part of camera matrix
    const glm::dmat4& V() const;    // View matrix
    const glm::dmat4& P() const;    // Perspective matrix
    const glm::dvec4& viewport() const;
  
    void setOrthoViewport(const glm::ivec4& viewport, const glm::dvec4& world);
    void setViewport(int w, int h, double fovy, double clipNear, double clipFar);
    void setViewMatrix(const glm::dmat4& V);

  private:
    glm::dmat4 V_;          // View
    glm::dmat4 P_;          // Projection
    glm::dvec4 viewport_;   // full window viewport
  };


  class CameraManip
  {
  public:
    explicit CameraManip(double smooth);

    glm::dvec3 eye() const noexcept { return eye_; }
    glm::dvec3 center() const noexcept { return center_; }
    glm::dvec3 up() const noexcept { return up_; }

    void lookAt(glm::dvec3 eye, glm::dvec3 center = glm::dvec3(0.0), glm::dvec3 up = glm::dvec3(0.0, 1.0, 0.0));
    void moveForward(double zDelta);
    void moveLeft(double xDelta);
    void moveUp(double yDelta);
    void move(double xDelta, double yDelta, double zDelta);
    void moveXYZ(double xDelta, double yDelta, double zDelta);
    void transform(const glm::dmat4& R);
    void rotateLeft(double degDelta);
    void rotateUp(double degDelta);
    void update(Camera& camera, double deltaT);

  private:
    void regenerateUp();

    glm::dvec3 eye_;
    glm::dvec3 center_;
    glm::dvec3 up_;
    double smooth_;
  };

}

#endif
