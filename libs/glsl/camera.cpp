#include "camera.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/matrix_interpolation.hpp>


using namespace glm;


namespace glsl {

  Camera::Camera()
    : V_(1.0), P_(1.0),
    viewport_(0.0, 0.0, 1.0, 1.0)
  {
  }


  Camera::~Camera()
  {
  }


  dvec3 Camera::eye() const
  {
    auto R = transpose(dmat3(V_));      // Camera rotation matrix
    return R * -dvec3(V_[3]);
  }


  dvec3 Camera::forward() const
  {
    return dvec3(V_[0][2], V_[1][2], V_[2][2]);
  }


  dvec3 Camera::side() const
  {
    return dvec3(V_[0][0], V_[1][0], V_[2][0]);
  }


  dvec3 Camera::up() const
  {
    return dvec3(V_[0][1], V_[1][1], V_[2][1]);
  }


  glm::dmat3 Camera::R() const 
  { 
    return glm::dmat3(forward(), up(), side()); 
  }


  const glm::dmat4& Camera::V() const 
  { 
    return V_; 
  }


  const glm::dmat4& Camera::P() const 
  { 
    return P_; 
  }


  const glm::dvec4& Camera::viewport() const 
  { 
    return viewport_; 
  }


  void Camera::setViewport(int w, int h, double fovy, double clipNear, double clipFar)
  {
    viewport_ = dvec4{ 0, 0, w, h };
    P_ = perspective(fovy, viewport_[2] / viewport_[3], clipNear, clipFar);
  }


  void Camera::setOrthoViewport(const glm::ivec4& viewport, const glm::dvec4& world)
  {
    viewport_ = dvec4(viewport);
    P_ = ortho(world.x, world.y, world.z, world.w);
  }



  void Camera::setViewMatrix(const dmat4& V)
  {
    V_ = V;
  }


  CameraManip::CameraManip(double smooth)
    : eye_(1, 0, 0), center_(0), up_(0, 1, 0), smooth_(smooth)
  {
  }


  void CameraManip::lookAt(dvec3 eye, dvec3 center, dvec3 up)
  {
    eye_ = eye;
    center_ = center;
    up_ = up;
    regenerateUp();
  }


  void CameraManip::moveForward(double zDelta)
  {

    auto dForward = normalize(center_ - eye_) * zDelta;
    auto dist = length(center_ - (eye_ + dForward));
    if (dist > 2.9f && dist < 10.0f)
    {
      eye_ += dForward;
    }
  }


  void CameraManip::moveLeft(double xDelta)
  {
    auto side = center_ - eye_;
    auto dSide = cross(up_, normalize(center_ - eye_)) * xDelta;
    eye_ += dSide;
    center_ += dSide;
  }


  void CameraManip::moveUp(double yDelta)
  {
    auto dUp = up_ * yDelta;
    eye_ += dUp;
    center_ += dUp;
  }


  void CameraManip::move(double xDelta, double yDelta, double zDelta)
  {
    moveLeft(xDelta);
    moveUp(yDelta);
    moveForward(zDelta);
  }


  void CameraManip::moveXYZ(double xDelta, double yDelta, double zDelta)
  {
    auto s = glm::dvec3(xDelta, yDelta, zDelta);
    eye_ += s;
    center_ += s;
  }


  void CameraManip::transform(const glm::dmat4& R)
  {
    eye_ = center_ - dvec3(R * dvec4(center_ - eye_, 1.0));
    up_ = dvec3(R * dvec4(up_, 1.0));
  }


  void CameraManip::rotateLeft(double degDelta)
  {
    this->transform(glm::rotate(dmat4(1), degDelta, dvec3(0, 1, 0)));
  }


  void CameraManip::rotateUp(double degDelta)
  {
    auto s = cross(up_, normalize(center_ - eye_));
    this->transform(glm::rotate(dmat4(1), degDelta, s));
  }


  void CameraManip::update(Camera& camera, double deltaT)
  {
    regenerateUp();
    deltaT = clamp(smooth_ * deltaT, 0.0, 1.0);
    auto eye = (1.0 - deltaT) * camera.eye() + deltaT * eye_;
    auto up = (1.0 - deltaT) * camera.up() + deltaT * up_;
    auto forward = normalize(center_ - eye);
    auto center = eye + forward;
    camera.setViewMatrix(glm::lookAt(eye, center, up));
  }


  void CameraManip::regenerateUp()
  {
    auto f = normalize(center_ - eye_);
    auto u = normalize(up_);
    auto s = normalize(cross(f, u));
    up_ = cross(s, f);
  }

}
