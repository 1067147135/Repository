#include "camera.h"

#include <iostream>
#include <sstream>
#include <fstream>

#include "CGL/misc.h"
#include "CGL/vector2D.h"
#include "CGL/vector3D.h"

using std::cout;
using std::endl;
using std::max;
using std::min;
using std::ifstream;
using std::ofstream;

namespace CGL {

using Collada::CameraInfo;

Ray Camera::generate_ray_for_thin_lens(double x, double y, double rndR, double rndTheta) const {

  // TODO Assignment 7: Part 4
  // compute position and direction of ray from the input sensor sample coordinate.
  // Note: use rndR and rndTheta to uniformly sample a unit disk.
  Vector3D SensorPos = {(x - 0.5) * tan(0.5 * radians(hFov)) * 2, (y - 0.5) * tan(0.5 * radians(vFov)) * 2, -1};
  // Vector3D WorldPos = c2w * SensorPos + pos; 
  // Vector3D WorldDir = (c2w * SensorPos).unit();
  // Ray red(WorldPos, WorldDir);

  Vector3D pLens = {lensRadius * sqrt(rndR) * cos(rndTheta), lensRadius * sqrt(rndR) * sin(rndTheta), 0};
  Vector3D pFocus = {SensorPos.x * focalDistance, SensorPos.y * focalDistance, SensorPos.z * focalDistance};
  Ray blue(c2w * pLens + pos, (c2w * (pFocus - pLens)).unit());
  blue.min_t = nClip;
  blue.max_t = fClip;
  return blue;
  // return Ray(pos, Vector3D(0, 0, -1));
}


} // namespace CGL
