#ifndef D3_HPP
#define D3_HPP

#include <cstring>
#include <fstream>
#include <glad/glad.h>
#include <print>
#include <stdint.h>
#include <vector>
#define GMATH_IMPLEMENTATION
#include <gmath/gmath.hpp>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image/stb_image.h>
#include <windows.h>
#include <winuser.h>
#include <string>
#include <assert.h>
#include <chrono>
#include <thread>

namespace d3 {

typedef BOOL (WINAPI *wglSwapIntervalEXT_t)(int);
wglSwapIntervalEXT_t wglSwapIntervalEXT = (wglSwapIntervalEXT_t)wglGetProcAddress("wglSwapIntervalEXT");

GLuint vao;


constexpr const char* fragment_shader = 
R"(#version 330 core
in vec2 uv;
uniform sampler2D screenTex;
out vec4 fragColor;
void main() {
    vec2 uv_inv = {uv.x, -uv.y};
    fragColor = texture(screenTex, uv_inv);
}
)";

constexpr const char* vertex_shader = 
R"(#version 330 core
out vec2 uv;
void main() {
    vec2 pos;
    if (gl_VertexID == 0) pos = vec2(-1.0, -1.0);
    if (gl_VertexID == 1) pos = vec2( 3.0, -1.0);
    if (gl_VertexID == 2) pos = vec2(-1.0,  3.0);
    gl_Position = vec4(pos, 0.0, 1.0);
    uv = pos * 0.5 + 0.5;
}
)";





GLuint compile_shader(GLenum type, const char* src) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);



    GLint ok;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
    if (!ok) {
	char log[1024];
	glGetShaderInfoLog(shader, sizeof(log), nullptr, log);
	MessageBoxA(0, log, "Shader Compile Error", MB_OK);
	glDeleteShader(shader);
	return 0;
    }
    return shader;
}

GLuint create_program(const char* vs, const char* fs) {
    GLuint v = compile_shader(GL_VERTEX_SHADER, vs);
    GLuint f = compile_shader(GL_FRAGMENT_SHADER, fs);
    GLuint prog = glCreateProgram();
    glAttachShader(prog, v);
    glAttachShader(prog, f);
    glLinkProgram(prog);

    GLint ok;
    glGetProgramiv(prog, GL_LINK_STATUS, &ok);
    if (!ok) {
	char log[1024];
	glGetProgramInfoLog(prog, sizeof(log), nullptr, log);
	MessageBoxA(0, log, "Program Link Error", MB_OK);
	glDeleteProgram(prog);
	return 0;
    }

    glDeleteShader(v);
    glDeleteShader(f);
    return prog;
}




LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {

	case WM_DESTROY:
	    PostQuitMessage(0);
	    return 0;
	break;

	case WM_CLOSE:
	    DestroyWindow(hwnd);
	    return 0;
	break;

    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

enum Log_Level {
    LOG_ALL, LOG_INFO, LOG_DEBUG, LOG_ERROR, 
};

struct RectangleI {
    int x;
    int y;
    int width;
    int height;
};

struct PointI {
    int x;
    int y;

    void clip(int width, int height) {
	if (x < 0) x = 0;
	if (x >= width) x = width - 1;
	if (y < 0) y = 0;
	if (y >= height) y = height - 1;
    }
};


struct Color {
    uint8_t r; 
    uint8_t g;
    uint8_t b;
    uint8_t a;
    int to_int() const {
	// yolo
	return *(int*)this;
    }
};

constexpr d3::Color BLACK = {0, 0, 0, 255};
constexpr d3::Color RED = {255, 0, 0, 255};
constexpr d3::Color GREEN = {0, 255, 0, 255};
constexpr d3::Color BLUE = {0, 0, 255, 255};
constexpr d3::Color WHITE = {255, 255, 255, 255};
constexpr d3::Color PURPLE = {128, 0, 128, 255};
constexpr d3::Color GRAY = {0x16, 0x16, 0x16, 255};

    struct UV {
	float u;
	float v;
    };

    static constexpr size_t cube_uvs_size = 4;
    constexpr UV cube_uvs[cube_uvs_size] = {
	{0.f, 0.f},
	{1.f, 0.f},  
	{0.f, 1.f}, 
	{1.f, 1.f}
    };


    //struct Vertex3 {
    //    gmath::Vec3.
    //    std::string to_str() {
    //        return std::string(. ") +.to_str() + std::string(", uv: ");// + std::to_string(u) + ", " + std::to_string(v) + ", tex_id: " + std::to_string(tex_id);
    //    }
    //};


    struct IndexRecord {
	size_t v_index;
	size_t uv_index;
	size_t n_index;
	std::string to_str() const {
	    return "v_index = " + std::to_string(v_index) + ", uv_index = " + std::to_string(uv_index) + ", n_index = " + std::to_string(n_index);
	}
    };

    struct Face {
	IndexRecord vs[3];
	int tex_index;
	std::string to_str() const {
	    return "Face indeces:\n" + vs[0].to_str() + "\n" + vs[1].to_str() + "\n" + vs[2].to_str();
	}
    };

    struct Transform {
	gmath::Vec3 position;
	gmath::Vec3 angles;

	void move_dir(gmath::Vec3 dir, float speed) {
	    using namespace gmath;
	    if (speed == 0.f || dir.length() == 0.f) return;

	    dir.multiply(gmath::Mat4::rotation_from_angles(angles));
	    dir.normalize();
	    dir.multiply(speed);
	    position.add(dir);
	    position.multiply(gmath::Mat4::translation(dir));
	}
    };

    struct IndexRange {
	size_t start = 0;
	size_t count = 0;
    };

    struct Object {
	size_t id;
    };


    //basically an image
    struct Texture {
	uint32_t* pixels = nullptr;
	int width;
	int height;
	int comp_per_px = 4;

	std::string to_str() const {
	    return std::string("\npixels = ") + std::to_string((size_t)pixels) + "\nwidth = " + std::to_string(width)
		+ "\nheight = " + std::to_string(height) + "\ncomp_per_px = " + std::to_string(comp_per_px);
	}

	bool load_from_file(const char* filename) {
	    assert(pixels == nullptr);
	    int n; 
	    pixels = (uint32_t*)stbi_load(filename, &width, &height, &n, comp_per_px);
	    return (pixels != nullptr);
	}

	void from_color(int width, int height, int color) {
	    assert(pixels == nullptr);
	    pixels = new uint32_t[width * height];
	    std::memset(pixels, color, sizeof(uint32_t) * width * height);
	    this->width = width;
	    this->height = height;
	}

	uint32_t get_color(float u, float v) const {
	    assert(!std::isnan(u));
	    assert(!std::isnan(v));
	    u = gmath::clamp(u, 0.f, 1.f);
	    v = gmath::clamp(v, 0.f, 1.f);
	    assert(u >= 0.f && u <= 1.f);
	    assert(v >= 0.f && v <= 1.f);
	    if(!(u <= 1.f && u >= 0.f && v <= 1.f && v >= 0.f)) {
		std::println("uv wrong: {} {}", u, v);
		//assert(0 && "uv wrong in get color");
		return 0;
	    }
	    int x = std::round(u * (width  - 1 + 0.09f));
	    int y = std::round(v * (height - 1 + 0.09f));
	    size_t index = x + y * width;
	    if (x < 0.f || x >= width || y < 0.f || y >= height) {
		std::println("ERROR: index  = {} is wrong, u = {}, v = {}, x = {}, y = {}, width = {}, height = {}, width * height = {}", index, u, v, x, y, width, height, width * height);
		//assert(0 && "wrong index get color");
		return 0;
	    }
	    return pixels[index];
	}
    };

    struct Line_Data {
	int x1, y1, x2, y2, dx, sx, dy, sy, err, err2;//, pixel_x, pixel_y; 
	//float xf, yf;

	bool done = false;
	bool went_down = false;

	void set_initial(int x1, int y1, int x2, int y2) {
	    this->x1 = x1;
	    this->y1 = y1;
	    this->x2 = x2;
	    this->y2 = y2;
	    done = false;

	    dx = std::abs(x2 - x1);
	    sx = (x1 < x2) ? 1 : -1;

	    dy = -std::abs(y2 - y1);
	    sy = (y1 < y2) ? 1 : -1;

	    err = dx + dy;

	    if (x1 == x2 && y1 == y2) {
		done = true;
	    }

	    //pixel_x = x1;
	    //pixel_y = y1;
	    //xf = x1;
	    //yf = y1;
	}

	float length() {
	    float x = (float)x2 - x1;
	    float y = (float)y2 - y1;
	    return sqrtf(x * x + y * y);
	}

	bool fully_on_screen(int width, int height) {
	    return x1 >= 0 && x1 < width && y1 >= 0 && y1 < height && x2 >= 0 && x2 < width && y2 >= 0 && y2 < height;
	}
    };


    template <typename T>
    void sort_y(T a, T b, T c, int indeces[3]) {
	if (b.y < a.y && b.y < c.y) {
	    indeces[0] = 1;
	    indeces[1] = 0;
	    if (a.y > c.y) {
		indeces[1] = 2;
		indeces[2] = 0;
	    }
	}
	else if (c.y < a.y && c.y < b.y) {
	    indeces[0] = 2;
	    indeces[2] = 0;
	    if (b.y > a.y) {
		indeces[1] = 0;
		indeces[2] = 1;
	    }
	}
	else {
	    if (b.y > c.y) {
		indeces[1] = 2;
		indeces[2] = 1;
	    }
	}
    }
    Color lerp_color(Color start, Color end, float t) {
	if (t < 0.f) t = 0.f;
	else if (t > 1.f) t = 1.f;

	float inv_t = 1.f - t;
	// TODO: blend alpha too?
	Color res = {
			(uint8_t)(start.r * inv_t + end.r * t), 
			(uint8_t)(start.g * inv_t + end.g * t),
			(uint8_t)(start.b * inv_t + end.b * t),
			255
	};
	return res;
    }

    struct Timer {
	
	std::chrono::time_point<std::chrono::steady_clock> begin; 
	std::chrono::milliseconds elapsed_total;

	Timer() {
	    timeBeginPeriod(1);
	}
	~Timer() {
	    timeEndPeriod(1);
	}

	void start() {
	    begin = std::chrono::steady_clock::now();
	}

	int get_delta_mills() {
	    auto end = std::chrono::steady_clock::now();
	    std::chrono::milliseconds elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin);
	    elapsed_total += elapsed;
	    return elapsed.count();
	}

	void busy_wait(int mills) {
	    std::this_thread::sleep_for(std::chrono::milliseconds(mills));
	}

    };

    static inline constexpr bool is_digit(char c) {
	return (c >= '0' && c <= '9');
    }


    struct Renderer {

	GLuint gl_tex;
	GLuint program;
	HGLRC gl_ctx;

	std::vector<gmath::Vec4> vertices_world;
	std::vector<gmath::Vec4> vertices_viewport;
	std::vector<UV> uvs;
	std::vector<gmath::Vec3> normals;
	std::vector<Texture> textures;
	std::vector<Transform> transforms;
	std::vector<Object> objects;
	std::vector<IndexRange> ranges;
	std::vector<Face> faces;

	Texture tex;

	std::vector<float> z_buffer;

	float far_clip = 10.f;
	float near_clip = .1f;
	float fov = gmath::PI / 2.f;

	Object camera = {0};

	Renderer () {
	    Transform cam_transform = {{0, 0, -1}, {0}};
	    camera.id = push_object(cam_transform);

	    assert(camera.id == 0);
	}

	~Renderer() {
	    if (tex.pixels) {
		delete[] tex.pixels;
		tex.pixels = nullptr;
	    }
	}


	bool loadOBJ(const char* filepath, size_t& obj_id, Transform t = {0}, int tex_id = -1) {
	    std::ifstream file(filepath);
	    if (file.bad()) return false;

	    // OBJ indeces start with 1, so subtracting 1 should always fix that
	    size_t v_index_start = this->vertices_world.size() - 1;
	    size_t uv_index_start = this->uvs.size() - 1;
	    size_t n_index_start = this->normals.size() - 1;
	    IndexRange range;
	    range.start = this->faces.size();

	    std::println("loading obj:\nv_i_start = {}, uv_i_start = {}, n_i_start = {}, range.start = {}", v_index_start, uv_index_start, n_index_start, range.start);

	    std::string line;
	    std::string start;
	    std::vector<gmath::Vec4> verts;
	    std::vector<Face> faces;
	    std::vector<UV> uvs;
	    std::vector<gmath::Vec3> normals;

	    Face face;
	    gmath::Vec4 vert;
	    gmath::Vec3 normal;
	    float u, v;

	    while(std::getline(file, line)) {
		std::istringstream iss(line);
		start = "";
		iss >> start;

		if (start.starts_with("vt")) {
		    iss >> u;		    
		    iss >> v;		    
		    uvs.emplace_back(UV{u,v}); 
		}
		else if (start.starts_with("vn")) {
		    std::println("parsing face: {}", line);
		    if (!(iss >> normal.x) ||
			!(iss >> normal.y) ||
			!(iss >> normal.z)) {
			std::println("error on line: {}", line);
			std::println("start = {}", start);
			std::println("normal = {}", normal.to_str());
			continue;

		    }
		    //normal.multiply(-1.f);
		    //normal.z *= -1.f;
		    //normal.y *= -1.f;
		    std::println("parsed normal = {}", normal.to_str());
		    normals.emplace_back(normal);

		}
		else if (start.starts_with("v")) {
		    if (!(iss >> vert.x) ||
			!(iss >> vert.y) ||
			!(iss >> vert.z)) {
			std::println("error on line: {}", line);
			std::println("start = {}", start);
			std::println("vertex = {}", vert.to_str());
			continue;

		    }
		    vert.w = 1.f;
		    verts.emplace_back(vert);
		}
		else if (start.starts_with("f")) {
		    //std::println("parsing face: {}", line);
		    int cur = 0;
		    char c = line[0];
		    // only works for f 1/2/3 etc, f v_index/uv_index/normal_index
		    // find first two numbers
		    for(int vertex = 0; vertex < 3; ++vertex) {
			iss >> face.vs[vertex].v_index;
			face.vs[vertex].v_index += v_index_start;
			iss >> c;
			assert(c == '/');
			iss >> face.vs[vertex].uv_index;
			face.vs[vertex].uv_index += uv_index_start;
			iss >> c;
			assert(c == '/');
			iss >> face.vs[vertex].n_index;
			face.vs[vertex].n_index += n_index_start;
		    }
		    face.tex_index = tex_id;
		    faces.emplace_back(face);
		}
	    }

	    std::println("LoadOBJ: after reading in file:\nvertices read = {}, uvs read = {}\nnormals read = {}, faces read = {}", verts.size(), uvs.size(), normals.size(), faces.size());

	    //obj_push_verts(obj_id, verts.data(), verts.size(), v_indeces.data(), v_indeces.size());
	    //obj_push_uvs(obj_id, uvs.data(), uvs.size(), uv_indeces.data());

	    push_vertices(verts.data(), verts.size());
	    push_uvs(uvs.data(), uvs.size());
	    push_normals(normals.data(), normals.size());
	    push_faces(faces.data(), faces.size());

	    range.count = faces.size();
	    std::println("teapot face range:\nstart = {}, count = {}", range.start, range.count);
	    push_object(t, range);

	    return true;
	}

	Transform get_cam_transform() {
	    return transforms[camera.id] ;
	}

	void set_cam_transform(Transform& t) {
	    transforms[camera.id] = t; 
	}


	size_t push_cube(float side = 1.f, Transform t = {0}, int tex_id = -1) {

	    size_t v_start = vertices_world.size();
	    size_t uv_start = uvs.size();
	    size_t n_start = normals.size();

	    constexpr size_t v_size = 8;
	    gmath::Vec4 vertices[v_size] = {0}; 
	    vertices[0] = {-side / 2.f,  side / 2.f, -side / 2.f, 1.f};//, .u = 0.f, .v = 0.f, .tex_id = 0};
	    vertices[1] = { side / 2.f,  side / 2.f, -side / 2.f, 1.f};//, .u = 1.f, .v = 0.f, .tex_id = 0};
	    vertices[2] = {-side / 2.f, -side / 2.f, -side / 2.f, 1.f};//, .u = 0.f, .v = 1.f, .tex_id = 0};
	    vertices[3] = { side / 2.f, -side / 2.f, -side / 2.f, 1.f};//, .u = 1.f, .v = 1.f, .tex_id = 0};
			        				
	    vertices[4] = {-side / 2.f,  side / 2.f,  side / 2.f, 1.f};//, .u = 0.f, .v = 0.f, .tex_id = 0};
	    vertices[5] = { side / 2.f,  side / 2.f,  side / 2.f, 1.f};//, .u = 1.f, .v = 0.f, .tex_id = 0};
	    vertices[6] = {-side / 2.f, -side / 2.f,  side / 2.f, 1.f};//, .u = 0.f, .v = 1.f, .tex_id = 0};
	    vertices[7] = { side / 2.f, -side / 2.f,  side / 2.f, 1.f};//, .u = 1.f, .v = 1.f, .tex_id = 0};

	    push_vertices(vertices, v_size);
	    push_uvs(cube_uvs, cube_uvs_size);

	    constexpr size_t n_count = 6;
	    constexpr gmath::Vec3 normals[n_count] = {
		{0, 0, -1}, {0, 0, 1},
		{1, 0, 0}, {-1, 0, 0},
		{0, 1, 0}, {0, -1, 0}
	    };

	    push_normals(normals, n_count);


	// v_is
	//    4, 0, 6,  6, 0, 2, 

	//    4, 5, 0,  0, 5, 1,

	//    2, 3, 6,  6, 3, 7,

	//    1, 5, 3,  3, 5, 7,

	//    6, 7, 4,  4, 7, 5,

	//    0, 1, 2,  2, 1, 3,


    //uvs:
    //0.f, 0.f,
    //1.f, 0.f,  
    //0.f, 1.f, 
    //1.f, 1.f
	    constexpr size_t face_count = 6 * 2;
	    Face faces[face_count] = {
		// front
		{{{0 + v_start, 0 + uv_start, 0 + n_start}, {1 + v_start, 1 + uv_start, 0 + n_start}, {2 + v_start, 2 + uv_start, 0 + n_start}}, tex_id},
		{{{2 + v_start, 2 + uv_start, 0 + n_start}, {1 + v_start, 1 + uv_start, 0 + n_start}, {3 + v_start, 3 + uv_start, 0 + n_start}}, tex_id},
		// back
		{{{6 + v_start, 3 + uv_start, 1 + n_start}, {7 + v_start, 2 + uv_start, 1 + n_start}, {4 + v_start, 1 + uv_start, 1 + n_start}}, tex_id},
		{{{4 + v_start, 1 + uv_start, 1 + n_start}, {7 + v_start, 2 + uv_start, 1 + n_start}, {5 + v_start, 0 + uv_start, 1 + n_start}}, tex_id},
		// left
		{{{4 + v_start, 0 + uv_start, 4 + n_start}, {0 + v_start, 1 + uv_start, 4 + n_start}, {6 + v_start, 2 + uv_start, 4 + n_start}}, tex_id},
		{{{6 + v_start, 2 + uv_start, 4 + n_start}, {0 + v_start, 1 + uv_start, 4 + n_start}, {2 + v_start, 3 + uv_start, 4 + n_start}}, tex_id},
		// right
		{{{1 + v_start, 0 + uv_start, 5 + n_start}, {5 + v_start, 1 + uv_start, 5 + n_start}, {3 + v_start, 2 + uv_start, 5 + n_start}}, tex_id},
		{{{3 + v_start, 2 + uv_start, 5 + n_start}, {5 + v_start, 1 + uv_start, 5 + n_start}, {7 + v_start, 3 + uv_start, 5 + n_start}}, tex_id},
		// top
		{{{4 + v_start, 0 + uv_start, 3 + n_start}, {5 + v_start, 1 + uv_start, 3 + n_start}, {0 + v_start, 2 + uv_start, 3 + n_start}}, tex_id},
		{{{0 + v_start, 2 + uv_start, 3 + n_start}, {5 + v_start, 1 + uv_start, 3 + n_start}, {1 + v_start, 3 + uv_start, 3 + n_start}}, tex_id},
		// bot
		{{{2 + v_start, 0 + uv_start, 2 + n_start}, {3 + v_start, 1 + uv_start, 2 + n_start}, {6 + v_start, 2 + uv_start, 2 + n_start}}, tex_id},
		{{{6 + v_start, 2 + uv_start, 2 + n_start}, {3 + v_start, 1 + uv_start, 2 + n_start}, {7 + v_start, 3 + uv_start, 2 + n_start}}, tex_id},
	    };
	    
	    IndexRange range;
	    range.start = this->faces.size();
	    range.count = face_count;

	    push_faces(faces, face_count);
	    

	    size_t id = push_object(t, range);

	    return id;

	}

	size_t push_object(Transform t = {0}, IndexRange range = {0}) {
	    assert(ranges.size() == objects.size());
	    assert(transforms.size() == objects.size());

	    size_t id = objects.size();

	    objects.push_back({id});
	    transforms.push_back(t);
	    ranges.push_back(range);

	    return id;
	}	    
	
	void push_vertices(const gmath::Vec4* verts, size_t count) {
	    assert(verts);
	    for (int i = 0; i < count; ++i) {
		vertices_world.push_back(verts[i]);
	    }
	}
	
	void push_uvs(const UV* uvs, size_t count) {
	    assert(uvs);
	    for (int i = 0; i < count; ++i) {
		this->uvs.push_back(uvs[i]);
	    }
	}

	void push_normals(const gmath::Vec3* normals, size_t count) {
	    assert(normals);
	    for (int i = 0; i < count; ++i) {
		this->normals.push_back(normals[i]);
	    }
	}

	void push_faces(const Face* faces, size_t count) {
	    assert(faces);
	    for (int i = 0; i < count; ++i) {
		this->faces.push_back(faces[i]);
	    }
	}

	void obj_set_transform(size_t obj_id, const Transform& t) {
	    assert(obj_id < objects.size());
	    assert(obj_id < transforms.size());
	    assert(objects.size() == transforms.size());

	    transforms[obj_id] = t; 

	}

	void init_texture() {
	    assert(tex.pixels && tex.width && tex.height);

	    glGenTextures(1, &gl_tex);
	    glBindTexture(GL_TEXTURE_2D, gl_tex);

	    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	    glTexImage2D(GL_TEXTURE_2D, 0,
			 GL_RGBA8, tex.width, tex.height, 0,
			 GL_RGBA, GL_UNSIGNED_BYTE,
			 tex.pixels);
	}

	void init_gl(HDC hdc) {

	    PIXELFORMATDESCRIPTOR pfd = {};
	    pfd.nSize = sizeof(pfd);
	    pfd.nVersion = 1;
	    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	    pfd.iPixelType = PFD_TYPE_RGBA;
	    pfd.cColorBits = 32;

	    int pf = ChoosePixelFormat(hdc, &pfd);
	    SetPixelFormat(hdc, pf, &pfd);

	    gl_ctx = wglCreateContext(hdc);

	    wglMakeCurrent(hdc, gl_ctx);

	    if(!gladLoadGL()) {
		std::println("{}", glGetError());	
	    }

	    glEnable(GL_TEXTURE_2D);

	    glViewport(0, 0, tex.width, tex.height); 

	    program = create_program(vertex_shader, fragment_shader);
	    glUseProgram(program);

	    GLint loc = glGetUniformLocation(program, "screenTex");
	    glUniform1i(loc, 0); // Texture Unit 0

	    glGenVertexArrays(1, &vao);
	    glBindVertexArray(vao);

	    init_texture();

	    // vsync an;
	    if (wglSwapIntervalEXT) wglSwapIntervalEXT(1);
	}

	void init_z() {
	    assert(tex.pixels);
	    z_buffer.clear();
	    z_buffer.reserve(tex.width * tex.height);
	    z_buffer.resize(tex.width * tex.height);
	    for (float& p: z_buffer) {
		p = far_clip * 2.f;
	    }
	}

	void reset_z() {
	    for (float& p: z_buffer) {
		p = far_clip * 2.f;
	    }
	}

	void draw_tex(HDC hdc) {
	    glClearColor(0,0,0,1);
	    glClear(GL_COLOR_BUFFER_BIT);

	    glBindTexture(GL_TEXTURE_2D, gl_tex);

	    glTexSubImage2D(GL_TEXTURE_2D, 0, 0,0, tex.width, tex.height,
			    GL_RGBA, GL_UNSIGNED_BYTE, tex.pixels);

	    glDrawArrays(GL_TRIANGLES, 0, 3);

	    SwapBuffers(hdc);
	}


	void transform_vertices() {
	    using namespace gmath;
	    if (vertices_viewport.capacity() < vertices_world.size()) {
		std::println("viewport verts reserve, old size = {}, new size = {}", vertices_viewport.size(), vertices_world.size());
		vertices_viewport.reserve(vertices_world.size());
	    }
	    assert(vertices_viewport.capacity() >= vertices_world.size());

	    if (vertices_viewport.size() < vertices_world.size())   vertices_viewport.resize(vertices_world.size());
	    assert(vertices_viewport.size() >= vertices_world.size());

	    Transform& camera_transform = transforms[camera.id];

	    camera_transform.angles.y *= -1.f;

	    Mat4 camera_model = Mat4::get_model(camera_transform.position, camera_transform.angles);
	    Mat4 view = Mat4::get_model(camera_transform.position * -1.f, camera_transform.angles * -1.f);

	    // skip cam_id = 0;
	    size_t obj_id = camera.id + 1;

	    for (int fi = 0; fi < faces.size(); ++fi) {
		if (fi >= (ranges[obj_id].start + ranges[obj_id].count) ) {
		    obj_id++;
		}

		assert(obj_id < objects.size());
		assert(obj_id < transforms.size());
		    
		const Object& obj = objects[obj_id];
		const Transform& obj_transform = transforms[obj_id];	
		Mat4 model = Mat4::get_model(obj_transform.position, obj_transform.angles);
		Mat4 projection = Mat4::projection((float)tex.width / tex.height, fov, near_clip, far_clip);
		Mat4 mvp = projection * view * model;

		const Face& face = faces[fi];

		vertices_viewport[face.vs[0].v_index] = vertices_world[face.vs[0].v_index];
		vertices_viewport[face.vs[1].v_index] = vertices_world[face.vs[1].v_index];
		vertices_viewport[face.vs[2].v_index] = vertices_world[face.vs[2].v_index];
		Vec4& a = vertices_viewport[face.vs[0].v_index];
		Vec4& b = vertices_viewport[face.vs[1].v_index];
		Vec4& c = vertices_viewport[face.vs[2].v_index];

		a.multiply(mvp);
		b.multiply(mvp);
		c.multiply(mvp);
		//a = a.project(tex.width, tex.height, fov);
		//b = b.project(tex.width, tex.height, fov);
		//c = c.project(tex.width, tex.height, fov);
		
		//a.perspective_divide();
		//b.perspective_divide();
		//c.perspective_divide();

		from_clip_to_viewport(a, tex.width, tex.height);
		from_clip_to_viewport(b, tex.width, tex.height);
		from_clip_to_viewport(c, tex.width, tex.height);

	    }
	}

	void draw_triangles_wireframe(Color wire_col) {
	    using namespace gmath;

	    // camera always at id = 0, so other objects start at 1
	    size_t obj_id = 1;

	    for (int i = 0; i < faces.size(); i++) {

		if (i >= (ranges[obj_id].start + ranges[obj_id].count) ) {
		    obj_id++;
		}

		assert(obj_id < objects.size());
		assert(obj_id < transforms.size());

		const Face& face = faces[i];

		const Vec4& a = vertices_viewport[face.vs[0].v_index];
		const Vec4& b = vertices_viewport[face.vs[1].v_index];
		const Vec4& c = vertices_viewport[face.vs[2].v_index];

		// near clip
		if (a.z <= near_clip || b.z <= near_clip || c.z <= near_clip) {
		    continue;
		}
		// far clip
		if (a.w >= far_clip || b.w >= far_clip || c.w >= far_clip) {
		    continue;
		}

		draw_line_color(a.x, a.y, b.x, b.y, wire_col);
		draw_line_color(a.x, a.y, c.x, c.y, wire_col);
		draw_line_color(c.x, c.y, b.x, b.y, wire_col);
	    }
	}

	void draw_triangles() {
	    using namespace gmath;

	    reset_z();


	    // camera always at id = 0, so other objects start at 1
	    size_t obj_id = 1;

	    for (int i = 0; i < faces.size(); i++) {
		Color debug_col = PURPLE;

		if (i >= (ranges[obj_id].start + ranges[obj_id].count) ) {
		    obj_id++;
		}

		//std::println("drawing face = {} ", i);
		assert(obj_id < objects.size());
		assert(obj_id < transforms.size());
		    
		//std::println("mv: \n{}", mv.to_str());
		const Face& face = faces[i];

		int tex_id = face.tex_index;

		const Vec4 a = vertices_viewport[face.vs[0].v_index];
		const Vec4 b = vertices_viewport[face.vs[1].v_index];
		const Vec4 c = vertices_viewport[face.vs[2].v_index];
		//a.multiply(mvp);
		//b.multiply(mvp);
		//c.multiply(mvp);


		// backface culling    
		Vec3 ab = Vec3(a.x, a.y, a.z) - Vec3(b.x, b.y, b.z);
		Vec3 ac = Vec3(c.x, c.y, c.z) - Vec3(a.x, a.y, a.z);
		Vec3 normal = gmath::Vec3::cross(ab, ac);
		normal.normalize();

		//Vec3 normal2 = normals[face.vs[0].n_index];
		//Mat4 n_m = camera_model * Mat4::rotation(obj_transform.angles);
		//normal2.multiply(Mat4::rotation(camera_transform.angles));
		////normal2.project(tex.width, tex.height);
		//normal2.normalize();

		//std::println("normal1 = {}", normal.to_str());
		//std::println("normal2 = {}", normal2.to_str());

		float cam_dot = gmath::dot({0, 0, -1}, normal);
		//std::println("cam_dot = {}", cam_dot);

		if (cam_dot >= 0.f) {
		    continue;
	//	    tex_id = -1;
	//	    debug_col = RED;
		}

		// near clip
		if (a.z <= near_clip || b.z <= near_clip || c.z <= near_clip) {
		    continue;
		}
		// far clip
		if (a.z >= far_clip || b.z >= far_clip || c.z >= far_clip) {
		    continue;
		}
		
		if (tex_id < 0) {
		    fill_triangle_color({a.x, a.y, a.z}, {b.x, b.y, b.z}, {c.x, c.y, c.z}, debug_col);
		} 
		else {
		    fill_triangle_tex(face);
		}
	    }
	}

	void clear_pixels(Color c) {
	    clear_pixels(tex.pixels, tex.width, tex.height, c);
	}

	void clear_pixels(uint32_t* pixels, int width, int height, Color c) {
	    assert(pixels && "clear_pixels: pixels = nullptr");
	    int col = c.to_int();
	    for(int i = 0; i < width * height; ++i) {
	        pixels[i] = col;
	    }
	}

	void draw_rec(RectangleI rec, Color col) {
	    draw_rec(tex.pixels, tex.width, tex.height, rec, col);
	}
	void draw_rec(uint32_t* pixels, int width, int height, RectangleI rec, Color col) {
	    assert(pixels && "draw_rec: pixels = nullptr");
	    if (rec.x >= width) return;
	    if (rec.y >= height) return;

	    if (rec.x < 0) {
		rec.width += rec.x; 
		rec.x = 0;
	    }
	    if (rec.y < 0) {
		rec.height += rec.y; 
		rec.y = 0;
	    }
	    if (rec.x + rec.width >= width) rec.width = width - 1 - rec.x;
	    if (rec.y + rec.height >= height) rec.height = height - 1 - rec.y;

	    uint32_t c = col.to_int();

	    for(int y = rec.y; y < rec.y + rec.height; ++y) {
		for(int x = rec.x; x < rec.x + rec.width; ++x) {
		    uint64_t index = x + y * width;
		    if (index >= width * height) {
			std::println("draw_rec: index = {}, x = {}, y = {} ", index, x, y);
		    }
		    assert(index < width * height && "draw rec");
		    pixels[index] = c;
		}
	    }
	}


	void line_next_pixel(Line_Data& line, int width, int height) {

	    line.went_down = false;

	    if (line.x1 == line.x2 && line.y1 == line.y2) {
		line.done = true;
		return;
	    }

	    line.err2 = line.err << 1;

	    if (line.err2 > line.dy) {
		line.err += line.dy;
		line.x1 += line.sx;
		//line.xf += line.sx;
	    }

	    if (line.err2 < line.dx) {
		line.err += line.dx;
		line.y1 += line.sy;
		//line.yf += line.sy;
		line.went_down = true;
	    }

	    //line.pixel_x = line.x1;//line.xf;
	    //line.pixel_y = line.y1;//line.yf;
	}

	void draw_line_color(Line_Data& line, Color color) {
	    draw_line_color(tex.pixels, tex.width, tex.height, line, color);
	}

	void draw_line_color(int x1, int y1, int x2, int y2, Color color) {
	    Line_Data line;
	    line.set_initial(x1, y1, x2, y2);
	    draw_line_color(tex.pixels, tex.width, tex.height, line, color);
	}

	void draw_line_color(uint32_t* pixels, int width, int height, Line_Data& line, Color color) {
	    assert(pixels);

	    //std::println("line : x1 = {}, y1 = {}, x2 = {}, y2 = {}", line.x1, line.y1, line.x2, line.y2);
	    //std::println("width = {}, height = {}", width, height);

	    if (line.dx == 0) {
		draw_line_vert(line.x1, line.y1, line.y2, color);
	    }

	    if (line.dy == 0) {
		draw_line_hor_col(line.x1, line.y1, line.x2, color);
	    }

	    while (!line.done) {
		if (line.x1 >= 0 && line.x1 < width && line.y1 >= 0 && line.y1 < height) {
		    size_t index = line.x1 + line.y1 * width;
		    pixels[index] = color.to_int();
		    //std::println("accepted pixel = {} {}", line.pixel_x, line.pixel_y);
		}

		line_next_pixel(line, width, height);
	    }
	}

	void draw_line_vert(int x1, int y1, int y2, Color color) {
	    draw_line_vert(tex.pixels, tex.width, tex.height, x1, y1, y2, color.to_int());
	}
	// horizontal line
	void draw_line_vert(uint32_t* pixels, int width, int height, 
		int x1, int y1, int y2, uint32_t col) {

	    if (x1 < 0 || x1 >= width) return;
	    assert(pixels);

	    int dy = std::abs(y2 - y1);
	    int sy = (y1 < y2) ? 1 : -1;

	    for (int i = 0; i < dy; ++i) {

		if (y1 < height && y1 >= 0.f)
		    pixels[x1 + y1 * width] = col;

		y1 += sy;
	    }
	}

	void draw_line_vert(int x1, int y1, int x2, float u1, float v1, float u2, float v2, const Texture& tex) {
	    draw_line_vert(tex.pixels, tex.width, tex.height, x1, y1, x2, u1, v1, u2, v2, tex);
	}
	// horizontal line
	void draw_line_vert(uint32_t* pixels, int width, int height, 
		int x1, int y1, int y2, float u1, float v1, float u2, float v2,  const Texture& tex) {
	    if (y1 >= height || y1 < 0.f) return;
	    assert(pixels);

	    int dy = std::abs(y2 - y1);
	    int sy = (y1 < y2) ? 1 : -1;

	    float u_step = dy == 0 ? 0 : (u2 - u1) / dy;
	    float v_step = dy == 0 ? 0 : (v2 - v1) / dy;

	    for (int i = 0; i < dy; ++i) {

		if (y1 < width && y1 >= 0.f)
		    pixels[x1 + y1 * width] = tex.get_color(u1, v1);

		y1 += sy;

		u1 += u_step;
		v1 += v_step;
	    }
	}

	void draw_line_hor_tex(int x1, int y1, int x2, float z1, float z2, float u1, float v1, float u2, float v2, int tex_id, bool second = false) {
	    assert(tex_id < textures.size());
	    draw_line_hor_tex(tex.pixels, tex.width, tex.height, x1, y1, x2, z1, z2, u1, v1, u2, v2, textures[tex_id], second);
	}

	// horizontal line
	//
	void draw_line_hor_tex(uint32_t* pixels, int width, int height, 
		int x1, int y1, int x2, float z1, float z2, float u1, float v1, float u2, float v2, const Texture& tex, bool second = false) {

	    assert(pixels);
	    if (y1 >= height || y1 < 0.f) return;

	    int dx = std::abs(x2 - x1);
	    int sx = (x1 < x2) ? 1 : -1;

	    float z1_reci = 1.f / z1;
	    float z2_reci = 1.f / z2;
	    u1 *= z1_reci;
	    v1 *= z1_reci;
	    u2 *= z2_reci;
	    v2 *= z2_reci;

	    float u_step = dx == 0 ? 0 : (u2 - u1) / dx;
	    float v_step = dx == 0 ? 0 : (v2 - v1) / dx;
	    float z_reci_step = dx == 0 ? 0 : (z2_reci - z1_reci) / dx;



	    for (int i = 0; i < dx; ++i) {

		float z = 1.f / z1_reci;
		if (x1 < width && x1 >= 0.f) {
		    size_t index = x1 + y1 * width;
		    if (z < z_buffer[index]) {
			uint32_t col = (second ? RED.to_int() : tex.get_color(u1 * z, v1 * z));
			pixels[index] = col;
			z_buffer[index] = z;
		    }

		}
		x1 += sx;

		u1 += u_step;
		v1 += v_step ;
		z1_reci += z_reci_step;

	    }
	}

	void draw_line_hor_col_z(int x1, int y1, int x2, float z1, float z2, Color col) {
	    draw_line_hor_col_z(tex.pixels, tex.width, tex.height, x1, y1, x2, z1, z2, col.to_int());
	}

	void draw_line_hor_col_z(uint32_t* pixels, int width, int height, int x1, int y1, int x2, float z1 = 0, float z2 = 0, uint32_t col = PURPLE.to_int()) {

	    if (y1 >= height || y1 < 0.f) return;
	    assert(pixels);

	    int dx = std::abs(x2 - x1);
	    int sx = (x1 < x2) ? 1 : -1;


	    float z_step = dx == 0 ? 0 : (z2 - z1) / dx;

	    for (int i = 0; i < dx; ++i) {

		if (x1 < width && x1 >= 0.f) {
		    size_t index = x1 + y1 * width;
		    if (z1 < z_buffer[index]) {
		        pixels[index] = col;
		        z_buffer[index] = z1;
		    }
		}

		x1 += sx;
		z1 += z_step;
	    }
	}

	void draw_line_hor_col(int x1, int y1, int x2, Color col) {
	    draw_line_hor_col(tex.pixels, tex.width, tex.height, x1, y1, x2, col.to_int());
	}

	void draw_line_hor_col(uint32_t* pixels, int width, int height, int x1, int y1, int x2, uint32_t col) {

	    if (y1 >= height || y1 < 0.f) return;
	    assert(pixels);

	    int dx = std::abs(x2 - x1);
	    int sx = (x1 < x2) ? 1 : -1;


	    for (int i = 0; i < dx; ++i) {

		if (x1 < width && x1 >= 0.f) {
		    size_t index = x1 + y1 * width;
		    pixels[index] = col;
		}

		x1 += sx;
	    }
	}

	void draw_line_blend(int x1, int y1, int x2, int y2, Color start, Color end) {
	    draw_line_blend(tex.pixels, tex.width, tex.height, x1, y1, x2, y2, start, end);
	}

	void draw_line_blend(uint32_t* pixels, int width, int height, 
		int x1, int y1, int x2, int y2, Color start, Color end) {

	    assert(pixels);
	    

	    int dx = std::abs(x2 - x1);
	    int sx = (x1 < x2) ? 1 : -1;

	    int dy = -std::abs(y2 - y1);
	    int sy = (y1 < y2) ? 1 : -1;

	    int err = dx + dy;
	    int e2;
	    gmath::Vec3 v = {(float)x2 - x1, (float)y2 - y1, 0};
	    float length = v.length();
	    float t = 0.f;

	    while (true) {
		if ((uint32_t)(x1 + y1 * width) < width * height) {
		    int color = lerp_color(start, end, t).to_int();
		    pixels[x1 + y1 * width] = color;
		}
		
		if (x1 == x2 && y1 == y2)
		    break;

		e2 = err << 1;

		if (e2 > dy) {
		    err += dy;
		    x1 += sx;
		}

		if (e2 < dx) {
		    err += dx;
		    y1 += sy;
		}
		v.x = x2 - x1;
		v.y = y2 - y1;
		t = (length - v.length()) / length;
		//std::println("p1 : {} {} , p2 : {} {}, index = {}", x1, y1, x2, y2, index);
	    }
	}

	void fill_triangle_tex(const Face& face) {
	    assert(tex.pixels);

	    int indices_sorted[3] = {0, 1, 2};

	    const gmath::Vec4& a = vertices_viewport[face.vs[0].v_index];
	    const gmath::Vec4& b = vertices_viewport[face.vs[1].v_index];
	    const gmath::Vec4& c = vertices_viewport[face.vs[2].v_index];

	    sort_y(a, b, c, indices_sorted);
	    // set start point lowest vertex (cursed)
	    gmath::Vec4 p1 = vertices_viewport[face.vs[indices_sorted[0]].v_index];
	    gmath::Vec4 p2 = p1;

	    float p1_u = uvs[face.vs[indices_sorted[0]].uv_index].u;
	    float p1_v = uvs[face.vs[indices_sorted[0]].uv_index].v;
	    float target1_u = uvs[face.vs[indices_sorted[1]].uv_index].u;
	    float target1_v = uvs[face.vs[indices_sorted[1]].uv_index].v;

	    float p2_u = uvs[face.vs[indices_sorted[0]].uv_index].u;
	    float p2_v = uvs[face.vs[indices_sorted[0]].uv_index].v;
	    float target2_u = uvs[face.vs[indices_sorted[2]].uv_index].u;
	    float target2_v = uvs[face.vs[indices_sorted[2]].uv_index].v;

	    // choose target (end points of lines) based on lowest vertex by sorted index
	    gmath::Vec4 target1 = vertices_viewport[face.vs[indices_sorted[1]].v_index];
	    gmath::Vec4 target2 = vertices_viewport[face.vs[indices_sorted[2]].v_index];


	    Line_Data line1;
	    Line_Data line2;

	    line1.set_initial(p1.x, p1.y, target1.x, target1.y);
	    line2.set_initial(p2.x, p2.y, target2.x, target2.y);

	    float dist1 = line1.length();
	    float dist2 = line2.length();

	    // toggle to show the second triangle in a different way for debugging
	    bool second = false;

	    // horizontal line from p1 - target1, draw and skip
	    if (line1.dy == 0) {


	        draw_line_hor_tex(p1.x, p1.y, target1.x, p1.z, target1.z, p1_u, p1_v, target1_u, target1_v, face.tex_index);

	        p1 = vertices_viewport[face.vs[indices_sorted[1]].v_index];
	        target1 = vertices_viewport[face.vs[indices_sorted[2]].v_index];

		p1_u = uvs[face.vs[indices_sorted[1]].uv_index].u;
		p1_v = uvs[face.vs[indices_sorted[1]].uv_index].v;
		target1_u = uvs[face.vs[indices_sorted[2]].uv_index].u;
		target1_v = uvs[face.vs[indices_sorted[2]].uv_index].v;

		line1.set_initial(p1.x, p1.y, target1.x, target1.y);
		dist1 = line1.length();
	    }

	    while (!line2.done) {

		if (!line1.went_down) {
		    line_next_pixel(line1, tex.width, tex.height);

		    if (line1.done) {
			//std::println("BEFORE:\np1: {}, {}", line1.x1, line1.y1);
			//std::println("target1: {}", target1.to_str());
			p1 = vertices_viewport[face.vs[indices_sorted[1]].v_index];
			target1 = vertices_viewport[face.vs[indices_sorted[2]].v_index];
			//line1.set_initial(p1.x, p1.y, target1.x, target1.y);
			line1.set_initial(p1.x, p1.y, target1.x, target1.y);
			dist1 = line1.length();
			//std::println("AFTER:\ndist1 = {}, v.length = {}", dist1, v.length());
			//std::println("p1: {}, {}", line1.x1, line1.y1);
			//std::println("target1: {}", target1.to_str());
			second = false;
		    }
		}

		if (!line2.went_down) {
		    line_next_pixel(line2, tex.width, tex.height);
		}

		if (line1.went_down && line2.went_down) {
		    line1.went_down = false;
		    line2.went_down = false;



		    float z1_recip = 1.f / p1.z;
		    float z2_recip = 1.f / p2.z;

		    float zt1_recip = 1.f / target1.z;
		    float zt2_recip = 1.f / target2.z;

		    float t = 1.f - line1.length() / dist1;
		    float u1 = gmath::lerpf(p1_u * z1_recip, target1_u * zt1_recip, t);
		    float v1 = gmath::lerpf(p1_v * z1_recip, target1_v * zt1_recip, t);
		    //float u1 = gmath::lerpf(p1.u, target1.u, t);
		    //float v1 = gmath::lerpf(p1.v, target1.v, t);
		    float z1 = gmath::lerpf(p1.z, target1.z, t);

		    float zi1 = 1.f / gmath::lerpf(z1_recip, zt1_recip, t);


		    t = 1.f - line2.length() / dist2;
		    float u2 = gmath::lerpf(p2_u * z2_recip, target2_u * zt2_recip, t);
		    float v2 = gmath::lerpf(p2_v * z2_recip, target2_v * zt2_recip, t);
		    float z2 = gmath::lerpf(p2.z, target2.z, t);

		    float zi2 = 1.f / gmath::lerpf(z2_recip, zt2_recip, t);

		    draw_line_hor_tex(line1.x1, line1.y1, line2.x1, zi1, zi2, u1 * zi1, v1 * zi1, u2 * zi2, v2 * zi2, face.tex_index, second);
		}
	    }
	}

	void fill_triangle_color(gmath::Vec3 a, gmath::Vec3 b, gmath::Vec3 c, Color col) {
	    using namespace gmath;
	    assert(tex.pixels);

	    Line_Data line1;
	    Line_Data line2;

	    int indices_sorted[3] = {0, 1, 2};

	    sort_y(a, b, c, indices_sorted);
	    Vec3* vs[3] = {
			(indices_sorted[0] == 0 ? &a : (indices_sorted[0] == 1 ? &b : &c)),
			(indices_sorted[1] == 0 ? &a : (indices_sorted[1] == 1 ? &b : &c)),
			(indices_sorted[2] == 0 ? &a : (indices_sorted[2] == 1 ? &b : &c)),
	    };
	    // set start point lowest vertex (cursed)
	    Vec3 p1 = *vs[0];
	    Vec3 p2 = p1;

	    // choose target (end points of lines) based on lowest vertex by sorted index
	    Vec3 target1 = *vs[1];
	    Vec3 target2 = *vs[2];

	    line1.set_initial(p1.x, p1.y, target1.x, target1.y);
	    line2.set_initial(p2.x, p2.y, target2.x, target2.y);

	    float dist1 = line1.length();
	    float dist2 = line2.length();

	    // horizontal line from p1 - target1, draw and skip
	    if (line1.dy == 0) {
		draw_line_hor_col_z(p1.x, p1.y, target1.x, p1.z, target1.z, col);
		p1 = target1;
		target1 = *vs[2];
		line1.set_initial(p1.x, p1.y, target1.x, target1.y);
		dist1 = line1.length();
	    }

	    while (!line2.done) {

		if (!line1.went_down) {
		    line_next_pixel(line1, tex.width, tex.height);

		    if (line1.done) {
			p1 = target1;
			target1 = *vs[2];
			line1.set_initial(p1.x, p1.y, target1.x, target1.y);
			dist1 = line1.length();
		    }
		}

		if (!line2.went_down) {
		    line_next_pixel(line2, tex.width, tex.height);
		}

		if (line1.went_down && line2.went_down) {
		    line1.went_down = false;
		    line2.went_down = false;

		    float t = 1.f - line1.length() / dist1;
		    float z1 = gmath::lerpf(p1.z, target1.z, t);

		    t = 1.f - line2.length() / dist2;
		    float z2 = gmath::lerpf(p2.z, target2.z, t);

		    draw_line_hor_col_z(line1.x1, line1.y1, line2.x1, z1, z2, col);
		}
	    }
	}

    };

    struct Window {
	uint32_t width = 0;
	uint32_t height = 0;
	const char* name = nullptr;

	Renderer renderer;

	HWND hwnd;
	HDC hdc;
	MSG msg;

	bool is_open = false;
	Timer timer;
	int frametime = ((uint64_t)(1000.f / 60.f));


	Window(uint64_t width, uint64_t height, const char* name):
	width(width), height(height), name(name){

	    std::println("Renderer objects count in window constructor = {}", renderer.objects.size());
	    renderer.tex.from_color(width, height, BLACK.to_int());

	    HINSTANCE hInstance = GetModuleHandle(nullptr);
	    
	    WNDCLASS window_class = {};
	    window_class.lpfnWndProc = WindowProc;
	    window_class.hInstance = hInstance;
	    window_class.lpszClassName = "Window";
	    
	    RegisterClass(&window_class);

	    RECT r = {0, 0, (long)width, (long)height};
	    AdjustWindowRect(&r, WS_OVERLAPPEDWINDOW, FALSE);

	    hwnd = CreateWindowEx(
				    0, 
				    window_class.lpszClassName, 
				    name, 
				    WS_OVERLAPPEDWINDOW, 
				    0, 0, r.right - r.left, r.bottom - r.top, 
				    nullptr, nullptr, hInstance, nullptr);

	    if (hwnd == nullptr) {
			std::println("ERROR: could not create Window class : {}, name :  {}", window_class.lpszClassName, name);
			exit(0);
	    }

	    hdc = GetDC(hwnd);
	    if (hdc == nullptr) {
			std::println("ERROR: could not obtain device context");
			exit(0);
	    } 

	    renderer.init_gl(hdc);
	    

	    renderer.init_z();

	    show(SW_SHOW);

	    is_open = true;
	}

	~Window() {
	    CloseWindow(hwnd);
	    PostQuitMessage(0);
	}

	void show(int nCmdShow) {
	    ShowWindow(hwnd, nCmdShow);
	}

	void begin_frame() {

	    timer.start();
	    //renderer.clear_pixels(WHITE);

	    while (PeekMessage(&msg, nullptr, 0,0, PM_REMOVE)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	    }

	    if (GetAsyncKeyState(VK_ESCAPE) & 0x8000 || msg.message == WM_QUIT) {
		is_open = false;
	    }
	}

	void end_frame() {
	    draw();

	    int delta_mills = timer.get_delta_mills();
	    if (delta_mills < frametime) {
		timer.busy_wait(frametime - delta_mills);
	    }
	}

	void draw() {
	    renderer.draw_tex(hdc);
	}


	void set_target_fps(int fps) {
	    frametime = ((uint64_t)(1000.f / fps));
	}

    };


} // d3

#endif // D3_HPP
