#include "pathtracer.h"

#include "scene/light.h"
#include "scene/sphere.h"
#include "scene/triangle.h"


using namespace CGL::SceneObjects;

namespace CGL {

PathTracer::PathTracer() {
  gridSampler = new UniformGridSampler2D();
  hemisphereSampler = new UniformHemisphereSampler3D();

  tm_gamma = 2.2f;
  tm_level = 1.0f;
  tm_key = 0.18;
  tm_wht = 5.0f;
}

PathTracer::~PathTracer() {
  delete gridSampler;
  delete hemisphereSampler;
}

void PathTracer::set_frame_size(size_t width, size_t height) {
  sampleBuffer.resize(width, height);
  sampleCountBuffer.resize(width * height);
}

void PathTracer::clear() {
  bvh = NULL;
  scene = NULL;
  camera = NULL;
  sampleBuffer.clear();
  sampleCountBuffer.clear();
  sampleBuffer.resize(0, 0);
  sampleCountBuffer.resize(0, 0);
}

void PathTracer::write_to_framebuffer(ImageBuffer &framebuffer, size_t x0,
                                      size_t y0, size_t x1, size_t y1) {
  sampleBuffer.toColor(framebuffer, x0, y0, x1, y1);
}

Vector3D
PathTracer::estimate_direct_lighting_hemisphere(const Ray &r,
                                                const Intersection &isect) {
  // Estimate the lighting from this intersection coming directly from a light.
  // For this function, sample uniformly in a hemisphere.

  // Note: When comparing Cornel Box (CBxxx.dae) results to importance sampling, you may find the "glow" around the light source is gone.
  // This is totally fine: the area lights in importance sampling has directionality, however in hemisphere sampling we don't model this behaviour.

  // make a coordinate system for a hit point
  // with N aligned with the Z direction.
  Matrix3x3 o2w;
  make_coord_space(o2w, isect.n);
  Matrix3x3 w2o = o2w.T();

  // w_out points towards the source of the ray (e.g.,
  // toward the camera if this is a primary ray)
  const Vector3D hit_p = r.o + r.d * isect.t;
  const Vector3D w_out = w2o * (-r.d);

  // This is the same number of total samples as
  // estimate_direct_lighting_importance (outside of delta lights). We keep the
  // same number of samples for clarity of comparison.
  int num_samples = scene->lights.size() * ns_area_light;
  Vector3D L_out = {0.0};
  
  // TODO (Part 3): Write your sampling loop here
  for (int i = 0; i < num_samples; i++){
    Vector3D sample = hemisphereSampler->get_sample();
    Ray ray_w(hit_p + EPS_D * o2w * sample, o2w * sample); 
    Intersection* in = new Intersection;
    if (bvh->intersect(ray_w, in)) {
      L_out += isect.bsdf->f(w_out, -sample) * in->bsdf->get_emission() * cos_theta(sample) * 2 * PI / num_samples; // / num_samples
    }
    delete in;
  }
  // TODO BEFORE YOU BEGIN
  // UPDATE `est_radiance_global_illumination` to return direct lighting instead of normal shading 

  return L_out;

}

Vector3D
PathTracer::estimate_direct_lighting_importance(const Ray &r,
                                                const Intersection &isect) {
  // Estimate the lighting from this intersection coming directly from a light.
  // To implement importance sampling, sample only from lights, not uniformly in
  // a hemisphere.

  // make a coordinate system for a hit point
  // with N aligned with the Z direction.
  Matrix3x3 o2w;
  make_coord_space(o2w, isect.n);
  Matrix3x3 w2o = o2w.T();

  // w_out points towards the source of the ray (e.g.,
  // toward the camera if this is a primary ray)
  const Vector3D hit_p = r.o + r.d * isect.t;
  const Vector3D w_out = w2o * (-r.d);
  // int num_samples = scene->lights.size() * ns_area_light;
  Vector3D L_out = {0.0};
  
  // TODO (Part 3): Write your sampling loop here
  for (auto light: scene->lights){
    if (light->is_delta_light()) {  // point light
      Vector3D wi;
      double distance;
      double pdf;
      Vector3D radiance = light->sample_L(hit_p, &wi, &distance, &pdf);
      Intersection in;
      Ray ray_w(hit_p + EPS_D * wi, wi); 
      ray_w.max_t = distance - EPS_D;
      // if (bvh->intersect(ray_w, &in)) {
      //   L_out += isect.bsdf->f(w_out, w2o * wi) * in.bsdf->get_emission() * cos_theta(w2o * wi) / pdf / scene->lights.size();
      // }
      if (!bvh->intersect(ray_w, &in)) {
        L_out += isect.bsdf->f(w_out, w2o * wi) * radiance * cos_theta(w2o * wi) / pdf;
      }
    }
    else {                          // area light
      for (int i = 0; i < ns_area_light; i++){
        Vector3D wi;
        double distance;
        double pdf;
        Vector3D radiance = light->sample_L(hit_p, &wi, &distance, &pdf);
        Intersection in;
        Ray ray_w(hit_p + EPS_D * wi, wi); 
        ray_w.max_t = distance - EPS_D;
        // if (bvh->intersect(ray_w, &in)) {
        //   L_out += isect.bsdf->f(w_out, w2o * wi) * in.bsdf->get_emission() * cos_theta(w2o * wi) / pdf / scene->lights.size();
        // }
        if (!bvh->intersect(ray_w, &in)) {
          L_out += isect.bsdf->f(w_out, w2o * wi) * radiance * cos_theta(w2o * wi) / pdf / ns_area_light;
        }
      }
    }
  }
  // TODO BEFORE YOU BEGIN
  // UPDATE `est_radiance_global_illumination` to return direct lighting instead of normal shading 

  return L_out;

}

Vector3D PathTracer::zero_bounce_radiance(const Ray &r,
                                          const Intersection &isect) {
  // TODO: Part 3, Task 2
  // Returns the light that results from no bounces of light

  return isect.bsdf->get_emission();
  // return Vector3D(1.0);
}

Vector3D PathTracer::one_bounce_radiance(const Ray &r,
                                         const Intersection &isect) {
  // TODO: Part 3, Task 3
  // Returns either the direct illumination by hemisphere or importance sampling
  // depending on `direct_hemisphere_sample`
  if (direct_hemisphere_sample){
    return estimate_direct_lighting_hemisphere(r, isect);
  }
  else {
    return estimate_direct_lighting_importance(r, isect);
  }

}

Vector3D PathTracer::at_least_one_bounce_radiance(const Ray &r,
                                                  const Intersection &isect) {
  Matrix3x3 o2w;
  make_coord_space(o2w, isect.n);
  Matrix3x3 w2o = o2w.T();

  Vector3D hit_p = r.o + r.d * isect.t;
  Vector3D w_out = w2o * (-r.d);

  Vector3D L_out(0, 0, 0);

  // TODO: Part 4, Task 2
  // Returns the one bounce radiance + radiance from extra bounces at this point.
  // Should be called recursively to simulate extra bounces.
  L_out += one_bounce_radiance(r, isect);
  Vector3D wi;
  double pdf;
  isect.bsdf->sample_f(w_out, &wi, &pdf);
  Intersection in;
  Ray ray_w(hit_p + EPS_D * wi, wi); 
  ray_w.depth = r.depth + 1;

  if (!bvh->intersect(ray_w, &in)) return L_out;
  if (coin_flip(0.375) && ray_w.depth < max_ray_depth){ //  / 0.375
    L_out += at_least_one_bounce_radiance(ray_w, in) * isect.bsdf->f(w_out, wi) * cos_theta(wi) / pdf / 0.375;
  }
  return L_out;
}

Vector3D PathTracer::est_radiance_global_illumination(const Ray &r) {
  Intersection isect;
  Vector3D L_out;

  // You will extend this in assignment 3-2.
  // If no intersection occurs, we simply return black.
  // This changes if you implement hemispherical lighting for extra credit.

  // The following line of code returns a debug color depending
  // on whether ray intersection with triangles or spheres has
  // been implemented.
  //
  // REMOVE THIS LINE when you are ready to begin Part 3.
  
  if (!bvh->intersect(r, &isect))
    return envLight ? envLight->sample_dir(r) : L_out;


  // L_out = (isect.t == INF_D) ? debug_shading(r.d) : normal_shading(isect.n);

  // TODO (Part 3): Return the direct illumination.
  // L_out = (isect.t == INF_D) ? debug_shading(r.d) : zero_bounce_radiance(r, isect);
  // L_out = (isect.t == INF_D) ? debug_shading(r.d) : zero_bounce_radiance(r, isect) + one_bounce_radiance(r, isect);

  // TODO (Part 4): Accumulate the "direct" and "indirect"
  // parts of global illumination into L_out rather than just direct
  L_out = zero_bounce_radiance(r, isect) + at_least_one_bounce_radiance(r, isect);
  
  return L_out;
}

void PathTracer::raytrace_pixel(size_t x, size_t y) {
  

  // TODO (Part 5):
  // Modify your implementation to include adaptive sampling.
  // Use the command line parameters "samplesPerBatch" and "maxTolerance"

  int num_samples = 0;          // total samples to evaluate
  Vector2D origin = Vector2D(x, y); // bottom left corner of the pixel

  // TODO (Part 1.2):
  // Make a loop that generates num_samples camera rays and traces them
  // through the scene. Return the average Vector3D.
  // You should call est_radiance_global_illumination in this function.
  
  Vector3D finalColor = {0};
  double s1 = 0;
  double s2 = 0;
  double mean;
  double variance;
  double I;

  for (int i = 0; i < ns_aa; i++){
    num_samples ++;
    Vector2D sample = gridSampler->get_sample();
    double dx = (x + sample[0]) / sampleBuffer.w;
    double dy = (y + sample[1]) / sampleBuffer.h;
    Ray r = camera->generate_ray(dx, dy);
    Vector3D radians = est_radiance_global_illumination(r);
    float xk = radians.illum();
    finalColor += radians;
    s1 += xk;
    s2 += xk * xk;
    if (num_samples % samplesPerBatch == 0){
      mean = s1 / num_samples;
      variance = 1.0 / (num_samples - 1) * (s2 - s1 * s1 /num_samples);
      I = 1.96 * sqrt(variance) / sqrt(num_samples);
      if (I <= maxTolerance * mean) break;
    }
  }
  finalColor = finalColor / num_samples;

  sampleBuffer.update_pixel(finalColor, x, y);
  sampleCountBuffer[x + y * sampleBuffer.w] = num_samples;

}

void PathTracer::autofocus(Vector2D loc) {
  Ray r = camera->generate_ray(loc.x / sampleBuffer.w, loc.y / sampleBuffer.h);
  Intersection isect;

  bvh->intersect(r, &isect);

  camera->focalDistance = isect.t;
}

} // namespace CGL
