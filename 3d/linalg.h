#pragma once
#include <Eigen/Dense>
#include "MathUtil.h"

using Vec2d = Eigen::Vector2d;
using Vec3d = Eigen::Vector3d;
using Vec4d = Eigen::Vector4d;
using Mat3d = Eigen::Matrix3d;

// Calculates the distance from a point to a line
double DistanceToLine(Vec3d const& line_point1, Vec3d const& line_point2) {
  Vec3d line = line_point2 - line_point1;
  double t = line_point1.dot(line) / line.norm();
  if (t < 0) {
    return line_point1.norm();
  } else if (t > 1) {
    return line_point2.norm();
  } else {
    Vec3d projection = line_point1 + t * line;
    return projection.norm();
  }
}

Vec3d Normal(Vec3d const& p1, Vec3d const& p2, Vec3d const& p3) {
  Vec3d cross = (p2-p1).cross(p3-p1);
  cross.normalize();
  return cross;
}

double NormToPoint(Vec3d const& t1, Vec3d const& t2, Vec3d const& t3, Vec3d const& p) {
  // Calculate the normal of the triangle
  Vec3d normal = Normal(t1,t2,t3);
  Vec3d vecToP = (p-t1).normalized();
  double facePoint = -normal.dot(vecToP);
  return facePoint;
}

double ShortestDistance(Vec3d const& t1, Vec3d const& t2, Vec3d const& t3) {
  // Calculate the normal of the triangle
  Vec3d normal = Normal(t1,t2,t3);

  // Calculate the distance from the point to the plane of the triangle
  double d = normal.dot(t1);

  // If the point is on the same side of the plane as the normal, the shortest
  // distance is the distance from the point to the plane
  if (d * normal.dot(t1) > 0) {
    return std::abs(d);
  }

  // Otherwise, the shortest distance is the distance from the point to one of the edges or vertices of the triangle
  return min(DistanceToLine(t1, t2),
		  DistanceToLine(t2, t3),
		  DistanceToLine(t3, t1),
		  t1.norm(), t2.norm(), t3.norm());
}
