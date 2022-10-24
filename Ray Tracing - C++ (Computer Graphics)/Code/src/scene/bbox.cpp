#include "bbox.h"

#include "GL/glew.h"

#include <algorithm>
#include <iostream>

namespace CGL {

bool BBox::intersect(const Ray& r, double& t0, double& t1) const {

  // TODO (Part 2.2):
  // Implement ray - bounding box intersection test
  // If the ray intersected the bouding box within the range given by
  // t0, t1, update t0 and t1 with the new intersection times.
  double t_min_x = (min.x - r.o.x) / r.d.x;
  double t_min_y = (min.y - r.o.y) / r.d.y;
  double t_min_z = (min.z - r.o.z) / r.d.z;

  double t_max_x = (max.x - r.o.x) / r.d.x;
  double t_max_y = (max.y - r.o.y) / r.d.y;
  double t_max_z = (max.z - r.o.z) / r.d.z;

  if (t_min_x > t_max_x) std::swap(t_min_x, t_max_x);
  if (t_min_y > t_max_y) std::swap(t_min_y, t_max_y);
  if (t_min_z > t_max_z) std::swap(t_min_z, t_max_z);

  float tmin = std::max(t_min_x, std::max(t_min_y, t_min_z));
  float tmax = std::min(t_max_x, std::min(t_max_y, t_max_z));

  if (tmin > tmax) return false;
  if (tmax < 0) return false;
  if (t0 < tmin) t0 = tmin;
  if (t1 > tmax) t1 = tmax;
  return true;

}

void BBox::draw(Color c, float alpha) const {

  glColor4f(c.r, c.g, c.b, alpha);

  // top
  glBegin(GL_LINE_STRIP);
  glVertex3d(max.x, max.y, max.z);
  glVertex3d(max.x, max.y, min.z);
  glVertex3d(min.x, max.y, min.z);
  glVertex3d(min.x, max.y, max.z);
  glVertex3d(max.x, max.y, max.z);
  glEnd();

  // bottom
  glBegin(GL_LINE_STRIP);
  glVertex3d(min.x, min.y, min.z);
  glVertex3d(min.x, min.y, max.z);
  glVertex3d(max.x, min.y, max.z);
  glVertex3d(max.x, min.y, min.z);
  glVertex3d(min.x, min.y, min.z);
  glEnd();

  // side
  glBegin(GL_LINES);
  glVertex3d(max.x, max.y, max.z);
  glVertex3d(max.x, min.y, max.z);
  glVertex3d(max.x, max.y, min.z);
  glVertex3d(max.x, min.y, min.z);
  glVertex3d(min.x, max.y, min.z);
  glVertex3d(min.x, min.y, min.z);
  glVertex3d(min.x, max.y, max.z);
  glVertex3d(min.x, min.y, max.z);
  glEnd();

}

std::ostream& operator<<(std::ostream& os, const BBox& b) {
  return os << "BBOX(" << b.min << ", " << b.max << ")";
}

} // namespace CGL
