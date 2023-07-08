#include "student_code.h"
#include "mutablePriorityQueue.h"

using namespace std;

namespace CGL
{

  /**
   * Evaluates one step of the de Casteljau's algorithm using the given points and
   * the scalar parameter t (class member).
   *
   * @param points A vector of points in 2D
   * @return A vector containing intermediate points or the final interpolated vector
   */
  std::vector<Vector2D> BezierCurve::evaluateStep(std::vector<Vector2D> const &points)
  {
    // TODO Task 1.
    std::vector<Vector2D> result;
    for (int i = 0; i < points.size() - 1; i++)
    {
      result.push_back(points[i] + (points[i + 1] - points[i]) * t);
    }
    return result;
  }

  /**
   * Evaluates one step of the de Casteljau's algorithm using the given points and
   * the scalar parameter t (function parameter).
   *
   * @param points    A vector of points in 3D
   * @param t         Scalar interpolation parameter
   * @return A vector containing intermediate points or the final interpolated vector
   */
  std::vector<Vector3D> BezierPatch::evaluateStep(std::vector<Vector3D> const &points, double t) const
  {
    // TODO Task 2.
    std::vector<Vector3D> result;
    for (int i = 0; i < points.size() - 1; i++)
    {
      result.push_back(points[i] + (points[i + 1] - points[i]) * t);
    }
    return result;
  }

  /**
   * Fully evaluates de Casteljau's algorithm for a vector of points at scalar parameter t
   *
   * @param points    A vector of points in 3D
   * @param t         Scalar interpolation parameter
   * @return Final interpolated vector
   */
  Vector3D BezierPatch::evaluate1D(std::vector<Vector3D> const &points, double t) const
  {
    // TODO Task 2.
    std::vector<std::vector<Vector3D>> evaluatedLevels;
    evaluatedLevels.push_back(points);
    std::vector<Vector3D> tmp = points;
    for (int i = 0; i < 3; i++)
    {
      tmp = evaluateStep(tmp, t);
      evaluatedLevels.push_back(tmp);
    }
    std::vector<Vector3D> &lastLevel = evaluatedLevels[evaluatedLevels.size() - 1];
    return lastLevel[0];
  }

  /**
   * Evaluates the Bezier patch at parameter (u, v)
   *
   * @param u         Scalar interpolation parameter
   * @param v         Scalar interpolation parameter (along the other axis)
   * @return Final interpolated vector
   */
  Vector3D BezierPatch::evaluate(double u, double v) const
  {
    // TODO Task 2.
    Vector3D v0, v1, v2, v3;
    v0 = evaluate1D(controlPoints[0], u);
    v1 = evaluate1D(controlPoints[1], u);
    v2 = evaluate1D(controlPoints[2], u);
    v3 = evaluate1D(controlPoints[3], u);
    return evaluate1D({v0, v1, v2, v3}, v);
  }

  Vector3D Vertex::normal(void) const
  {
    // TODO Task 3.
    // Returns an approximate unit normal at this vertex, computed by
    // taking the area-weighted average of the normals of neighboring
    // triangles, then normalizing.
    Vector3D normal(0., 0., 0.);
    HalfedgeCIter h = halfedge();
    do
    {
      // traverse all faces around a vertex
      if (!h->face()->isBoundary())
      {
        HalfedgeCIter ha = h->face()->halfedge();
        // traverse all edges around a face
        do
        {
          Vector3D pi = ha->vertex()->position;
          Vector3D pj = ha->next()->vertex()->position;

          normal += cross(pi, pj); // unit() is the normal vector, norm() is the double area of the triangle

          ha = ha->next();
        } while (ha != h->face()->halfedge());
      }
      h = h->twin()->next();
    } while (h != halfedge());

    return normal.unit();
  }

  EdgeIter HalfedgeMesh::flipEdge(EdgeIter e0)
  {
    // TODO Task 4.
    // This method should flip the given edge and return an iterator to the flipped edge.

    if (e0->isBoundary())
      return e0;

    // Phase 1: collect elements
    // halfedges
    HalfedgeIter h0 = e0->halfedge();
    HalfedgeIter h1 = h0->next();
    HalfedgeIter h2 = h1->next();
    HalfedgeIter h3 = h0->twin();
    HalfedgeIter h4 = h3->next();
    HalfedgeIter h5 = h4->next();
    HalfedgeIter h6 = h1->twin();
    HalfedgeIter h7 = h2->twin();
    HalfedgeIter h8 = h4->twin();
    HalfedgeIter h9 = h5->twin();

    // vertices
    VertexIter v0 = h0->vertex();
    VertexIter v1 = h3->vertex();
    VertexIter v2 = h6->vertex();
    VertexIter v3 = h8->vertex();

    // edges
    EdgeIter e1 = h1->edge();
    EdgeIter e2 = h2->edge();
    EdgeIter e3 = h4->edge();
    EdgeIter e4 = h5->edge();

    // faces
    FaceIter f0 = h0->face();
    FaceIter f1 = h3->face();

    // Phase 2: reassign elements
    // halfedges
    h0->next() = h1;
    h0->twin() = h3;
    h0->vertex() = v3;
    h0->edge() = e0;
    h0->face() = f0;

    h1->next() = h2;
    h1->twin() = h7;
    h1->vertex() = v2;
    h1->edge() = e2;
    h1->face() = f0;

    h2->next() = h0;
    h2->twin() = h8;
    h2->vertex() = v0;
    h2->edge() = e3;
    h2->face() = f0;

    h3->next() = h4;
    h3->twin() = h0;
    h3->vertex() = v2;
    h3->edge() = e0;
    h3->face() = f1;

    h4->next() = h5;
    h4->twin() = h9;
    h4->vertex() = v3;
    h4->edge() = e4;
    h4->face() = f1;

    h5->next() = h3;
    h5->twin() = h6;
    h5->vertex() = v1;
    h5->edge() = e1;
    h5->face() = f1;

    h6->next() = h6->next();
    h6->twin() = h5;
    h6->vertex() = v2;
    h6->edge() = e1;
    h6->face() = h6->face();

    h7->next() = h7->next();
    h7->twin() = h1;
    h7->vertex() = v0;
    h7->edge() = e2;
    h7->face() = h7->face();

    h8->next() = h8->next();
    h8->twin() = h2;
    h8->vertex() = v3;
    h8->edge() = e3;
    h8->face() = h8->face();

    h9->next() = h9->next(); // didn’t change, but set it anyway!
    h9->twin() = h4;
    h9->vertex() = v1;
    h9->edge() = e4;
    h9->face() = h9->face(); // didn’t change, but set it anyway!

    // vertices
    v0->halfedge() = h2;
    v1->halfedge() = h5;
    v2->halfedge() = h3;
    v3->halfedge() = h0;

    // edges
    e0->halfedge() = h0;
    e1->halfedge() = h5;
    e2->halfedge() = h1;
    e3->halfedge() = h2;
    e4->halfedge() = h4;

    // faces
    f0->halfedge() = h0;
    f1->halfedge() = h3;

    return e0;
  }

  VertexIter HalfedgeMesh::splitEdge(EdgeIter e0)
  {
    // TODO Task 5.
    // This method should split the given edge and return an iterator to the newly inserted vertex.
    // The halfedge of this vertex should point along the edge that was split, rather than the new edges.
    if (e0->isBoundary())
    {
      // Phase 1a: collect elements
      // halfedges
      HalfedgeIter h0;
      if (e0->halfedge()->isBoundary())
        h0 = e0->halfedge()->twin();
      else
        h0 = e0->halfedge();
      HalfedgeIter h1 = h0->next();
      HalfedgeIter h2 = h1->next();
      HalfedgeIter h3 = h0->twin();
      HalfedgeIter h4 = h2->twin();
      HalfedgeIter h5 = h1->twin();

      // vertices
      VertexIter v0 = h1->vertex();
      VertexIter v1 = h0->vertex();
      VertexIter v2 = h2->vertex();

      // edges
      EdgeIter e1 = h1->edge();
      EdgeIter e2 = h2->edge();

      // faces
      FaceIter f0 = h0->face();

      // Phase 1b: allocate new elements
      VertexIter v3 = newVertex();

      HalfedgeIter h6 = newHalfedge();
      HalfedgeIter h7 = newHalfedge();
      HalfedgeIter h8 = newHalfedge();
      HalfedgeIter h9 = newHalfedge();

      EdgeIter e3 = newEdge();
      EdgeIter e4 = newEdge();

      FaceIter f1 = newFace();

      // Phase 2a: reassign elements

      h3->face()->halfedge() = h8;
      HalfedgeIter h = h3;
      while (h->next() != h3)
        h = h->next();
      h->next() = h9;

      // halfedges
      h8->next() = h3->next();
      h8->twin() = h0;
      h8->vertex() = v3;
      h8->edge() = e3;
      h8->face() = h3->face();

      h9->next() = h8;
      h9->twin() = h3;
      h9->vertex() = v0;
      h9->edge() = e4;
      h9->face() = h3->face();

      h0->next() = h1;
      h0->twin() = h8;
      h0->vertex() = v1;
      h0->edge() = e3;
      h0->face() = f0;

      h1->next() = h2;
      h1->twin() = h7;
      h1->vertex() = v3;
      h1->edge() = e0;
      h1->face() = f0;

      h2->next() = h0;
      h2->twin() = h4;
      h2->vertex() = v2;
      h2->edge() = e2;
      h2->face() = f0;

      h3->next() = h6;
      h3->twin() = h9;
      h3->vertex() = v3;
      h3->edge() = e4;
      h3->face() = f1;

      h6->next() = h7;
      h6->twin() = h5;
      h6->vertex() = v0;
      h6->edge() = e1;
      h6->face() = f1;

      h7->next() = h3;
      h7->twin() = h1;
      h7->vertex() = v2;
      h7->edge() = e0;
      h7->face() = f1;

      h4->next() = h4->next();
      h4->twin() = h2;
      h4->vertex() = v1;
      h4->edge() = e2;
      h4->face() = h4->face();

      h5->next() = h5->next();
      h5->twin() = h6;
      h5->vertex() = v2;
      h5->edge() = e1;
      h5->face() = h5->face();

      // vertices
      v0->halfedge() = h6;
      v1->halfedge() = h0;
      v2->halfedge() = h2;
      v3->halfedge() = h8;
      v3->position = 0.5 * (v0->position + v1->position);

      // edges
      e0->halfedge() = h1;
      e0->isNew = true;
      e1->halfedge() = h6;
      e2->halfedge() = h2;
      e3->halfedge() = h0;
      e4->halfedge() = h3;

      // faces
      f0->halfedge() = h0;
      f1->halfedge() = h3;

      return v3;
    }
    else
    {
      // Phase 1a: collect elements
      // halfedges
      HalfedgeIter h0 = e0->halfedge();
      HalfedgeIter h1 = h0->next();
      HalfedgeIter h2 = h1->next();
      HalfedgeIter h3 = h0->twin();
      HalfedgeIter h4 = h3->next();
      HalfedgeIter h5 = h4->next();
      HalfedgeIter h6 = h1->twin();
      HalfedgeIter h7 = h2->twin();
      HalfedgeIter h8 = h4->twin();
      HalfedgeIter h9 = h5->twin();

      // vertices
      VertexIter v0 = h0->vertex();
      VertexIter v1 = h3->vertex();
      VertexIter v2 = h6->vertex();
      VertexIter v3 = h8->vertex();

      // edges
      EdgeIter e1 = h1->edge();
      EdgeIter e2 = h2->edge();
      EdgeIter e3 = h4->edge();
      EdgeIter e4 = h5->edge();

      // faces
      FaceIter f0 = h0->face();
      FaceIter f1 = h3->face();

      // Phase 1b: allocate new elements
      VertexIter v4 = newVertex();

      HalfedgeIter h10 = newHalfedge();
      HalfedgeIter h11 = newHalfedge();
      HalfedgeIter h12 = newHalfedge();
      HalfedgeIter h13 = newHalfedge();
      HalfedgeIter h14 = newHalfedge();
      HalfedgeIter h15 = newHalfedge();

      EdgeIter e5 = newEdge();
      EdgeIter e6 = newEdge();
      EdgeIter e7 = newEdge();

      FaceIter f2 = newFace();
      FaceIter f3 = newFace();

      // Phase 2a: reassign elements
      // halfedges
      h0->next() = h1;
      h0->twin() = h13;
      h0->vertex() = v2;
      h0->edge() = e7;
      h0->face() = f0;

      h1->next() = h2;
      h1->twin() = h3;
      h1->vertex() = v4;
      h1->edge() = e0;
      h1->face() = f0;

      h2->next() = h0;
      h2->twin() = h6;
      h2->vertex() = v1;
      h2->edge() = e1;
      h2->face() = f0;

      h3->next() = h4;
      h3->twin() = h1;
      h3->vertex() = v1;
      h3->edge() = e0;
      h3->face() = f1;

      h4->next() = h5;
      h4->twin() = h10;
      h4->vertex() = v4;
      h4->edge() = e6;
      h4->face() = f1;

      h5->next() = h3;
      h5->twin() = h9;
      h5->vertex() = v3;
      h5->edge() = e4;
      h5->face() = f1;

      h6->next() = h6->next();
      h6->twin() = h2;
      h6->vertex() = v2;
      h6->edge() = e1;
      h6->face() = h6->face();

      h7->next() = h7->next();
      h7->twin() = h14;
      h7->vertex() = v0;
      h7->edge() = e2;
      h7->face() = h7->face();

      h8->next() = h8->next();
      h8->twin() = h12;
      h8->vertex() = v3;
      h8->edge() = e3;
      h8->face() = h8->face();

      h9->next() = h9->next();
      h9->twin() = h5;
      h9->vertex() = v1;
      h9->edge() = e4;
      h9->face() = h9->face();

      h10->next() = h11;
      h10->twin() = h4;
      h10->vertex() = v3;
      h10->edge() = e6;
      h10->face() = f2;

      h11->next() = h12;
      h11->twin() = h15;
      h11->vertex() = v4;
      h11->edge() = e5;
      h11->face() = f2;

      h12->next() = h10;
      h12->twin() = h8;
      h12->vertex() = v0;
      h12->edge() = e3;
      h12->face() = f2;

      h13->next() = h14;
      h13->twin() = h0;
      h13->vertex() = v4;
      h13->edge() = e7;
      h13->face() = f3;

      h14->next() = h15;
      h14->twin() = h7;
      h14->vertex() = v2;
      h14->edge() = e2;
      h14->face() = f3;

      h15->next() = h13;
      h15->twin() = h11;
      h15->vertex() = v0;
      h15->edge() = e5;
      h15->face() = f3;

      // vertices
      v0->halfedge() = h15;
      v1->halfedge() = h3;
      v2->halfedge() = h0;
      v3->halfedge() = h10;
      v4->halfedge() = h1;
      v4->position = 0.5 * (v0->position + v1->position);

      // edges
      e0->halfedge() = h1;
      e1->halfedge() = h2;
      e2->halfedge() = h7;
      e3->halfedge() = h8;
      e4->halfedge() = h5;
      e5->halfedge() = h11;
      e6->halfedge() = h4;
      e6->isNew = true;
      e7->halfedge() = h0;
      e7->isNew = true;

      // faces
      f0->halfedge() = h0;
      f1->halfedge() = h3;
      f2->halfedge() = h10;
      f3->halfedge() = h13;

      return v4;
    }
  }

  void MeshResampler::upsample(HalfedgeMesh &mesh)
  {
    // TODO Task 6.
    // This routine should increase the number of triangles in the mesh using Loop subdivision.
    // One possible solution is to break up the method as listed below.

    // 1. Compute new positions for all the vertices in the input mesh, using the Loop subdivision rule,
    // and store them in Vertex::newPosition. At this point, we also want to mark each vertex as being
    // a vertex of the original mesh.
    for (VertexIter v = mesh.verticesBegin(); v != mesh.verticesEnd(); v++)
    {
      std::vector<Vector3D> vertices;
      std::vector<Vector3D> boundaryVertices;
      HalfedgeIter h = v->halfedge();
      int n = 0;
      do
      {
        // check if the current halfedge is on the boundary
        if (h->isBoundary())
        {
          boundaryVertices.push_back(h->twin()->vertex()->position);
        }
        else if (h->twin()->isBoundary())
        {
          boundaryVertices.push_back(h->twin()->vertex()->position);
        }
        else
        {
          vertices.push_back(h->twin()->vertex()->position);
          n++;
        }

        // move to the next halfedge around the vertex
        h = h->twin()->next();
      } while (h != v->halfedge()); // done iterating over halfedges

      if (!boundaryVertices.empty())
      {
        if (boundaryVertices.size() < 2)
          printf("error: boundaryVertices.size()\n");
        v->newPosition = 0.125 * (boundaryVertices[0] + boundaryVertices[boundaryVertices.size() - 1]) + 0.75 * v->position;
      }
      else
      {
        double u;
        if (n == 3)
          u = 0.1875;
        else
          u = 3.0 / (8.0 * n);
        Vector3D newPos = (1.0 - n * u) * v->position;
        for (int i = 0; i < n; i++)
        {
          newPos += u * vertices[i];
        }
        v->newPosition = newPos;
      }
    }

    // 2. Compute the updated vertex positions associated with edges, and store it in Edge::newPosition.
    for (EdgeIter e = mesh.edgesBegin(); e != mesh.edgesEnd(); e++)
    {
      Vector3D v0 = e->halfedge()->vertex()->position;
      Vector3D v1 = e->halfedge()->twin()->vertex()->position;
      e->isNew = false;
      if (e->isBoundary())
        e->newPosition = 0.5 * (v0 + v1);
      else
      {
        Vector3D v2 = e->halfedge()->next()->next()->vertex()->position;
        Vector3D v3 = e->halfedge()->twin()->next()->next()->vertex()->position;
        e->newPosition = 0.375 * (v0 + v1) + 0.125 * (v2 + v3);
      }
    }

    // 3. Split every edge in the mesh, in any order. For future reference, we're also going to store some
    // information about which subdivide edges come from splitting an edge in the original mesh, and which edges
    // are new, by setting the flat Edge::isNew. Note that in this loop, we only want to iterate over edges of
    // the original mesh---otherwise, we'll end up splitting edges that we just split (and the loop will never end!)
    vector<EdgeIter> splitlist;
    for (EdgeIter e = mesh.edgesBegin(); e != mesh.edgesEnd(); e++)
    {
      splitlist.push_back(e);
    }
    for (EdgeIter e : splitlist)
    {
      Vector3D newPos = e->newPosition;
      VertexIter newVet = mesh.splitEdge(e);
      newVet->position = newPos;
      newVet->isNew = true;
    }

    // 4. Flip any new edge that connects an old and new vertex.
    vector<EdgeIter> fliplist;
    for (EdgeIter e = mesh.edgesBegin(); e != mesh.edgesEnd(); e++)
    {
      if (!e->isNew)
        continue;
      bool n0 = e->halfedge()->vertex()->isNew;
      bool n1 = e->halfedge()->twin()->vertex()->isNew;
      if (n0 ^ n1)
        fliplist.push_back(e);
      e->isNew = false;
    }
    for (EdgeIter e : fliplist)
    {
      mesh.flipEdge(e);
    }

    // 5. Copy the new vertex positions into final Vertex::position.
    for (VertexIter v = mesh.verticesBegin(); v != mesh.verticesEnd(); v++)
    {
      if (!v->isNew)
      {
        v->position = v->newPosition;
      }
      else
        v->isNew = false;
    }
  }
}
