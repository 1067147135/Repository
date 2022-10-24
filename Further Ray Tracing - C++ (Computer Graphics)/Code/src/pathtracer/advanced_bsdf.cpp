#include "bsdf.h"

#include <algorithm>
#include <iostream>
#include <utility>

#include "application/visual_debugger.h"

using std::max;
using std::min;
using std::swap;

namespace CGL {

// Mirror BSDF //

Vector3D MirrorBSDF::f(const Vector3D wo, const Vector3D wi) {
  return Vector3D();
}

Vector3D MirrorBSDF::sample_f(const Vector3D wo, Vector3D* wi, double* pdf) {

  // TODO Assignment 7: Part 1
  // Implement MirrorBSDF
  reflect(wo, wi);
  *pdf = 1.0;
  return reflectance / abs_cos_theta(*wi);
}

void MirrorBSDF::render_debugger_node()
{
  if (ImGui::TreeNode(this, "Mirror BSDF"))
  {
    DragDouble3("Reflectance", &reflectance[0], 0.005);
    ImGui::TreePop();
  }
}

// Microfacet BSDF //

double MicrofacetBSDF::G(const Vector3D wo, const Vector3D wi) {
  return 1.0 / (1.0 + Lambda(wi) + Lambda(wo));
}

double MicrofacetBSDF::D(const Vector3D h) {
  // TODO Assignment 7: Part 2
  // Compute Beckmann normal distribution function (NDF) here.
  // You will need the roughness alpha.
  return 1.0;
}

Vector3D MicrofacetBSDF::F(const Vector3D wi) {
  // TODO Assignment 7: Part 2
  // Compute Fresnel term for reflection on dielectric-conductor interface.
  // You will need both eta and etaK, both of which are Vector3D.

  return Vector3D();
}

Vector3D MicrofacetBSDF::f(const Vector3D wo, const Vector3D wi) {
  // TODO Assignment 7: Part 2
  // Implement microfacet model here.

  return Vector3D();
}

Vector3D MicrofacetBSDF::sample_f(const Vector3D wo, Vector3D* wi, double* pdf) {
  // TODO Assignment 7: Part 2
  // *Importance* sample Beckmann normal distribution function (NDF) here.
  // Note: You should fill in the sampled direction *wi and the corresponding *pdf,
  //       and return the sampled BRDF value.

  *wi = cosineHemisphereSampler.get_sample(pdf);
  return MicrofacetBSDF::f(wo, *wi);
}

void MicrofacetBSDF::render_debugger_node()
{
  if (ImGui::TreeNode(this, "Micofacet BSDF"))
  {
    DragDouble3("eta", &eta[0], 0.005);
    DragDouble3("K", &k[0], 0.005);
    DragDouble("alpha", &alpha, 0.005);
    ImGui::TreePop();
  }
}

// Refraction BSDF //

Vector3D RefractionBSDF::f(const Vector3D wo, const Vector3D wi) {
  return Vector3D();
}

Vector3D RefractionBSDF::sample_f(const Vector3D wo, Vector3D* wi, double* pdf) {
  // TODO Assignment 7: Part 1
  // Implement RefractionBSDF
  if (refract(wo,wi,ior)){
    double eta;
    if (wo.z > 0) eta = 1.0 / ior;
    else eta = ior;
    *pdf = 1.0;
    return transmittance / abs_cos_theta(*wi) / (eta * eta);
  }
  else return Vector3D(); // Total Internal Reflection
}

void RefractionBSDF::render_debugger_node()
{
  if (ImGui::TreeNode(this, "Refraction BSDF"))
  {
    DragDouble3("Transmittance", &transmittance[0], 0.005);
    DragDouble("ior", &ior, 0.005);
    ImGui::TreePop();
  }
}

// Glass BSDF //

Vector3D GlassBSDF::f(const Vector3D wo, const Vector3D wi) {
  return Vector3D();
}

Vector3D GlassBSDF::sample_f(const Vector3D wo, Vector3D* wi, double* pdf) {

  // TODO Assignment 7: Part 1
  // Compute Fresnel coefficient and either reflect or refract based on it.

  // compute Fresnel coefficient and use it as the probability of reflection
  // - Fundamentals of Computer Graphics page 305
  if (refract(wo, wi, ior) == false){
    reflect(wo, wi);
    *pdf = 1.0;
    return reflectance / abs_cos_theta(*wi);
  }
  else {
    double R0 = (ior - 1) * (ior - 1) / (ior + 1) / (ior + 1);  // reflectance at normal incidence
    double R = R0 + (1 - R0) * pow(1 - abs_cos_theta(wo), 5);   // Schlick's reflection coefficient
    if (coin_flip(R)){
      reflect(wo,wi);
      *pdf = R;
      return R * reflectance / abs_cos_theta(*wi);
    }
    else {
      refract(wo,wi,ior);
      *pdf = 1 - R;
      double eta;
      if (wo.z > 0) eta = 1.0 / ior;
      else eta = ior;
      return (1-R) * transmittance / abs_cos_theta(*wi) / (eta * eta);
    }
  }
  return Vector3D();
}

void GlassBSDF::render_debugger_node()
{
  if (ImGui::TreeNode(this, "Refraction BSDF"))
  {
    DragDouble3("Reflectance", &reflectance[0], 0.005);
    DragDouble3("Transmittance", &transmittance[0], 0.005);
    DragDouble("ior", &ior, 0.005);
    ImGui::TreePop();
  }
}

void BSDF::reflect(const Vector3D wo, Vector3D* wi) {

  // TODO Assignment 7: Part 1
  // Implement reflection of wo about normal (0,0,1) and store result in wi.
  Vector3D n = {0,0,1};
  *wi = -wo + 2 * dot(wo, n) * n;
}

bool BSDF::refract(const Vector3D wo, Vector3D* wi, double ior) {

  // TODO Assignment 7: Part 1
  // Use Snell's Law to refract wo surface and store result ray in wi.
  // Return false if refraction does not occur due to total internal reflection
  // and true otherwise. When dot(wo,n) is positive, then wo corresponds to a
  // ray entering the surface through vacuum.
  double eta;
  if (wo.z > 0) { // enter the non-air material
    eta = 1.0 / ior;
    double delta = 1 - eta * eta * (1 - wo.z * wo.z); 
    if (delta < 0) {
      // *wi = Vector3D();
      return false;
    }
    wi->z = - sqrt(delta);
  } else {        // exit the non-air material
    eta = ior;
    double delta = 1 - eta * eta * (1 - wo.z * wo.z); 
    if (delta < 0) {
      // *wi = Vector3D();
      return false;
    }
    wi->z = sqrt(delta);
  }
  wi->x = -eta * wo.x;
  wi->y = -eta * wo.y;

  return true;

}

} // namespace CGL
