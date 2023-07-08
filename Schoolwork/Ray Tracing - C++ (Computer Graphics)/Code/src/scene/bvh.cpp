#include "bvh.h"

#include "CGL/CGL.h"
#include "triangle.h"

#include <iostream>
#include <stack>

using namespace std;

namespace CGL {
namespace SceneObjects {

BVHAccel::BVHAccel(const std::vector<Primitive *> &_primitives,
                   size_t max_leaf_size) {

  primitives = std::vector<Primitive *>(_primitives);
  root = construct_bvh(primitives.begin(), primitives.end(), max_leaf_size);
}

BVHAccel::~BVHAccel() {
  if (root)
    delete root;
  primitives.clear();
}

BBox BVHAccel::get_bbox() const { return root->bb; }

void BVHAccel::draw(BVHNode *node, const Color &c, float alpha) const {
  if (node->isLeaf()) {
    for (auto p = node->start; p != node->end; p++) {
      (*p)->draw(c, alpha);
    }
  } else {
    draw(node->l, c, alpha);
    draw(node->r, c, alpha);
  }
}

void BVHAccel::drawOutline(BVHNode *node, const Color &c, float alpha) const {
  if (node->isLeaf()) {
    for (auto p = node->start; p != node->end; p++) {
      (*p)->drawOutline(c, alpha);
    }
  } else {
    drawOutline(node->l, c, alpha);
    drawOutline(node->r, c, alpha);
  }
}

BVHNode *BVHAccel::construct_bvh(std::vector<Primitive *>::iterator start,
                                 std::vector<Primitive *>::iterator end,
                                 size_t max_leaf_size) {

  // TODO (Part 2.1):
  // Construct a BVH from the given vector of primitives and maximum leaf
  // size configuration. The starter code build a BVH aggregate with a
  // single leaf node (which is also the root) that encloses all the
  // primitives.

  BBox bbox;
  int size = 0;

  for (auto p = start; p != end; p++) {
    BBox bb = (*p)->get_bbox();
    bbox.expand(bb);
    size++;
  }

  BVHNode *node = new BVHNode(bbox);

  if (size == 0){
    return node;
  }
  else if (size <= max_leaf_size){
    node->start = start;
    node->end = end;
    return node;
  }
  else{
    // cout << "before sort: " << &start << endl;
    if (bbox.extent.x >= bbox.extent.y && bbox.extent.x >= bbox.extent.z){
      sort(start, end, [](Primitive * a,Primitive * b){return a->get_bbox().centroid().x < b->get_bbox().centroid().x;});
    }
    else if (bbox.extent.y >= bbox.extent.x && bbox.extent.y >= bbox.extent.z){
      sort(start, end, [](Primitive * a,Primitive * b){return a->get_bbox().centroid().y < b->get_bbox().centroid().y;});
    }
    else{
      sort(start, end, [](Primitive * a,Primitive * b){return a->get_bbox().centroid().z < b->get_bbox().centroid().z;});
    }
    // cout << "after sort: " << &start << endl;
    node->l = construct_bvh(start, start + size/2, max_leaf_size);
    node->r = construct_bvh(start + size/2, end, max_leaf_size);
  }

  // node->start = start;
  // node->end = end;

  return node;


}

bool BVHAccel::has_intersection(const Ray &ray, BVHNode *node) const {
  // TODO (Part 2.3):
  // Fill in the intersect function.
  // Take note that this function has a short-circuit that the
  // Intersection version cannot, since it returns as soon as it finds
  // a hit, it doesn't actually have to find the closest hit.
  double t0 = ray.min_t;
  double t1 = ray.max_t;
  if (node->bb.intersect(ray, t0, t1) == false) {
    return false;
  }
  if (node->l == NULL && node->r == NULL){
    for (auto p = node->start; p != node->end; p++){
      total_isects++;
      if ((*p)->has_intersection(ray))
        return true;
    }
  }
  return (has_intersection(ray, node->r) || has_intersection(ray, node->l));
  
  // for (auto p : primitives) {
  //   total_isects++;
  //   if (p->has_intersection(ray))
  //     return true;
  // }
  // return false;
}

bool BVHAccel::intersect(const Ray &ray, Intersection *i, BVHNode *node) const {
  // TODO (Part 2.3):
  // Fill in the intersect function.

  double t0 = ray.min_t;
  double t1 = ray.max_t;
  if (node->bb.intersect(ray, t0, t1) == false) {
    return false;
  }

  if (node->l == NULL && node->r == NULL){
    bool flag = false;
    Intersection* tmp = new Intersection;
    for (auto p = node->start; p != node->end; p++){
      total_isects++;
      if ((*p)->intersect(ray, tmp)){
        flag = true;
        if (tmp->t < i->t){
          i->t = tmp->t;
          i->primitive = tmp->primitive;
          i->n = tmp->n;
          i->bsdf = tmp->bsdf;
        }
      }
      
    }
    delete tmp;
    return flag;
  }

  Intersection* hit1 = new Intersection;
  Intersection* hit2 = new Intersection;
  
  bool flag1 = intersect(ray, hit1, node->l);
  bool flag2 = intersect(ray, hit2, node->r);
  if (flag1 && flag2){
    if (hit1->t < hit2->t){
      i->t = hit1->t;
      i->primitive = hit1->primitive;
      i->n = hit1->n;
      i->bsdf = hit1->bsdf;
    }
    else {
      i->t = hit2->t;
      i->primitive = hit2->primitive;
      i->n = hit2->n;
      i->bsdf = hit2->bsdf;
    }
  }
  else if (flag1){
    i->t = hit1->t;
    i->primitive = hit1->primitive;
    i->n = hit1->n;
    i->bsdf = hit1->bsdf;
  }
  else if (flag2){
    i->t = hit2->t;
    i->primitive = hit2->primitive;
    i->n = hit2->n;
    i->bsdf = hit2->bsdf;
  }
  else {
    delete hit1;
    delete hit2;
    return false;
  }
  delete hit1;
  delete hit2;
  return true;



  // bool hit = false;
  // for (auto p : primitives) {
  //   total_isects++;
  //   hit = p->intersect(ray, i) || hit;
  // }
  // return hit;
}

} // namespace SceneObjects
} // namespace CGL
