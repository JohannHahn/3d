#ifndef D3_HPP
#define D3_HPP

#include <cstring>
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

static constexpr size_t cube_uvs_size = 4 * 2;
float cube_uvs[cube_uvs_size] = {
    0.f, 0.f,
    0.f, 1.f, 
    1.f, 0.f,  
    1.f, 1.f
};

const char* fragment_shader = 
R"(#version 330 core
in vec2 uv;
uniform sampler2D screenTex;
out vec4 fragColor;
void main() {
    vec2 uv_inv = {uv.x, -uv.y};
    fragColor = texture(screenTex, uv_inv);
}
)";

const char* vertex_shader = 
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

    struct Vertex3C {
	float x;
	float y;
	float z;
	Color col;		
    };

    struct Vertex3 {
	gmath::Vec3 pos;
	std::string to_str() {
	    return std::string("pos: ") + pos.to_str() + std::string(", uv: ");// + std::to_string(u) + ", " + std::to_string(v) + ", tex_id: " + std::to_string(tex_id);
	}
    };

    struct Vertex3UV {
	gmath::Vec3 pos;
	float u;
	float v;
	std::string to_str() {
	    return std::string("pos: ") + pos.to_str() + std::string(", uv: ") + std::to_string(u) + ", " + std::to_string(v);
	}
    };

    struct TriangleUV {
	Vertex3UV a;
	Vertex3UV b;
	Vertex3UV c;
	size_t tex_id;
    };

    struct Transform {
	gmath::Vec3 position;
	gmath::Vec3 angles;
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
	    if (index >= width * height) {
		std::println("ERROR: index  = {} is wrong, u = {}, v = {}, x = {}, y = {}, width = {}, height = {}, width * height = {}", index, u, v, x, y, width, height, width * height);
		//assert(0 && "wrong index get color");
		return 0;
	    }
	    return pixels[index];
	}
    };

    struct Line_Data {
	int x1, y1, x2, y2, dx, sx, dy, sy, err, err2, pixel_x, pixel_y; 
	float xf, yf;

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

	    pixel_x = x1;
	    pixel_y = y1;
	    xf = x1;
	    yf = y1;
	}

	float length() {
	    float x = (float)x2 - xf;
	    float y = (float)y2 - yf;
	    return sqrtf(x * x + y * y);
	}
    };


    void sort_y(Vertex3C a, Vertex3C b, Vertex3C c, int indeces[3]) {
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
    template <typename T>
    void sort_y(T a, T b, T c, int indeces[3]) {
    //void sort_y(Vertex3 a, Vertex3 b, Vertex3 c, int indeces[3]) {
	if (b.pos.y < a.pos.y && b.pos.y < c.pos.y) {
	    indeces[0] = 1;
	    indeces[1] = 0;
	    if (a.pos.y > c.pos.y) {
		indeces[1] = 2;
		indeces[2] = 0;
	    }
	}
	else if (c.pos.y < a.pos.y && c.pos.y < b.pos.y) {
	    indeces[0] = 2;
	    indeces[2] = 0;
	    if (b.pos.y > a.pos.y) {
		indeces[1] = 0;
		indeces[2] = 1;
	    }
	}
	else {
	    if (b.pos.y > c.pos.y) {
		indeces[1] = 2;
		indeces[2] = 1;
	    }
	}
    }
    Color lerp(Color start, Color end, float t) {
	if (t < 0.f) t = 0.f;
	else if (t > 1.f) t = 1.f;

	float inv_t = 1.f - t;
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


    struct Renderer {

	GLuint gl_tex;
	GLuint program;
	HGLRC gl_ctx;

	std::vector<Vertex3> vertices;
	std::vector<size_t> vert_indeces;

	std::vector<float> uvs;
	std::vector<size_t> uv_indeces;

	std::vector<Texture> textures;
	std::vector<int> tex_ids;


	std::vector<Object> objects;
	std::vector<Transform> transforms;
	std::vector<IndexRange> ranges;

	Texture tex;

	std::vector<float> z_buffer;

	float far_clip = 100.f;

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



	size_t push_cube(float side = 1.f, Transform t = {0}, size_t tex_id = 0) {


	    static constexpr size_t v_size = 8;
	    Vertex3 vertices[v_size] = {0}; 
	    vertices[0] = {.pos = {-side / 2.f,  side / 2.f, -side / 2.f}};//, .u = 0.f, .v = 0.f, .tex_id = 0};
	    vertices[1] = {.pos = {-side / 2.f, -side / 2.f, -side / 2.f}};//, .u = 0.f, .v = 1.f, .tex_id = 0};
	    vertices[2] = {.pos = { side / 2.f,  side / 2.f, -side / 2.f}};//, .u = 1.f, .v = 0.f, .tex_id = 0};
	    vertices[3] = {.pos = { side / 2.f, -side / 2.f, -side / 2.f}};//, .u = 1.f, .v = 1.f, .tex_id = 0};
									   //
	    vertices[4] = {.pos = {-side / 2.f,  side / 2.f,  side / 2.f}};//, .u = 0.f, .v = 0.f, .tex_id = 0};
	    vertices[5] = {.pos = {-side / 2.f, -side / 2.f,  side / 2.f}};//, .u = 0.f, .v = 1.f, .tex_id = 0};
	    vertices[6] = {.pos = { side / 2.f,  side / 2.f,  side / 2.f}};//, .u = 1.f, .v = 0.f, .tex_id = 0};
	    vertices[7] = {.pos = { side / 2.f, -side / 2.f,  side / 2.f}};//, .u = 1.f, .v = 1.f, .tex_id = 0};

	    static constexpr size_t i_size = 6 * 6;
	    size_t indeces[i_size] = {

		4, 0, 6,  6, 0, 2, 

		4, 5, 0,  0, 5, 1,

		2, 3, 6,  6, 3, 7,

		1, 5, 3,  3, 5, 7,

		6, 7, 4,  4, 7, 5,

		0, 1, 2,  2, 1, 3,

	    };
	    

	    //push_object(obj, vertices, v_size, indeces, i_size, uvs, &tex_id, 1);
	    size_t id = push_object(t);
	    obj_push_verts(id, vertices, v_size, indeces, i_size);

	    size_t uvs_indeces[i_size] = { };
	    for (int i = 0; i < i_size - 5; i += 6) {
		uvs_indeces[i] = 0;
		uvs_indeces[i + 1] = 1;
		uvs_indeces[i + 2] = 2;

		uvs_indeces[i + 3] = 2;
		uvs_indeces[i + 4] = 1;
		uvs_indeces[i + 5] = 3;
	    } 

	    obj_push_uvs(id, cube_uvs, cube_uvs_size, uvs_indeces);
	    obj_push_tex_id(id, tex_id);

	    return id;

	}

	size_t push_object(Transform t) {
	    assert(ranges.size() == objects.size());
	    assert(transforms.size() == objects.size());
	    // might need change
	    assert(tex_ids.size() == objects.size());

	    size_t id = objects.size();
	    objects.push_back({id});
	    transforms.push_back(t);

	    ranges.push_back({});
	    tex_ids.push_back(-1);

	    return id;
	}	    

	void obj_push_verts(size_t id, Vertex3* verts, size_t v_count, size_t* indeces, size_t i_count) {
	    assert(id < objects.size());
	    assert(verts);
	    assert(indeces);

	    size_t i_start = vertices.size();
	    for (int i = 0; i < v_count; ++i) {
		vertices.push_back(verts[i]);
	    }

	    IndexRange& range = ranges[id];
	    range.start = vert_indeces.size();
	    range.count = i_count;

	    for (int i = 0; i < i_count; ++i) {
		vert_indeces.push_back(i_start + indeces[i]);
	    }

	    
	}
	
	void obj_push_tex_id(size_t obj_id, size_t tex_id) {
	    assert(obj_id < objects.size());
	    assert(tex_id < textures.size());
	    assert(tex_ids.size() == objects.size());

	    tex_ids[obj_id] = tex_id;
	}
	
	void obj_set_transform(size_t obj_id, const Transform& t) {
	    assert(obj_id < objects.size());
	    assert(obj_id < transforms.size());
	    assert(objects.size() == transforms.size());

	    transforms[obj_id] = t; 

	}

	void obj_push_uvs(size_t id, float* uvs, size_t uvs_count, size_t* uvs_indeces) {
	    assert(id < objects.size());
	    assert(uvs);
	    assert(uvs_indeces);

	    IndexRange& range = ranges[id];

	    // can only push uvs if vertices + indeces already pushed
	    assert(vert_indeces.size() == uv_indeces.size() + range.count);

	    // 2 floats per index
	    size_t i_start = this->uvs.size() / 2;
	    
	    for (int i = 0; i < uvs_count; ++i) {
		this->uvs.push_back(uvs[i]);
	    }
	    for (int i = 0; i < range.count; ++i) {
		uv_indeces.push_back(i_start + uvs_indeces[i]);
	    }

	    assert(vert_indeces.size() == uv_indeces.size());
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



	void draw_triangles() {
	    using namespace gmath;
	    assert(vert_indeces.size() % 3 == 0 && "Indeces count is not divisible by 3");	    
	    assert(vert_indeces.size() == uv_indeces.size());

	    reset_z();

	    const Transform& camera_transform = transforms[camera.id];
	    Mat4 camera_model = Mat4::get_model(camera_transform.position, camera_transform.angles);
	    Mat4 view = Mat4::get_model(camera_transform.position * -1, camera_transform.angles * -1);

	    // camera always at id = 0
	    size_t obj_id = 1;
	    //std::println("camera model : \n{}", camera_model.to_str());
	    //std::println("view : \n{}", view.to_str());

	    for (int i = 2; i < vert_indeces.size(); i += 3) {
		//if (i > ranges[obj_id].start && i >= ranges[obj_id].start + ranges[obj_id].count) {
		//    obj_id++;
		//}

		assert(obj_id < objects.size());
		assert(obj_id < transforms.size());
		assert(obj_id < tex_ids.size());
		    
		const Object& obj = objects[obj_id];
		const Transform& obj_transform = transforms[obj_id];	
		Mat4 model = Mat4::get_model(obj_transform.position, obj_transform.angles);
		//std::println("model : \n{}", model.to_str());
		Mat4 mv = view * model;
		//std::println("mv: \n{}", mv.to_str());
		size_t tex_id = 0;
		if (tex_ids[obj_id] > 0) tex_id = tex_ids[obj_id];


		TriangleUV tri = {

		    .a = { .pos = vertices[vert_indeces[i - 2]].pos, .u = uvs[uv_indeces[(i - 2)] * 2], .v = uvs[uv_indeces[(i - 2)] * 2 + 1]},
		    .b = { .pos = vertices[vert_indeces[i - 1]].pos, .u = uvs[uv_indeces[(i - 1)] * 2], .v = uvs[uv_indeces[(i - 1)] * 2 + 1]},
		    .c = { .pos = vertices[vert_indeces[i]].pos, .u = uvs[uv_indeces[i] * 2], .v = uvs[uv_indeces[i] * 2 + 1]},
		    .tex_id = tex_id
		};



		tri.a.pos.multiply(mv);
		tri.b.pos.multiply(mv);
		tri.c.pos.multiply(mv);


		tri.a.pos = tri.a.pos.project(tex.width, tex.height);
		tri.b.pos = tri.b.pos.project(tex.width, tex.height);
		tri.c.pos = tri.c.pos.project(tex.width, tex.height);

		fill_triangle_tex(tri);

	    }
	}

	void clear_pixels(Color c) {
	    clear_pixels(tex.pixels, tex.width, tex.height, c);
	}

	void clear_pixels(uint32_t* pixels, int width, int height, Color c) {
	    assert(pixels && "clear_pixels: pixels = nullptr");
	    std::memset(pixels, c.to_int(), width * height * sizeof(uint32_t));
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
		line.xf += line.sx;
	    }

	    if (line.err2 < line.dx) {
		line.err += line.dx;
		line.y1 += line.sy;
		line.yf += line.sy;
		line.went_down = true;
	    }

	    line.pixel_x = line.xf;
	    line.pixel_y = line.yf;
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
		draw_line_hor_col(line.x1, line.y1, line.y2, color);
	    }

	    while (!line.done) {
		size_t index = line.pixel_x + line.pixel_y * width;
		if (index < width * height) {
		    pixels[index] = color.to_int();
		    //std::println("accepted pixel = {} {}", line.pixel_x, line.pixel_y);
		}
		else {
		    std::println("rejected pixel = {} {}", line.pixel_x, line.pixel_y);
		    assert(0);
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

	    if (y1 >= height || y1 < 0.f) return;
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

	void draw_line_hor_tex(int x1, int y1, int x2, float z1, float z2, float u1, float v1, float u2, float v2, int tex_id) {
	    assert(tex_id < textures.size());
	    draw_line_hor_tex(tex.pixels, tex.width, tex.height, x1, y1, x2, z1, z2, u1, v1, u2, v2, textures[tex_id]);
	}

	// horizontal line
	//
	void draw_line_hor_tex(uint32_t* pixels, int width, int height, 
		int x1, int y1, int x2, float z1, float z2, float u1, float v1, float u2, float v2, const Texture& tex) {

	    assert(pixels);
	    if (y1 >= height || y1 < 0.f) return;

	    int dx = std::abs(x2 - x1);
	    int sx = (x1 < x2) ? 1 : -1;
	    float dz = std::abs(z2 - z1);
	    float z_step = dz == 0 ? 0 : (z2 - z1) / dx;

	    float u_step = dx == 0 ? 0 : (u2 - u1) / dx;
	    float v_step = dx == 0 ? 0 : (v2 - v1) / dx;


	    for (int i = 0; i < dx; ++i) {

		if (x1 < width && x1 >= 0.f && z1 > 0.f) {
		    size_t index = x1 + y1 * width;
		    assert(index < width * height && index < z_buffer.size());
		    if (z1 < z_buffer[index]) {
			pixels[index] = tex.get_color(u1, v1);
			z_buffer[index] = z1;
		    }

		}
		x1 += sx;
		u1 += u_step;
		v1 += v_step ;
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

		if (x1 < width && x1 >= 0.f)
		    pixels[x1 + y1 * width] = col;

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
		    int color = lerp(start, end, t).to_int();
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

	//void fill_triangle_tex(Vertex3& a, Vertex3& b, Vertex3& c, const gmath::Mat4 mv) {
	void fill_triangle_tex(TriangleUV& tri) {
	    assert(tex.pixels);

	     Vertex3UV& a = tri.a;
	     Vertex3UV& b = tri.b;
	     Vertex3UV& c = tri.c;

		//std::println("after projection:\na = {}", a.to_str());
		//std::println("b = {}", b.to_str());
		//std::println("c = {}", c.to_str());
	//void fill_triangle_tex(Vertex3& a, Vertex3& b, Vertex3& c) {

	    gmath::Vec3 ab = b.pos - a.pos;
	    gmath::Vec3 ac = c.pos - a.pos;
	    gmath::Vec3 normal = gmath::normalize(gmath::cross(ab, ac));
	    gmath::Vec3 camera_dir = {0.f, 0.f, 1.f};
	    

	    //std::println("normal = {}", normal.to_str());

	    //std::println("dot(camera, normal) = {}", gmath::dot(camera_dir, normal));

	    // backface culling
	    if (gmath::dot(camera_dir, normal) >= 0.f) return;


	    Line_Data line1;
	    Line_Data line2;

	    int indices_sorted[3] = {0, 1, 2};

	    sort_y(a, b, c, indices_sorted);
	    Vertex3UV* vs[3] = {
			(indices_sorted[0] == 0 ? &a : (indices_sorted[0] == 1 ? &b : &c)),
			(indices_sorted[1] == 0 ? &a : (indices_sorted[1] == 1 ? &b : &c)),
			(indices_sorted[2] == 0 ? &a : (indices_sorted[2] == 1 ? &b : &c)),
	    };
	    // set start point lowest vertex (cursed)
	    Vertex3UV p1 = *vs[0];
	    Vertex3UV p2 = p1;

	    // choose target (end points of lines) based on lowest vertex by sorted index
	    Vertex3UV target1 = *vs[1];
	    Vertex3UV target2 = *vs[2];

	    line1.set_initial(p1.pos.x, p1.pos.y, target1.pos.x, target1.pos.y);
	    line2.set_initial(p2.pos.x, p2.pos.y, target2.pos.x, target2.pos.y);

	    float dist1 = line1.length();
	    float dist2 = line2.length();

	    // horizontal line from p1 - target1, draw and skip
	    if (line1.dy == 0) {
	        draw_line_hor_tex(p1.pos.x, p1.pos.y, target1.pos.x, p1.pos.z, target1.pos.z, p1.u, p1.v, target1.u, target1.v, tri.tex_id);
	        p1 = *vs[1];
	        target1 = *vs[2];
	        //line1.set_initial(p1.pos.x, p1.pos.y, target1.pos.x, target1.pos.y);
		line1.set_initial(p1.pos.x, p1.pos.y, target1.pos.x, target1.pos.y);
		dist1 = line1.length();
	    }

	    while (!line2.done) {

		if (!line1.went_down) {
		    line_next_pixel(line1, tex.width, tex.height);

		    if (line1.done) {
			//std::println("BEFORE:\np1: {}, {}", line1.x1, line1.y1);
			//std::println("target1: {}", target1.to_str());
			p1 = *vs[1];
			target1 = *vs[2];
			//line1.set_initial(p1.pos.x, p1.pos.y, target1.pos.x, target1.pos.y);
			line1.set_initial(p1.pos.x, p1.pos.y, target1.pos.x, target1.pos.y);
			dist1 = line1.length();
			//std::println("AFTER:\ndist1 = {}, v.length = {}", dist1, v.length());
			//std::println("p1: {}, {}", line1.x1, line1.y1);
			//std::println("target1: {}", target1.to_str());
		    }
		}

		if (!line2.went_down) {
		    line_next_pixel(line2, tex.width, tex.height);
		}

		if (line1.went_down && line2.went_down) {
		    line1.went_down = false;
		    line2.went_down = false;

		    float t = 1.f - line1.length() / dist1;
		    float u1 = gmath::lerpf(p1.u, target1.u, t);
		    float v1 = gmath::lerpf(p1.v, target1.v, t);
		    float z1 = gmath::lerpf(p1.pos.z, target1.pos.z, t);

		    t = 1.f - line2.length() / dist2;
		    float u2 = gmath::lerpf(p2.u, target2.u, t);
		    float v2 = gmath::lerpf(p2.v, target2.v, t);
		    float z2 = gmath::lerpf(p2.pos.z, target2.pos.z, t);


		    draw_line_hor_tex(line1.pixel_x, line1.pixel_y, line2.pixel_x, z1, z2, u1, v1, u2, v2, tri.tex_id);
		}
	    }
	}

	void fill_triangle_color(Vertex3& a, Vertex3& b, Vertex3& c, Color col) {
	    assert(tex.pixels);

	    Line_Data line1;
	    Line_Data line2;

	    int indices_sorted[3] = {0, 1, 2};

	    sort_y(a, b, c, indices_sorted);
	    Vertex3* vs[3] = {
			(indices_sorted[0] == 0 ? &a : (indices_sorted[0] == 1 ? &b : &c)),
			(indices_sorted[1] == 0 ? &a : (indices_sorted[1] == 1 ? &b : &c)),
			(indices_sorted[2] == 0 ? &a : (indices_sorted[2] == 1 ? &b : &c)),
	    };
	    // set start point lowest vertex (cursed)
	    Vertex3 p1 = *vs[0];
	    Vertex3 p2 = p1;

	    // choose target (end points of lines) based on lowest vertex by sorted index
	    Vertex3 target1 = *vs[1];
	    Vertex3 target2 = *vs[2];

	    line1.set_initial(p1.pos.x, p1.pos.y, target1.pos.x, target1.pos.y);
	    line2.set_initial(p2.pos.x, p2.pos.y, target2.pos.x, target2.pos.y);

	    // horizontal line from p1 - target1, draw and skip
	    if (line1.dy == 0) {
		draw_line_hor_col(p1.pos.x, p1.pos.y, target1.pos.x, col);
		p1 = target1;
		target1 = *vs[2];
		line1.set_initial(p1.pos.x, p1.pos.y, target1.pos.x, target1.pos.y);
	    }

	    while (!line2.done) {

		if (!line1.went_down) {
		    line_next_pixel(line1, tex.width, tex.height);

		    if (line1.done) {
			p1 = target1;
			target1 = *vs[2];
			line1.set_initial(p1.pos.x, p1.pos.y, target1.pos.x, target1.pos.y);
			line1.done = false;
		    }
		}

		if (!line2.went_down) {
		    line_next_pixel(line2, tex.width, tex.height);
		}

		if (line1.went_down && line2.went_down) {
		    line1.went_down = false;
		    line2.went_down = false;

		    draw_line_hor_col(line1.pixel_x, line1.pixel_y, line2.pixel_x, col);
		}
	    }
	}


	void fill_triangle_color(Vertex3C a, Vertex3C b, Vertex3C c) {
	    fill_triangle_color(tex.pixels, tex.width, tex.height, a, b, c);
	}
	void fill_triangle_color(uint32_t* pixels, int width, int height, Vertex3C a, Vertex3C b, Vertex3C c) {
	    using namespace gmath;
	    assert(pixels);
	    int indices_sorted[3] = {0, 1, 2};

	    //sort_y(vs);
	    sort_y(a, b, c, indices_sorted);
	    // transform to 2d integer points
	    Vertex3C* vs[3] = {
			(indices_sorted[0] == 0 ? &a : (indices_sorted[0] == 1 ? &b : &c)),
			(indices_sorted[1] == 0 ? &a : (indices_sorted[1] == 1 ? &b : &c)),
			(indices_sorted[2] == 0 ? &a : (indices_sorted[2] == 1 ? &b : &c)),
	    };
	    // set start point lowest vertex (cursed)
	    Vertex3C l_1 = *vs[0];
	    Vertex3C l_2 = l_1;

	    // choose target (end points of lines) based on lowest vertex by sorted index
	    Vertex3C target1 = *vs[1];
	    Vertex3C target2 = *vs[2];

	    int dx1 = std::abs(vs[1]->x - vs[0]->x);
	    int sx1 = (vs[0]->x < vs[1]->x) ? 1 : -1;

	    int dy1 = -std::abs(vs[1]->y - vs[0]->y);
	    int sy1 = (vs[0]->y < vs[1]->y) ? 1 : -1;

	    int err1 = dx1 + dy1;
	    int e2_1;

	    int dx2 = std::abs(vs[2]->x - vs[0]->x);
	    int sx2 = (vs[0]->x < vs[2]->x) ? 1 : -1;

	    int dy2 = -std::abs(vs[2]->y - vs[0]->y);
	    int sy2 = (vs[0]->y < vs[2]->y) ? 1 : -1;

	    int err2 = dx2 + dy2;
	    int e2_2;
	    size_t index = 0;

	    bool stop1 = false;
	    bool stop2 = false;

	    bool end1 = false;
	    
	    float t1 = 0.f;
	    Vec3 v = {target1.x - l_1.x, target1.y - l_1.y, 0};
	    float l1 = v.length();
	    v = {target2.x - l_2.x, target2.y - l_2.y, 0};
	    float l2 = v.length();
	    float t2 = 0.f;
	    
	    Color col1, col2;

	    while (true) {
		v = {target1.x - l_1.x, target1.y - l_1.y, 0};
		t1 = (l1 - v.length()) / l1;
		col1 = lerp(l_1.col, target1.col, t1);
		v = {target2.x - l_2.x, target2.y - l_2.y, 0};
		t2 = (l2 - v.length()) / l2;
		col2 = lerp(l_2.col, target2.col, t2);

		index = l_1.y * width  + l_1.x;
		assert(index < width * height && "fill triag line 1");
		pixels[index] = col1.to_int();
		index = l_2.y * width  + l_2.x;
		assert(index < width * height && "fill triag line 2");
		pixels[index] = col2.to_int();

		if (l_1.x == target1.x && l_1.y == target1.y) {
		    end1 = true;
		}

		if (l_2.x == target2.x && l_2.y == target2.y) {
		    break;
		}

		if (end1) {
			
		    dx1 = std::abs(vs[2]->x - vs[1]->x);
		    sx1 = (vs[1]->x < vs[2]->x) ? 1 : -1;

		    dy1 = -std::abs(vs[2]->y - vs[1]->y);
		    sy1 = (vs[1]->y < vs[2]->y) ? 1 : -1;

		    err1 = dx1 + dy1;
		    end1 = false;
		    l_1.col = target1.col;
		    target1 = *vs[2];
		    t1 = 0.f;
		    v = {target1.x - l_1.x, target1.y - l_1.y, 0};
		    l1 = v.length();
		}

		if (!stop1) {
		    e2_1 = err1 << 1;

		    if (e2_1 > dy1) {
			err1 += dy1;
			l_1.x += sx1;
		    }

		    if (e2_1 < dx1) {
			err1 += dx1;
			l_1.y += sy1;
			stop1 = true;
		    }
		}
		if (!stop2) {
		    e2_2 = err2 << 1;

		    if (e2_2 > dy2) {
			err2 += dy2;
			l_2.x += sx2;
		    }

		    if (e2_2 < dx2) {
			err2 += dx2;
			l_2.y += sy2;
			stop2 = true;
		    }
		}
		
		if (stop1 && stop2) {
		    Renderer::draw_line_blend(pixels, width, height, 
			    l_1.x, l_1.y, l_2.x, l_2.y, col1, col2);
		    stop1 = false;
		    stop2 = false;
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

	bool in_window(PointI p) {
	    return p.x >= 0 && p.x < width && p.y >= 0 && p.y < height;
	}

    };


} // d3

#endif // D3_HPP
