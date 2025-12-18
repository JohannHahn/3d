#include <glad/glad.h>
#include <chrono>
#include <print>
#include <windows.h>
#include <wingdi.h>
#include <winuser.h>
#include <inttypes.h>
//#include <gl/GL.h>
#include <thread>
#include <assert.h>
#include <cstring>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <math.h>

const char* window_class_name = "window_class";
const char* window_title = "Hello World";
const char* vertex_path = "src/vertex.glsl";
const char* fragment_path = "src/fragment.glsl";

constexpr uint64_t window_width = 900;
constexpr uint64_t window_height = 600;

GLuint vao;

typedef BOOL (WINAPI *wglSwapIntervalEXT_t)(int);
wglSwapIntervalEXT_t wglSwapIntervalEXT =
    (wglSwapIntervalEXT_t)wglGetProcAddress("wglSwapIntervalEXT");

std::vector<double> vertices;


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

namespace d3 {

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
    Color lerp(Color start, Color end, double t) {
	double inv_t = 1.f - t;
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

	Window(uint64_t width, uint64_t height, const char* name, const char* class_name, HINSTANCE hInstance):
	width(width), height(height), name(name), pixels(new uint32_t[width * height]){
	    
	    WNDCLASS window_class = {};
	    window_class.lpfnWndProc = WindowProc;
	    window_class.hInstance = hInstance;
	    window_class.lpszClassName = class_name;
	    
	    RegisterClass(&window_class);

	    RECT r = {0, 0, 900, 600};
	    AdjustWindowRect(&r, WS_OVERLAPPEDWINDOW, FALSE);

	    hwnd = CreateWindowEx(
				    0, 
				    class_name, 
				    name, 
				    WS_OVERLAPPEDWINDOW, 
				    0, 0, r.right - r.left, r.bottom - r.top, 
				    nullptr, nullptr, hInstance, nullptr);

	    if (hwnd == nullptr) {
		std::println("ERROR: could not create Window class : {}, name :  {}", class_name, name);
		exit(0);
	    }

	    hdc = GetDC(hwnd);
	    if (hdc == nullptr) {
		std::println("ERROR: could not obtain device context");
		exit(0);
	    } 

	    init_gl();
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

	void render() {
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
		    uint64_t index = x + y * window_width;
		    if (index >= width * height) {
			std::println("index = {}, x = {}, y = {} ", index, x, y);
		    }
		    assert(index < width * height && "draw rec");
		    pixels[index] = c;
		}
	    }
	}

	void draw_line(int x1, int y1, int x2, int y2, uint32_t color) {
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
	    float length = std::sqrtf(dx * dx + dy * dy);
	    float step = 1.f / length;
	    float t = 0.f;

	    while (true) {
		if ((uint32_t)(x1 + y1 * width) < width * height) {
		    float currentlength = std::sqrtf((float)x * dx + dy * dy);
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
		t += step;
		std::println("t = {}", t);
	        //std::println("p1 : {} {} , p2 : {} {}, index = {}", x1, y1, x2, y2, index);
	    }
	}

	bool in_window(PointI p) {
	    return p.x >= 0 && p.x < width && p.y >= 0 && p.y < height;
	}

	void fill_triangle(Vertex3C a, Vertex3C b, Vertex3C c) {
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

	    uint32_t color = 0xffffffff;

	    while (true) {
		index = l_1.y * width  + l_1.x;
		assert(index < width * height && "fill triag line 1");
		pixels[index] = l_1.col.to_int();
		index = l_2.y * width  + l_2.x;
		assert(index < width * height && "fill triag line 2");
		pixels[index] = l_2.col.to_int();

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
		    target1 = *vs[2];
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
		    draw_line_blend(l_1.x, l_1.y, l_2.x, l_2.y, l_1.col, l_2.col);
		    stop1 = false;
		    stop2 = false;
		}
	    }
	}


    };






} // d3



d3::RectangleI rec = {0, 0, 100, 100};
constexpr d3::Color BLACK = {0, 0, 0, 255};
constexpr d3::Color RED = {255, 0, 0, 255};
constexpr d3::Color GREEN = {0, 255, 0, 255};
constexpr d3::Color BLUE = {0, 0, 255, 255};




int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    using namespace std::chrono_literals;

    d3::Window window(window_width, window_height, window_title, window_class_name, hInstance);
    window.target_fps = 999;

    window.show(nCmdShow);
    MSG message;

    vertices.reserve(100);

    timeBeginPeriod(1);
    std::chrono::milliseconds frametime((uint64_t)(1000.f / window.target_fps));
    std::chrono::milliseconds elapsed_total;

    while (message.message != WM_QUIT) {

        auto begin = std::chrono::steady_clock::now();

        while (PeekMessage(&message, nullptr, 0,0, PM_REMOVE)) {
            TranslateMessage(&message);
            DispatchMessage(&message);
        }

	int speed = 500;
	int step = 1;//speed * window.target_fps / 1000.f;
	if (GetAsyncKeyState(VK_LEFT) & 0x8000) {
	    rec.x -= step;
	}
	if (GetAsyncKeyState(VK_RIGHT) & 0x8000) {
	    rec.x += step;
	}
	if (GetAsyncKeyState(VK_UP) & 0x8000) {
	    rec.y--;
	}
	if (GetAsyncKeyState(VK_DOWN) & 0x8000) {
	    rec.y++;
	}
	if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) {
	    PostMessage(window.hwnd, WM_CLOSE, 0, 0);
	}
	
	window.clear_pixels(BLACK);

	d3::Vertex3C a = {50, 50, 0, RED};
	d3::Vertex3C b = {50, window.height - 50.f, 0, GREEN};
	d3::Vertex3C c = {(float)rec.x, (float)rec.y, 0, BLUE};
	window.draw_rec(rec, {0xFF, 0x00, 0x22, 0xFF} );
	//window.draw_line(a.x, a.y, b.x, b.y, RED.to_int());
	//window.draw_line(a.x, a.y, c.x, c.y, RED.to_int());
	//window.draw_line(b.x, b.y, c.x, c.y, RED.to_int());
	window.fill_triangle(a, b, c);

	window.render();

	auto end = std::chrono::steady_clock::now();
	auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin);
	//std::println("frame time = {}", elapsed);
	std::chrono::milliseconds wait_time = frametime - elapsed;
	//std::println("wait time = {}", wait_time);
	if (wait_time > 0ms) std::this_thread::sleep_for(wait_time);

	elapsed_total = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - begin);

	//std::println("total time = {}", elapsed_total);

    }

    timeEndPeriod(1);

    return (int)message.wParam;
}
