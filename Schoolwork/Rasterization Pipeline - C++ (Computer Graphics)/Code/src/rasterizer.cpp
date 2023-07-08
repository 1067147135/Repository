#include "rasterizer.h"

using namespace std;

namespace CGL {

  RasterizerImp::RasterizerImp(PixelSampleMethod psm, LevelSampleMethod lsm,
    size_t width, size_t height,
    unsigned int sample_rate) {
    this->psm = psm;  // Pixel Sample Method: P_NEAREST / P_LINEAR
    this->lsm = lsm;
    this->width = width;
    this->height = height;
    this->sample_rate = sample_rate;

    sample_buffer.resize(width * height * sample_rate, Color::White);
  }

  // Used by rasterize_point and rasterize_line
  void RasterizerImp::fill_pixel(size_t x, size_t y, Color c) {
    // TODO: Task 2: You might need to this function to fix points and lines (such as the black rectangle border in test4.svg)
    // NOTE: You are not required to implement proper supersampling for points and lines
    // It is sufficient to use the same color for all supersamples of a pixel for points and lines (not triangles)


    sample_buffer[y * width + x] = c;
  }

  // Rasterize a point: simple example to help you start familiarizing
  // yourself with the starter code.
  //
  void RasterizerImp::rasterize_point(float x, float y, Color color) {
    // fill in the nearest pixel
    int sx = (int)floor(x);
    int sy = (int)floor(y);

    // check bounds
    if (sx < 0 || sx >= width) return;
    if (sy < 0 || sy >= height) return;

    fill_pixel(sx, sy, color);
    return;
  }

  static bool insideTriangle(float x, float y, const float x0, const float y0, const float x1, const float y1, const float x2, const float y2){
    Vector3D p = {x, y, 1.0};
    Vector3D v[3];
    v[0] = {x0, y0, 1.0};
    v[1] = {x1, y1, 1.0};
    v[2] = {x2, y2, 1.0};
    Vector3D ab = v[1] - v[0];
    Vector3D bc = v[2] - v[1];
    Vector3D ca = v[0] - v[2];
    Vector3D ap = p - v[0];
    Vector3D bp = p - v[1];
    Vector3D cp = p - v[2];
    // p and c are on the same side of ab && p and b are on the same side of ac && p and a are on the same side of bc
    if ((dot(cross(ab, bc), cross(ab, ap)) >= 0) && (dot(cross(bc, ca), cross(bc, bp)) >= 0) && (dot(cross(ca, ab), cross(ca, cp)) >= 0)){
      return true;
    }
    else {
      return false;
    }
    
  }

  // Rasterize a line.
  void RasterizerImp::rasterize_line(float x0, float y0,
    float x1, float y1,
    Color color) {
    if (x0 > x1) {
      swap(x0, x1); swap(y0, y1);
    }

    float pt[] = { x0,y0 };
    float m = (y1 - y0) / (x1 - x0);
    float dpt[] = { 1,m };
    float steep = abs(m) > 1;
    if (steep) {
      dpt[0] = x1 == x0 ? 0 : 1 / abs(m);
      dpt[1] = x1 == x0 ? (y1 - y0) / abs(y1 - y0) : m / abs(m);
    }

    while (floor(pt[0]) <= floor(x1) && abs(pt[1] - y0) <= abs(y1 - y0)) {
      rasterize_point(pt[0], pt[1], color);
      pt[0] += dpt[0]; pt[1] += dpt[1];
    }
  }

  // Rasterize a triangle.
  void RasterizerImp::rasterize_triangle(float x0, float y0,
    float x1, float y1,
    float x2, float y2,
    Color color) {
    // TODO: Task 1: Implement basic triangle rasterization here, no supersampling
    // Create a 2D bouding box for the triangle
    float min_x = min(x0, min(x1, x2));
    float max_x = max(x0, max(x1, x2));
    float min_y = min(y0, min(y1, y2));
    float max_y = max(y0, max(y1, y2));
    if (sample_rate == 1){ // basic triangle rasterization
      // Traverse all the pixels in the bounding box. 
      // Then use Sample coordinates = the integer point + (0.5,0.5) to check whether it is in the triangle
      for (int x = min_x; x < max_x; x++){
        for (int y = min_y; y < max_y; y++){
          // Check whether the center of the pixel is in the triangle
          if (insideTriangle(x + 0.5, y + 0.5, x0, y0, x1, y1, x2, y2)){
            fill_pixel(x, y, color);
          }
        }
      }
    }
    // TODO: Task 2: Update to implement super-sampled rasterization
    else{
      // sqrt(sample_rate) * sqrt(sample_rate) for each pixel
      vector<Vector2D> pos;
      int n = sqrt(sample_rate);
      for (int i = 0; i < n; i++){
        for (int j = 0; j < n; j++){
          pos.push_back({(i + 0.5) / n, (j + 0.5) / n});
        }
      }
      
      for (int x = min_x; x < max_x; x++){
        for (int y = min_y; y < max_y; y++){
          float count = 0;
          for (int i = 0; i < sample_rate; i++){
            if (insideTriangle(x + pos[i][0], y + pos[i][1], x0, y0, x1, y1, x2, y2)){
              count += 1;
            }
          }
          if (count != 0){
            fill_pixel(x, y, color * (count / sample_rate) + sample_buffer[y * width + x] * ((sample_rate - count) / sample_rate));
          }
        }
      }
    }
  }

  std::tuple<float, float, float> computeBarycentric2D(float x, float y, const float x0, const float y0, const float x1, const float y1, const float x2, const float y2){
    float gamma = ((y0 - y1) * x + (x1 - x0) * y + x0 * y1 - x1 * y0) / ((y0 - y1) * x2 + (x1 - x0) * y2 + x0 * y1 - x1 * y0);
    float beta = ((y0 - y2) * x + (x2 - x0) * y + x0 * y2 - x2 * y0) / ((y0 - y2) * x1 + (x2 - x0) * y1 + x0 * y2 - x2 * y0);
    float alpha = 1 - beta - gamma;
    return {alpha, beta, gamma};
  }

  void RasterizerImp::rasterize_interpolated_color_triangle(float x0, float y0, Color c0,
    float x1, float y1, Color c1,
    float x2, float y2, Color c2)
  {
    // TODO: Task 4: Rasterize the triangle, calculating barycentric coordinates and using them to interpolate vertex colors across the triangle
    // Hint: You can reuse code from rasterize_triangle
    // Create a 2D bouding box for the triangle
    float min_x = min(x0, min(x1, x2));
    float max_x = max(x0, max(x1, x2));
    float min_y = min(y0, min(y1, y2));
    float max_y = max(y0, max(y1, y2));
    if (sample_rate == 1){ // basic triangle rasterization
      // Traverse all the pixels in the bounding box. 
      // Then use Sample coordinates = the integer point + (0.5,0.5) to check whether it is in the triangle
      for (int x = min_x; x < max_x; x++){
        for (int y = min_y; y < max_y; y++){
          // Check whether the center of the pixel is in the triangle
          if (insideTriangle(x + 0.5, y + 0.5, x0, y0, x1, y1, x2, y2)){
            auto tup = computeBarycentric2D(x + 0.5, y + 0.5, x0, y0, x1, y1, x2, y2);
            float alpha;
            float beta;
            float gamma;
            tie(alpha, beta, gamma) = tup;
            fill_pixel(x, y, alpha * c0 + beta * c1 + gamma * c2);
          }
        }
      }
    }
    else{
      // sqrt(sample_rate) * sqrt(sample_rate) for each pixel
      vector<Vector2D> pos;
      int n = sqrt(sample_rate);
      for (int i = 0; i < n; i++){
        for (int j = 0; j < n; j++){
          pos.push_back({(i + 0.5) / n, (j + 0.5) / n});
        }
      }
      
      for (int x = min_x; x < max_x; x++){
        for (int y = min_y; y < max_y; y++){
          float count = 0;
          Color color = {0,0,0};
          for (int i = 0; i < sample_rate; i++){
            if (insideTriangle(x + pos[i][0], y + pos[i][1], x0, y0, x1, y1, x2, y2)){
              auto tup = computeBarycentric2D(x + pos[i][0], y + pos[i][1], x0, y0, x1, y1, x2, y2);
              float alpha;
              float beta;
              float gamma;
              tie(alpha, beta, gamma) = tup;
              color += (alpha * c0 + beta * c1 + gamma * c2) * (1.0 / sample_rate);  
              count += 1;
            }
          }
          if (count != 0){
            fill_pixel(x, y, color + sample_buffer[y * width + x] * ((sample_rate - count) / sample_rate));
          }
        }
      }
    }
  }


  void RasterizerImp::rasterize_textured_triangle(float x0, float y0, float u0, float v0,
    float x1, float y1, float u1, float v1,
    float x2, float y2, float u2, float v2,
    Texture& tex)
  {
    // TODO: Task 5: Fill in the SampleParams struct and pass it to the tex.sample function.
    // TODO: Task 6: Set the correct barycentric differentials in the SampleParams struct.
    // Hint: You can reuse code from rasterize_triangle/rasterize_interpolated_color_triangle
    // Create a 2D bouding box for the triangle
    float min_x = min(x0, min(x1, x2));
    float max_x = max(x0, max(x1, x2));
    float min_y = min(y0, min(y1, y2));
    float max_y = max(y0, max(y1, y2));
    if (sample_rate == 1){ // basic triangle rasterization
      // Traverse all the pixels in the bounding box. 
      // Then use Sample coordinates = the integer point + (0.5,0.5) to check whether it is in the triangle
      for (int x = min_x; x < max_x; x++){
        for (int y = min_y; y < max_y; y++){
          // Check whether the center of the pixel is in the triangle
          if (insideTriangle(x + 0.5, y + 0.5, x0, y0, x1, y1, x2, y2)){
            auto tup = computeBarycentric2D(x + 0.5, y + 0.5, x0, y0, x1, y1, x2, y2);
            float alpha;
            float beta;
            float gamma;
            tie(alpha, beta, gamma) = tup;
            Vector2D uv = {alpha * u0 + beta * u1 + gamma * u2, alpha * v0 + beta * v1 + gamma * v2};
            if (lsm == L_ZERO){
              if (psm == P_LINEAR){
                fill_pixel(x, y, tex.sample_bilinear(uv));
              }
              else if (psm == P_NEAREST){
                fill_pixel(x, y, tex.sample_nearest(uv));
              }
              else{
                printf("undefined psm!\n");
              }
            }
            else{
              tup = computeBarycentric2D(x + 1.5, y + 0.5, x0, y0, x1, y1, x2, y2);
              tie(alpha, beta, gamma) = tup;
              Vector2D uvx = {alpha * u0 + beta * u1 + gamma * u2, alpha * v0 + beta * v1 + gamma * v2};
              tup = computeBarycentric2D(x + 0.5, y + 1.5, x0, y0, x1, y1, x2, y2);
              tie(alpha, beta, gamma) = tup;
              Vector2D uvy = {alpha * u0 + beta * u1 + gamma * u2, alpha * v0 + beta * v1 + gamma * v2};
              SampleParams sp;
              sp.p_uv = uv;
              sp.p_dx_uv = uvx;
              sp.p_dy_uv = uvy;
              sp.psm = psm;
              sp.lsm = lsm;
              fill_pixel(x, y, tex.sample(sp));
            } 
          }
        }
      }
    }
    else{
      // sqrt(sample_rate) * sqrt(sample_rate) for each pixel
      vector<Vector2D> pos;
      int n = sqrt(sample_rate);
      for (int i = 0; i < n; i++){
        for (int j = 0; j < n; j++){
          pos.push_back({(i + 0.5) / n, (j + 0.5) / n});
        }
      }
      
      for (int x = min_x; x < max_x; x++){
        for (int y = min_y; y < max_y; y++){
          float count = 0;
          Color color = {0,0,0};
          for (int i = 0; i < sample_rate; i++){
            if (insideTriangle(x + pos[i][0], y + pos[i][1], x0, y0, x1, y1, x2, y2)){
              auto tup = computeBarycentric2D(x + pos[i][0], y + pos[i][1], x0, y0, x1, y1, x2, y2);
              float alpha;
              float beta;
              float gamma;
              tie(alpha, beta, gamma) = tup;
              Vector2D uv = {alpha * u0 + beta * u1 + gamma * u2, alpha * v0 + beta * v1 + gamma * v2};
              if (lsm == L_ZERO){
                if (psm == P_LINEAR){
                  color += (tex.sample_bilinear(uv)) * (1.0 / sample_rate);
                }
                else if (psm == P_NEAREST){
                  color += (tex.sample_nearest(uv)) * (1.0 / sample_rate);
                } 
                else{
                  printf("undefined psm!\n");
                }
              }
              else{
                tup = computeBarycentric2D(x + pos[i][0] + 1, y + pos[i][1], x0, y0, x1, y1, x2, y2);
                tie(alpha, beta, gamma) = tup;
                Vector2D uvx = {alpha * u0 + beta * u1 + gamma * u2, alpha * v0 + beta * v1 + gamma * v2};
                tup = computeBarycentric2D(x + pos[i][0], y + pos[i][1] + 1, x0, y0, x1, y1, x2, y2);
                tie(alpha, beta, gamma) = tup;
                Vector2D uvy = {alpha * u0 + beta * u1 + gamma * u2, alpha * v0 + beta * v1 + gamma * v2};
                SampleParams sp;
                sp.p_uv = uv;
                sp.p_dx_uv = uvx;
                sp.p_dy_uv = uvy;
                sp.psm = psm;
                sp.lsm = lsm;
                color += (tex.sample(sp) * (1.0 / sample_rate));
              }            
              count += 1;
            }
          }
          if (count != 0){
            fill_pixel(x, y, color + sample_buffer[y * width + x] * ((sample_rate - count) / sample_rate));
          }
        }
      }
    }
  }

  void RasterizerImp::set_sample_rate(unsigned int rate) {
    // TODO: Task 2: You may want to update this function for supersampling support

    this->sample_rate = rate;


    this->sample_buffer.resize(width * height, Color::White);
  }


  void RasterizerImp::set_framebuffer_target(unsigned char* rgb_framebuffer,
    size_t width, size_t height)
  {
    // TODO: Task 2: You may want to update this function for supersampling support

    this->width = width;
    this->height = height;
    this->rgb_framebuffer_target = rgb_framebuffer;


    this->sample_buffer.resize(width * height, Color::White);
  }


  void RasterizerImp::clear_buffers() {
    std::fill(rgb_framebuffer_target, rgb_framebuffer_target + 3 * width * height, 255);
    std::fill(sample_buffer.begin(), sample_buffer.end(), Color::White);
  }


  // This function is called at the end of rasterizing all elements of the
  // SVG file.  If you use a supersample buffer to rasterize SVG elements
  // for antialising, you could use this call to fill the target framebuffer
  // pixels from the supersample buffer data.
  //
  void RasterizerImp::resolve_to_framebuffer() {
    // TODO: Task 2: You will likely want to update this function for supersampling support


    for (int x = 0; x < width; ++x) {
      for (int y = 0; y < height; ++y) {
        Color col = sample_buffer[y * width + x];

        for (int k = 0; k < 3; ++k) {
          this->rgb_framebuffer_target[3 * (y * width + x) + k] = (&col.r)[k] * 255;
        }
      }
    }

  }

  Rasterizer::~Rasterizer() { }


}// CGL
