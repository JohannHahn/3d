#include <print>
#include <stdint.h>
#include "d3.hpp"
#include <gmath/gmath.hpp>


const char* window_title = "Hello World";

constexpr uint64_t window_width = 1024;
constexpr uint64_t window_height = 1024;

float angle = 0.f;
float z = 0.f;

d3::RectangleI rec = {window_width - 200, 150, 10, 10};

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

void controls(d3::Window& window) {

	int speed = 500;
	int step = 10;//speed * window.target_fps / 1000.f;
	if (GetAsyncKeyState(VK_LEFT) & 0x8000) {
	    rec.x -= step;
	    for (d3::Vertex3& v : window.renderer.vertices) {
		//v.pos.x--;
	    }
	    angle += 0.01f;
	}
	if (GetAsyncKeyState(VK_RIGHT) & 0x8000) {
	    rec.x += step;
	    for (d3::Vertex3& v : window.renderer.vertices) {
		//v.pos.x++;
	    }
	    angle -= 0.01f;
	}
	if (GetAsyncKeyState(VK_UP) & 0x8000) {
	    rec.y -= step;
	    for (d3::Vertex3& v : window.renderer.vertices) {
		//v.pos.y--;
	    }
	}
	if (GetAsyncKeyState(VK_DOWN) & 0x8000) {
	    rec.y += step;
	    for (d3::Vertex3& v : window.renderer.vertices) {
		//v.pos.y++;
	    }
	}

	if (GetAsyncKeyState('W') & 0x8000) {
	}

	if (GetAsyncKeyState(VK_RIGHT) & 0x8000) {
	    rec.x += step;
	    for (d3::Vertex3& v : window.renderer.vertices) {
		//v.pos.x++;
	    }
	    angle -= 0.01f;
	}
	POINT mouse;
	GetCursorPos(&mouse);
	ScreenToClient(window.hwnd, &mouse);

	if ((unsigned)mouse.x < window.width &&
	    (unsigned)mouse.y < window.height)
	{
	    rec.x = mouse.x;
	    rec.y = mouse.y;
	}
}


int main() {
//int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {

    assert(index_test(0, 0, 2048, 2048) && "index test failed");

    d3::Window window(window_width, window_height, window_title);
    d3::Renderer& renderer = window.renderer;
    window.set_target_fps(60);

    d3::Object object;
    renderer.objects.push_back(object);
    //window.vertices.push_back({{50, window_height / 2.f, 0.f}, 0.f, 0.f, 0});
    //window.vertices.push_back({{window_width / 2.f, window.height - 50.f, 0,}, 0.f, 1.f, 0});
    //window.vertices.push_back({{window_width / 2.f, 50, 0}, 1.f, 0.f, 0});
    //window.vertices.push_back({{window_width - 50, window_height / 2.f, 0}, 1.f, 1.f, 0});

    renderer.vertices.push_back({{50, 50, 0.f}, 0.f, 0.f, 0});
    renderer.vertices.push_back({{200.f , window.height - 200.f, 0,}, 0.f, 1.f, 0});
    renderer.vertices.push_back({{window_width - 200, 50, 0}, 1.f, 0.f, 0});
    renderer.vertices.push_back({{window_width - 200, window_height - 200, 0}, 1.f, 1.f, 0});

    renderer.indeces.push_back(0);
    renderer.indeces.push_back(1);
    renderer.indeces.push_back(2);

    renderer.indeces.push_back(1);
    renderer.indeces.push_back(2);
    renderer.indeces.push_back(3);

    while(window.is_open) {

	controls(window);

	window.begin_frame();

	renderer.clear_pixels(d3::BLACK);

	renderer.draw_rec(rec, {0xFF, 0x00, 0x22, 0xFF} );

	//window.draw_triangles();
	//renderer.draw_line_color(window.width / 2.f, window.height / 2.f, rec.x, rec.y, d3::GREEN.to_int());
	d3::Vertex3 a = {window.width / 2.f, 50, 0.f};
	d3::Vertex3 b = {50, window.height - 50.f, 0.f};
	d3::Vertex3 c = {(float)rec.x, (float)rec.y, 0.f};

	renderer.fill_triangle_color(a, b, c, d3::RED);

	window.end_frame();

    }

    return 0;
}
