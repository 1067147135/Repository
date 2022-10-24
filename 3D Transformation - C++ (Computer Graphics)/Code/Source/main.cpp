#include "Triangle.hpp"
#include "rasterizer.hpp"
#include <eigen3/Eigen/Eigen>
#include <iostream>
#include <fstream>
#include <string>
#include <opencv2/opencv.hpp>
#include <cmath>
// add some other header files you need

constexpr double MY_PI = 3.1415926;
char data[100]; // data buffer
float float3[3];
int int3[3];
// eye_pos: camera coordinates
Eigen::Matrix4f get_view_matrix(Eigen::Vector3f eye_pos)
{
    Eigen::Matrix4f view = Eigen::Matrix4f::Identity();

    // move the camera to the origin and move all the other objects at the same time, 
    // because the camera and object are moving in the same position, 
    // so the relative position is actually the same
    Eigen::Matrix4f translate;
    translate << 1, 0, 0, -eye_pos[0], 0, 1, 0, -eye_pos[1], 0, 0, 1,
        -eye_pos[2], 0, 0, 0, 1;

    view = translate * view;
    // std::clog << "view" << std::endl << view << std::endl;  // check data

    return view;
}


Eigen::Matrix4f get_model_matrix(float rotation_angle, Eigen::Vector3f T, Eigen::Vector3f S, Eigen::Vector3f P0, Eigen::Vector3f P1)
{
    // TODO: Implement this function

    //Step 1: Build the Translation Matrix M_trans:
    Eigen::Matrix4f M_trans;
    M_trans << 1, 0, 0, T[0], 
        0, 1, 0, T[1], 
        0, 0, 1, T[2], 
        0, 0, 0, 1;

    //Step 2: Build the Scale Matrix S_trans:
    Eigen::Matrix4f S_trans;
    S_trans << S[0], 0, 0, 0, 
        0, S[1], 0, 0, 
        0, 0, S[2], 0, 
        0, 0, 0, 1;
    
    //Step 3: Implement Rodrigues' Rotation Formular, rotation by angle theta around axix u, then get the model matrix
	// The axis u is determined by two points, u = P1-P0: Eigen::Vector3f P0 ,Eigen::Vector3f P1  
    // Create the model matrix for rotating the triangle around a given axis. // Hint: normalize axis first
    float radian = (rotation_angle * MY_PI) / 180;
    Eigen::Vector3f u = (P1 - P0).normalized();
    // std::cout << "axis: " << u << std::endl;
    Eigen::Matrix3f I = Eigen::Matrix3f::Identity();
    Eigen::Matrix3f N, R;
    N << 0, -u[2], u[1], 
        u[2], 0, -u[0], 
        -u[1], u[0], 0;
    R = cosf(radian) * I + (1 - cosf(radian)) * u * u.transpose() + sinf(radian) * N;
    Eigen::Matrix4f rotation;
    rotation << R(0, 0), R(0, 1), R(0, 2), 0, 
        R(1, 0), R(1, 1), R(1, 2), 0, 
        R(2, 0), R(2, 1), R(2, 2), 0, 
        0, 0, 0, 1;

    Eigen::Matrix4f model;
    model =  S_trans * rotation * M_trans;

	//Step 4: Use Eigen's "AngleAxisf" to verify your Rotation
	Eigen::AngleAxisf rotation_vector(radian, Vector3f(u[0], u[1], u[2]));  
	Eigen::Matrix3f rotation_matrix;
	rotation_matrix = rotation_vector.toRotationMatrix();
    std::cout << "Rodrigues: " << std::endl;
    std::cout << R << std::endl;
    std::cout << "AngleAxisf: " << std::endl;
    std::cout << rotation_matrix << std::endl;

	return model;
}

// eye_fov: field of view (unit: degree)
// aspect_ratio: width/hight ratio
// zNear: unit distance between the near plane of frustum and the camera (negative)
// zFar: unit distance between the far plane of frustum and the camera (negative)
Eigen::Matrix4f get_projection_matrix(float eye_fov, float aspect_ratio,
                                      float zNear, float zFar)
{

    // frustum -> cubic: convert prospective projection to orthogonal projection
    float half_angle = eye_fov / 2 / 180.0 * MY_PI; // angle / 2, degree -> radian
    float t = abs(zNear) * tanf(half_angle);        // top: top / |zNear| = tan(halfEyeAngelRadian)
    float r = t * aspect_ratio;                     // right: right / top = aspect_ration
    float l = (-1) * r;                             // left: assume symmetry i.e. left = -right
    float b = (-1) * t;                             // bottom: assume symmetry i.e. bottom = - top

    Eigen::Matrix4f persp2ortho;
    persp2ortho << zNear, 0, 0, 0,
        0, zNear, 0, 0, 
        0, 0, zNear + zFar, -(zNear * zFar),
        0, 0, 1, 0;

    // orthographic projection
    Eigen::Matrix4f scale;    // scale to a 2 * 2 * 2 cube
    scale << 2 / (r - l), 0, 0, 0,
        0, 2 / (t - b), 0, 0,
        0, 0, 2 / (zNear - zFar), 0,
        0, 0, 0, 1;

    Eigen::Matrix4f trans;    // transfer the center to the origin
    trans << 1, 0, 0, -((r + l) / 2),
        0, 1, 0, -((t + b) / 2),
        0, 0, 1, -((zNear + zFar) / 2),
        0, 0, 0, 1;

    Eigen::Matrix4f ortho;
    ortho = scale * trans;

    // squash all transformations
    Eigen::Matrix4f projection;
    projection = ortho * persp2ortho;
    // std::clog << "projection" << std::endl << projection << std::endl; //check

    return projection;
}

void parse(int n, char type){   // type: f = float, i = int
    bool flag = false;
    std::string snum = "";
    int count = 0;
    for (char c: data){
        if (c == '}'){
            flag == false;
        }
        if (flag){
            if ((c >= 48 && c <= 57) || c == '.' || c == '-'){
                snum += c;
            }
            else {
                if (!snum.empty()){
                    if (count >= n) std::cout << "Parse overflow!" << std::endl;
                    if (type == 'f'){
                        float3[count++] = std::stof(snum);
                    }
                    else if (type == 'i'){
                        int3[count++] = std::stoi(snum);
                    }
                    else{
                        std::cout << "Error in parse!" << std::endl;
                    }
                    snum = "";
                }
            }
        }
        if (c == '{'){
            flag = true;
        }
    }
}

int main(int argc, const char** argv)
{

    float angle = 0;
    bool command_line = false;
    std::string filename = "result.png";

    if (argc >= 3) {
        command_line = true;
        angle = std::stof(argv[2]); // -r by default
        if (argc == 4) {
            filename = std::string(argv[3]);
        }
        else
            return 0;
    }

    rst::rasterizer r(1024, 1024);

    std::ifstream para;
    para.open("../Source/parameters.txt");
    
    // define your eye position "eye_pos" to a proper position

    // Eigen::Vector3f eye_pos = {0, 0, 10};
    para >> data;
    parse(3, 'f');
    Eigen::Vector3f eye_pos = {float3[0], float3[1],float3[2]};
    
    

    // define a triangle named by "pos" and "ind"

    // std::vector<Eigen::Vector3f> pos{{2, 0, -1}, {0, 2, -1}, {-2, 0, -1}};
    // std::vector<Eigen::Vector3i> ind{{0, 1, 2}};
    std::vector<Eigen::Vector3f> pos;
    for (int i = 0; i < 3; i++){
        para >> data;
        parse(3, 'f');
        pos.push_back({float3[0], float3[1],float3[2]});
    }
    std::vector<Eigen::Vector3i> ind;
    para >> data;
    parse(3, 'i');
    ind.push_back({int3[0], int3[1],int3[2]});

    auto pos_id = r.load_positions(pos);
    auto ind_id = r.load_indices(ind);

    int key = 0;
    int frame_count = 0;

    // added parameters for get_projection_matrix(float eye_fov, float aspect_ratio,float zNear, float zFar)
    // float eye_fov = 45;
    // float aspect_ratio = 1;
    // float zNear = -0.1;
    // float zFar = -50;
    para >> data;
    parse(1, 'f');
    float eye_fov = float3[0];
    para >> data;
    parse(1, 'f');
    float aspect_ratio = float3[0];
    para >> data;
    parse(1, 'f');
    float zNear = float3[0];
    para >> data;
    parse(1, 'f');
    float zFar = float3[0];

    // Eigen::Vector3f T = {0, 0, 0};
    // Eigen::Vector3f S = {1, 1, 1};
    // Eigen::Vector3f P0 = {0, 0, 0};
    // Eigen::Vector3f P1 = {1, 2, 0};
    para >> data;
    parse(3, 'f');
    Eigen::Vector3f T = {float3[0], float3[1],float3[2]};
    para >> data;
    parse(3, 'f');
    Eigen::Vector3f S = {float3[0], float3[1],float3[2]}; 
    para >> data;
    parse(3, 'f');
    Eigen::Vector3f P0 = {float3[0], float3[1],float3[2]};
    para >> data;
    parse(3, 'f');
    Eigen::Vector3f P1 = {float3[0], float3[1],float3[2]};


    para.close();

    // Eigen::Vector3f axis(0, 0, 1);
    // Eigen::Vector3f axis(1, 0, 0);

    if (command_line) {
        r.clear(rst::Buffers::Color | rst::Buffers::Depth);

        r.set_model(get_model_matrix(angle, T, S, P0, P1));
        r.set_view(get_view_matrix(eye_pos));
        r.set_projection(get_projection_matrix(eye_fov, aspect_ratio, zNear, zFar));

        r.draw(pos_id, ind_id, rst::Primitive::Triangle);
        cv::Mat image(1024, 1024, CV_32FC3, r.frame_buffer().data());
        image.convertTo(image, CV_8UC3, 1.0f);

        cv::imwrite(filename, image);

        return 0;
    }

    while (key != 27) { // press "esc" to stop
        r.clear(rst::Buffers::Color | rst::Buffers::Depth);

        r.set_model(get_model_matrix(angle, T, S, P0, P1));
        r.set_view(get_view_matrix(eye_pos));
        r.set_projection(get_projection_matrix(eye_fov, aspect_ratio, zNear, zFar));

        r.draw(pos_id, ind_id, rst::Primitive::Triangle);

        cv::Mat image(1024, 1024, CV_32FC3, r.frame_buffer().data());
        image.convertTo(image, CV_8UC3, 1.0f);
        cv::imshow("image", image);
        key = cv::waitKey(10);

        std::cout << "frame count: " << frame_count++ << '\n';
        std::clog << "angle: " << angle << std::endl;
    

        if (key == 'a') {
            angle += 10;
        }
        else if (key == 'd') {
            angle -= 10;
        }
    }

    return 0;
}
