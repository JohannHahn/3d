#ifndef D3_HPP
#define D3_HPP

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
#include <fstream>
#include <sstream>
#include <string>
#include <assert.h>

namespace d3 {

GLuint vao;
const char* window_class_name = "window_class";
const char* vertex_path = "src/vertex.glsl";
const char* fragment_path = "src/fragment.glsl";

typedef BOOL (WINAPI *wglSwapIntervalEXT_t)(int);
wglSwapIntervalEXT_t wglSwapIntervalEXT =
    (wglSwapIntervalEXT_t)wglGetProcAddress("wglSwapIntervalEXT");



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

    struct Vertex3C {
	float x;
	float y;
	float z;
	Color col;		
    };

    struct Vertex3 {
	Vec3 pos;
	float u;
	float v;
	uint32_t tex_id;
	std::string to_str() {
	    return std::string("pos: ") + pos.to_str() + std::string(", uv: ") + std::to_string(u) + ", " + std::to_string(v) + ", tex_id: " + std::to_string(tex_id);
	}
    };

    struct Object{
        std::vector<size_t> indeces;
        Vec4 position;
        Mat4 mat;
    };

    //basically an image
    struct Texture {
	uint32_t* data;
	int width;
	int height;
	int comp_per_px = 4;
	bool load_from_file(const char* filename) {
	    int n; 
	    data = (uint32_t*)stbi_load(filename, &width, &height, &n, comp_per_px);
	    return (data != nullptr);
	}
	uint32_t get_color(float u, float v) const {
	    if (std::isnan(u)) u = 0.f;
	    if (std::isnan(v)) v = 0.f;
	    u = clamp(u, 0.f, 1.f);
	    v = clamp(v, 0.f, 1.f);
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
	    return data[index];
	}
    };

    void sort_y(PointI points[3]) {
	for (int i = 1; i < 3; ++i) {
	    if (points[0].y > points[i].y) {
		PointI h = points[0];
		points[0] = points[i];
		points[i] = h;
		break;
	    }
	}
	if (points[1].y > points[2].y) {
		PointI h = points[1];
		points[1] = points[2];
		points[2] = h;
	}
    }

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
    void sort_y(Vertex3 a, Vertex3 b, Vertex3 c, int indeces[3]) {
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


    struct Window {
	uint32_t* pixels = nullptr;
	uint32_t width = 0;
	uint32_t height = 0;
	const char* name = nullptr;

	GLuint gl_tex;
	GLuint program;
	HGLRC gl_ctx;
	HWND hwnd;
	HDC hdc;

	uint64_t target_fps = 120;

	std::vector<Vertex3> vertices;
	std::vector<size_t> indeces;
	std::vector<Texture> textures;
	std::vector<Object> objects;

	Window(uint64_t width, uint64_t height, const char* name, HINSTANCE hInstance):
	width(width), height(height), name(name), pixels(new uint32_t[width * height]){
	    
	    WNDCLASS window_class = {};
	    window_class.lpfnWndProc = WindowProc;
	    window_class.hInstance = hInstance;
	    window_class.lpszClassName = window_class_name;
	    
	    RegisterClass(&window_class);

	    RECT r = {0, 0, (long)width, (long)height};
	    AdjustWindowRect(&r, WS_OVERLAPPEDWINDOW, FALSE);

	    hwnd = CreateWindowEx(
				    0, 
				    window_class_name, 
				    name, 
				    WS_OVERLAPPEDWINDOW, 
				    0, 0, r.right - r.left, r.bottom - r.top, 
				    nullptr, nullptr, hInstance, nullptr);

	    if (hwnd == nullptr) {
			std::println("ERROR: could not create Window class : {}, name :  {}", window_class_name, name);
			exit(0);
	    }

	    hdc = GetDC(hwnd);
	    if (hdc == nullptr) {
			std::println("ERROR: could not obtain device context");
			exit(0);
	    } 

	    init_gl();
	    
	    Texture t;
	    if(!t.load_from_file("res/johanndr.jpg")) {
			std::println("ERROR: could not load johanndr");
			exit(0);
	    }
	    textures.push_back(t);
	}

	~Window() {
	    CloseWindow(hwnd);
	    delete[] pixels;
	}

	void show(int nCmdShow) {
	    ShowWindow(hwnd, nCmdShow);
	}

	void clear_pixels(Color  ntc) {
	    //draw_rec({0, 0, width, height}, c);
	    std::memset(pixels, 0, width * height * sizeof(pixels[0]));
	}

	void init_texture() {
	    glGenTextures(1, &gl_tex);
	    glBindTexture(GL_TEXTURE_2D, gl_tex);

	    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	    glTexImage2D(GL_TEXTURE_2D, 0,
			 GL_RGBA8, width, height, 0,
			 GL_RGBA, GL_UNSIGNED_BYTE,
			 pixels);
	}

	void init_gl() {

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

	    glViewport(0, 0, width, height); 

	    std::ifstream s(vertex_path);
	    std::stringstream buffer;
	    buffer << s.rdbuf();
	    std::string vertex_source = buffer.str();
	    s.close();

	    s.open(fragment_path);
	    buffer.str(std::string());
	    buffer << s.rdbuf();
	    std::string fragment_source = buffer.str();
	    s.close();

	    program = create_program(vertex_source.c_str(), fragment_source.c_str());
	    glUseProgram(program);

	    GLint loc = glGetUniformLocation(program, "screenTex");
	    glUniform1i(loc, 0); // Texture Unit 0

	    glGenVertexArrays(1, &vao);
	    glBindVertexArray(vao);

	    init_texture();

	    // vsync an;
	    if (wglSwapIntervalEXT) wglSwapIntervalEXT(1);
	}
	

	void render_screen() {
	    glClearColor(0,0,0,1);
	    glClear(GL_COLOR_BUFFER_BIT);

	    glBindTexture(GL_TEXTURE_2D, gl_tex);

	    glTexSubImage2D(GL_TEXTURE_2D, 0, 0,0, width, height,
			    GL_RGBA, GL_UNSIGNED_BYTE, pixels);

	    glDrawArrays(GL_TRIANGLES, 0, 3);

	    SwapBuffers(hdc);
	}

	void draw_rec(RectangleI rec, Color col) {
	    assert(pixels);
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

	    uint32_t c = *(uint32_t*)(&col);

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

	void draw_line_color(int x1, int y1, int x2, int y2, uint32_t color) {
	    assert(pixels);

	    //if (x1 < 0) x1 = 0;
	    //if (x1 >= width) x1 = width - 1;
	    //if (y1 < 0) y1 = 0;
	    //if (y2 >= height) y1 = height - 1;

	    int dx = std::abs(x2 - x1);
	    int sx = (x1 < x2) ? 1 : -1;

	    int dy = -std::abs(y2 - y1);
	    int sy = (y1 < y2) ? 1 : -1;

	    int err = dx + dy;
	    int e2;

	    while (true) {
		if ((uint32_t)(x1 + y1 * width) < width * height)
		    pixels[x1 + y1 * width] = color;

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
	        //std::println("p1 : {} {} , p2 : {} {}, index = {}", x1, y1, x2, y2, index);
	    }
	}

	// horizontal line
	void draw_line_tex_h(int x1, int y1, int x2, float u1, float v1, float u2, float v2,  const Texture& tex) {
	    if (y1 >= height || y1 < 0.f) return;
	    assert(pixels);

	    int dx = std::abs(x2 - x1);
	    int sx = (x1 < x2) ? 1 : -1;

	    float u_step = dx == 0 ? 0 : (u2 - u1) / dx;
	    float v_step = dx == 0 ? 0 : (v2 - v1) / dx;

	    for (int i = 0; i < dx; ++i) {

		if (x1 < width && x1 >= 0.f)
		    pixels[x1 + y1 * width] = tex.get_color(u1, v1);
		x1 += sx;

		//float cur_dx = std::abs(x2 - x1);
		//float t = 1.f - cur_dx / dx;
		//std::println("draw line: t = {}", t);
		//u1 = lerpf(u1, u2, t);
		//v1 = lerpf(v1, v2, t);
	    	u1 += u_step;
	    	v1 += v_step;
	    }
	    //std::println("after draw_line_tex: u1 = {}, v1 = {}", u1, v1);
	    //exit(0);
	}

	void draw_line_blend(int x1, int y1, int x2, int y2, Color start, Color end) {
	    assert(pixels);

	    //if (x1 < 0) x1 = 0;
	    //if (x1 >= width) x1 = width - 1;
	    //if (y1 < 0) y1 = 0;
	    //if (y2 >= height) y1 = height - 1;
	    

	    int dx = std::abs(x2 - x1);
	    int sx = (x1 < x2) ? 1 : -1;

	    int dy = -std::abs(y2 - y1);
	    int sy = (y1 < y2) ? 1 : -1;

	    int err = dx + dy;
	    int e2;
	    Vec3 v = {(float)x2 - x1, (float)y2 - y1, 0};
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

	bool in_window(PointI p) {
	    return p.x >= 0 && p.x < width && p.y >= 0 && p.y < height;
	}

	void push_object(Object obj, const Vertex3 vertices[], size_t count) {
	    //for(int i = 0; i < count; ++i) {
	    //    vertices.push_back(vertices[i]);
	    //}     
	}

	void draw_triangles() {
	    assert(indeces.size() % 3 == 0 && "Indeces count is not divisible by 3");	    

	    Mat4 rotation = Mat4::rotation_z(0.f);

	    for (int i = 2; i < indeces.size(); i += 3) {
		Vertex3 a = vertices[indeces[i - 2]];
		Vertex3 b = vertices[indeces[i - 1]];
		Vertex3 c = vertices[indeces[i]];
		//std::println("before :\na = {}", a.to_str());
		//std::println("b = {}", b.to_str());
		//std::println("c = {}", c.to_str());

		a.pos.multiply(rotation);
		b.pos.multiply(rotation);
		c.pos.multiply(rotation);
		//if (z != 0.f) { 
		//    a.pos.x /= z;
		//    a.pos.y /= z;
		//    b.pos.x /= z;
		//    b.pos.x /= z;
		//    c.pos.y /= z;
		//    c.pos.y /= z;
		//}

		//std::println("after :\na = {}", a.to_str());
		//std::println("b = {}", b.to_str());
		//std::println("c = {}", c.to_str());
		//fill_triangle(vertices[indeces[i - 2]], vertices[indeces[i - 1]], vertices[indeces[i]]);
		fill_triangle(a, b, c);
	    }
	}

	void draw_object(const Object& obj) {
	    assert(obj.indeces.size() % 3 == 0 && "Object indeces count is not divisible by 3");	    
	    for (int i = 2; i < indeces.size(); i += 3) {
		fill_triangle(vertices[indeces[i - 2]], vertices[indeces[i - 1]], vertices[indeces[i]]);
	    }
	}

	void fill_triangle(Vertex3& a, Vertex3& b, Vertex3& c) {
	    int indices_sorted[3] = {0, 1, 2};
	    //std::println("fill trig: a = {}, b = {}, c = {}", a.to_str(), b.to_str(), c.to_str());


	    sort_y(a, b, c, indices_sorted);
	    Vertex3* vs[3] = {
			(indices_sorted[0] == 0 ? &a : (indices_sorted[0] == 1 ? &b : &c)),
			(indices_sorted[1] == 0 ? &a : (indices_sorted[1] == 1 ? &b : &c)),
			(indices_sorted[2] == 0 ? &a : (indices_sorted[2] == 1 ? &b : &c)),
	    };
	    // set start point lowest vertex (cursed)
	    Vertex3 l_1 = *vs[0];
	    Vertex3 l_2 = l_1;

	    // choose target (end points of lines) based on lowest vertex by sorted index
	    Vertex3 target1 = *vs[1];
	    Vertex3 target2 = *vs[2];
	    
	    //std::println("fill_trig: l_1 = {}, l_2 = {}, target1 = {}, target2 = {}", l_1.to_str(), l_2.to_str(), target1.to_str(), target2.to_str());

	    float dist1 = (target1.pos - l_1.pos).length();
	    float dist2 = (target2.pos - l_2.pos).length();

	    int dx1 = std::abs(target1.pos.x - l_1.pos.x);
	    int sx1 = (l_1.pos.x < target1.pos.x) ? 1 : -1;

	    int dy1 = -std::abs(target1.pos.y - l_1.pos.y);
	    int sy1 = (l_1.pos.y < target1.pos.y) ? 1 : -1;

	    int err1 = dx1 + dy1;
	    int e2_1;

	    int dx2 = std::abs(target2.pos.x - l_2.pos.x);
	    int sx2 = (l_2.pos.x < target2.pos.x) ? 1 : -1;

	    int dy2 = -std::abs(target2.pos.y - l_2.pos.y);
	    int sy2 = (l_2.pos.y < target2.pos.y) ? 1 : -1;

	    int err2 = dx2 + dy2;
	    int e2_2;
	    size_t index = 0;

	    bool stop1 = false;
	    bool stop2 = false;

	    bool end1 = false;
	    
	    float u_step1 = dx1 != 0 ? (target1.u - l_1.u) / (float)dx1 : 0; 
	    float v_step1 = dy1 != 0 ? -(target1.v - l_1.v) / (float)dy1 : 0; 
	    float u_step2 = dx2 != 0 ? (target2.u - l_2.u) / (float)dx2 : 0; 
	    float v_step2 = dy2 != 0 ? -(target2.v - l_2.v) / (float)dy2 : 0; 

	    //std::println("fill trig: l_1.u = {}, l_1.v = {}, l_2.u = {}, l_2.v = {}", l_1.u, l_1.v, l_2.u, l_2.v);
	    //std::println("fill trig: u_step1 = {}, v_step1 = {}, u_step2 = {}, v_step2 = {}", u_step1, v_step1, u_step2, v_step2);
	    //std::println("fill trig: dx1 = {}, dy1 = {}, dx2 = {}, dy2 = {}", dx1, dy1, dx2, dy2);
	    float cur_dist1 = width * height * 2.f;
	    float cur_dist2 = width * height * 2.f;

	    while (true) {

		//index = l_1.pos.y * width  + l_1.pos.x;
		//assert(index < width * height && "fill triag line 1");
		//pixels[index] = textures[l_1.tex_id].get_color(l_1.u, l_1.v);
		//index = l_2.pos.y * width  + l_2.pos.x;
		//assert(index < width * height && "fill triag line 2");
		//pixels[index] = textures[l_2.tex_id].get_color(l_2.u, l_2.v);

		//float last = cur_dist1;
		//cur_dist1 = (target1.pos - l_1.pos).length();
		////std::println("fill trig : cur_dist1 = {}, last_dist1 = {}", cur_dist1, last);
		//if (float_eq(cur_dist1, 0.f, 0.5f)) end1 = true;
		if (float_eq(l_1.pos.x, target1.pos.x, .1f) && float_eq(l_1.pos.y, target1.pos.y, .1f)) {
		    end1 = true;
		}

		float last = cur_dist2;
		//cur_dist2 = (target2.pos - l_2.pos).length();
		//std::println("fill trig : cur_dist2 = {}, last_dist2 = {}", cur_dist2, last);
		//std::println("fill trig : target2 = {}, l_2 = {}", target2.to_str(), l_2.to_str());
		//std::println("fill trig : target1 = {}, l_1 = {}", target1.to_str(), l_1.to_str());
		//std::println("fill trig : stop1 = {}, stop2 = {}, dy2 = {}, sy2 = {}", stop1, stop2, dy2, sy2);
		if (last < cur_dist2 || float_eq(cur_dist2, 0.f, 0.5f) ) break;
		if (float_eq(l_2.pos.x, target2.pos.x, .5f) && float_eq(l_2.pos.y, target2.pos.y, .5f)) {
		    break;
		}

		// reassign values when one line is finished before the other -> have to change target
	  	if (end1 && target1.pos.x != target2.pos.x && target1.pos.y != target2.pos.y) {

		    l_1 = target1;
		    target1 = *vs[2];
		    dx1 = std::abs(target1.pos.x - l_1.pos.x);
		    sx1 = (l_1.pos.x < target1.pos.x) ? 1 : -1;

		    dy1 = -std::abs(target1.pos.y - l_1.pos.y);
		    sy1 = (l_1.pos.y < target1.pos.y) ? 1 : -1;
		    u_step1 = dx1 != 0 ? (target1.u - l_1.u) / (float)dx1 : 0; 
		    v_step1 = dy1 != 0 ? -(target1.v - l_1.v) / (float)dy1 : 0; 
	  	    	
		    //std::println("trig_fill end1: old dist1", dist1);
		    dist1 = (target1.pos - l_1.pos).length();
		    //std::println("trig_fill end1: new dist1 = {}", dist1);
	  	    //dx1 = std::abs(vs[2]->pos.x - vs[1]->pos.x);
	  	    //sx1 = (vs[1]->pos.x < vs[2]->pos.x) ? 1 : -1;
          
		    //dy1 = -std::abs(vs[2]->pos.y - vs[1]->pos.y);
		    //sy1 = (vs[1]->pos.y < vs[2]->pos.y) ? 1 : -1;

		    err1 = dx1 + dy1;
		    end1 = false;
		    std::println("trig_fill end1: l_1 = {}", l_1.to_str());
		    stop2 = false;
		    
		}

		if (!stop1) {
		    e2_1 = err1 << 1;

		    if (e2_1 > dy1) {
			err1 += dy1;
			l_1.pos.x += sx1;
			//std::println("e2_1 > dy1: e2_1 = {}, dy1 = {}, \nl_1 = {}, err1 = {}", e2_1, dy1, l_1.to_str(), err1);
		    }

		    if (e2_1 < dx1) {
			err1 += dx1;
			l_1.pos.y += sy1;
			stop1 = true;
		    }
		}
		if (!stop2) {
		    e2_2 = err2 << 1;

		    if (e2_2 > dy2) {
			err2 += dy2;
			l_2.pos.x += sx2;
		    }

		    if (e2_2 < dx2) {
			err2 += dx2;
			l_2.pos.y += sy2;
			if (!end1)
			    stop2 = true;
		    }
		}

		
		
		if (stop1 && stop2) {
		    //std::println("fill_trig: draw: l_1 = {}, l_2 = {}", l_1.to_str(), l_2.to_str());
		    cur_dist1 = (target1.pos - l_1.pos).length();
		    float t = 1.f - cur_dist1 / dist1;
		    //std::println("fill trig: t1 = {}", t);
		    //std::println("fill trig: dist1 = {}", dist1);
		    //std::println("fill trig: cur_dist1 = {}", cur_dist1);
		    //std::println("fill trig: l_1.u = {}, target1.u = {}", l_1.u, target1.u);
		    float u1 = lerpf(l_1.u, target1.u, t);
		    //std::println("fill trig: u1 = {}", u1);
		    float v1 = lerpf(l_1.v, target1.v, t);

		    t = 1.f - (target2.pos - l_2.pos).length() / dist2;
		    //std::println("fill trig: t2 = {}", t);
		    float u2 = lerpf(l_2.u, target2.u, t);
		    float v2 = lerpf(l_2.v, target2.v, t);

		    draw_line_tex_h(l_1.pos.x, l_1.pos.y, l_2.pos.x, u1, v1, u2, v2, textures[l_1.tex_id]);
		    stop1 = false;
		    stop2 = false;
		}
	    }
	}
	void fill_triangle_color(Vertex3C a, Vertex3C b, Vertex3C c) {
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
		    draw_line_blend(l_1.x, l_1.y, l_2.x, l_2.y, col1, col2);
		    stop1 = false;
		    stop2 = false;
		}
	    }
	}

    };



} // d3

#endif // D3_HPP
