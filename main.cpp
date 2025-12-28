#include <chrono>
#include <print>
#include <stdint.h>
#include <thread>
#include "d3.hpp"
#include <gmath/gmath.hpp>


const char* window_title = "Hello World";

constexpr uint64_t window_width = 1024;
constexpr uint64_t window_height = 1024;

float angle = 0.f;
float z = 0.f;






d3::RectangleI rec = {window_width - 200, 150, 100, 100};
constexpr d3::Color BLACK = {0, 0, 0, 255};
constexpr d3::Color RED = {255, 0, 0, 255};
constexpr d3::Color GREEN = {0, 255, 0, 255};
constexpr d3::Color BLUE = {0, 0, 255, 255};



bool index_test(float u, float v, int width, int height) {
    size_t index = u * (width - 1) + v * (height - 1) * width;
    float u_step = 1.f / width;
    float v_step = 1.f / height;
    int x_t = 0;
    int y_t = 0;
    float u1 = u;
    float v1 = v;
    std::println("Testing index: u = {}, v = {}, width = {}, height = {}, u_step = {}, v_step = {}", u, v, width, height, u_step, v_step);
    std::println("Testing index: index initial = {}", index);
    bool succ = true;
    for (int y = 0; y < height; ++y) {
	for (int x = 0; x < width; ++x) {
	    x_t = std::round(u1 * (width  - 1 + 0.9f));
	    y_t = std::round(v1 * (height - 1 + 0.9f));
	    if (x_t != x) {
		std::println("ERROR: wrong x calculated = {}, expected = {}, u = {}, u * width = {}", x_t, x, u1, u1 * (width - 1 + 0.09f));
		succ = false;
	    }
	    if (y_t != y) {
		std::println("ERROR: wrong y calculated = {}, expected = {}, v = {}, v * height = {}", y_t, y, v1, v1 * (height - 1 + 0.09f));
		succ = false;
	    }
	    index = x_t + y_t * width;
	    if (index > width * height) {
		std::println("ERROR: wrong index = {}, width = {}, height = {}, width * height = {}", index, width, height, width * height);
	    }
	    u1 += u_step;
	}
	v1 += v_step;
	u1 = u;
    }
    return succ;
}


int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    using namespace std::chrono_literals;

    assert(index_test(0, 0, 2048, 2048) && "index test failed");

    
    d3::Window window(window_width, window_height, window_title, hInstance);
    window.target_fps = 100;
    d3::Object object;
    window.objects.push_back(object);
    Vec3 a = {};
    float l = a.length();
    std::println("l = {}", l);

    //window.vertices.push_back({{50, window_height / 2.f, 0.f}, 0.f, 0.f, 0});
    //window.vertices.push_back({{window_width / 2.f, window.height - 50.f, 0,}, 0.f, 1.f, 0});
    //window.vertices.push_back({{window_width / 2.f, 50, 0}, 1.f, 0.f, 0});
    //window.vertices.push_back({{window_width - 50, window_height / 2.f, 0}, 1.f, 1.f, 0});

    window.vertices.push_back({{50, 50, 0.f}, 0.f, 0.f, 0});
    window.vertices.push_back({{200.f , window.height - 200.f, 0,}, 0.f, 1.f, 0});
    window.vertices.push_back({{window_width - 200, 50, 0}, 1.f, 0.f, 0});
    window.vertices.push_back({{window_width - 200, window_height - 200, 0}, 1.f, 1.f, 0});

    window.indeces.push_back(0);
    window.indeces.push_back(1);
    window.indeces.push_back(2);

    window.indeces.push_back(1);
    window.indeces.push_back(2);
    window.indeces.push_back(3);

    window.show(nCmdShow);
	MSG message = {0};


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
	    for (d3::Vertex3& v : window.vertices) {
		//v.pos.x--;
	    }
	    angle += 0.01f;
	}
	if (GetAsyncKeyState(VK_RIGHT) & 0x8000) {
	    rec.x += step;
	    for (d3::Vertex3& v : window.vertices) {
		//v.pos.x++;
	    }
	    angle -= 0.01f;
	}
	if (GetAsyncKeyState(VK_UP) & 0x8000) {
	    rec.y--;
	    for (d3::Vertex3& v : window.vertices) {
		//v.pos.y--;
	    }
	}
	if (GetAsyncKeyState(VK_DOWN) & 0x8000) {
	    rec.y++;
	    for (d3::Vertex3& v : window.vertices) {
		//v.pos.y++;
	    }
	}
	if (GetAsyncKeyState('W') & 0x8000) {
	}
	if (GetAsyncKeyState(VK_RIGHT) & 0x8000) {
	    rec.x += step;
	    for (d3::Vertex3& v : window.vertices) {
		//v.pos.x++;
	    }
	    angle -= 0.01f;
	}
	if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) {
	    PostMessage(window.hwnd, WM_CLOSE, 0, 0);
	}
	window.vertices[3].pos.x = rec.x;
	window.vertices[3].pos.y = rec.y;
	
	window.clear_pixels(BLACK);

	window.draw_rec(rec, {0xFF, 0x00, 0x22, 0xFF} );

	window.draw_triangles();
	//window.vertices[3].pos.x = rec.x;
	//window.vertices[3].pos.y = rec.y;
	window.render_screen();

	auto end = std::chrono::steady_clock::now();
	auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin);
	//std::println("frame time = {}", elapsed);
	std::chrono::milliseconds wait_time = frametime - elapsed;
	//std::println("wait time = {}", wait_time);
	if (wait_time > 0ms) std::this_thread::sleep_for(wait_time);

	elapsed_total = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - begin);
    }

    timeEndPeriod(1);

    return (int)message.wParam;
}
