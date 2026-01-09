#include <print>
#include <stdint.h>
#include "d3.hpp"
#include <gmath/gmath.hpp>


const char* window_title = "Hello World";

constexpr uint64_t window_width = 1024;
constexpr uint64_t window_height = 1024;

float angle_x = 0.f;
float angle_y = 0.f;
float angle_z = 0.f;
float z = 2.f;

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

POINT mouse_prev;
void controls(d3::Window& window) {

	int speed = 500;
	int step = 10;//speed * window.target_fps / 1000.f;
		      //
	float angle_step = 0.05f;
	if (GetAsyncKeyState(VK_SPACE) & 0x8000) {
	    angle_y += angle_step;
	}

	if (GetAsyncKeyState(VK_LEFT) & 0x8000) {
	    rec.x -= step;
	    window.renderer.camera.position.x -= 0.01f;
	}
	if (GetAsyncKeyState(VK_RIGHT) & 0x8000) {
	    rec.x += step;
	    window.renderer.camera.position.x += 0.01f;
	}
	if (GetAsyncKeyState(VK_UP) & 0x8000) {
	    rec.y -= step;
	    window.renderer.camera.position.z += 0.01f;
	}
	if (GetAsyncKeyState(VK_DOWN) & 0x8000) {
	    rec.y += step;
	    window.renderer.camera.position.z -= 0.01f;
	}

	if (GetAsyncKeyState('W') & 0x8000) {
	    z += 0.01f;
	}
	if (GetAsyncKeyState('S') & 0x8000) {
	    z -= 0.01f;
	}

	POINT mouse;
	GetCursorPos(&mouse);
	ScreenToClient(window.hwnd, &mouse);

	if ((unsigned)mouse.x < window.width &&
	    (unsigned)mouse.y < window.height)
	{
	    rec.x = mouse.x - rec.width / 2.f;
	    rec.y = mouse.y - rec.height / 2.f;
	}
	int dif_x = mouse_prev.x - mouse.x;
	int dif_y = mouse_prev.y - mouse.y;
	window.renderer.camera.angles.y -= 0.01f * dif_x;
	window.renderer.camera.angles.x -= 0.01f * dif_y;
	mouse_prev = mouse;
}


int main() {
    //assert(index_test(0, 0, 2048, 2048) && "index test failed");

    d3::Window window(window_width, window_height, window_title);
    d3::Renderer& renderer = window.renderer;
    window.set_target_fps(60);

    renderer.push_cube(1.f, -1, 0, 1);
    renderer.push_cube(1.f, 1, 0, 1);
    //renderer.push_cube(1, -0.5f);
    //window.vertices.push_back({{50, window_height / 2.f, 0.f}, 0.f, 0.f, 0});
    //window.vertices.push_back({{window_width / 2.f, window.height - 50.f, 0,}, 0.f, 1.f, 0});
    //window.vertices.push_back({{window_width / 2.f, 50, 0}, 1.f, 0.f, 0});
    //window.vertices.push_back({{window_width - 50, window_height / 2.f, 0}, 1.f, 1.f, 0});

    //renderer.vertices.push_back({{50, 50, 0.f}, 0.f, 0.f, 0});
    //renderer.vertices.push_back({{50.f , window.height - 50.f, 0,}, 0.f, 1.f, 0});
    //renderer.vertices.push_back({{window_width - 50.f, 50.f, 0}, 1.f, 0.f, 0});
    //renderer.vertices.push_back({{window_width - 50.f, window_height - 50.f, 0}, 1.f, 1.f, 0});

    //renderer.indeces.push_back(0);
    //renderer.indeces.push_back(1);
    //renderer.indeces.push_back(2);

    //renderer.indeces.push_back(1);
    //renderer.indeces.push_back(2);
    //renderer.indeces.push_back(3);

    while(window.is_open) {

	controls(window);

	window.begin_frame();

	renderer.clear_pixels(d3::BLACK);

	renderer.draw_rec(rec, {0xFF, 0x00, 0x22, 0xFF} );

	//renderer.draw_triangles();
	//renderer.draw_line_color(window.width / 2.f, window.height / 2.f, rec.x, rec.y, d3::GREEN.to_int());
	//gmath::Mat4 rotation_x = gmath::Mat4::rotation_x(angle_x);
	//gmath::Mat4 rotation_y = gmath::Mat4::rotation_y(angle_y);
	//gmath::Mat4 rotation_z = gmath::Mat4::rotation_z(angle_z);
	//gmath::Mat4 rotation = rotation_x * rotation_y * rotation_z;
	////rotation.multiply(gmath::Mat4::rotation_y(angle));
	////rotation.multiply(gmath::Mat4::rotation_x(angle));
	//float z_offset = 10;
	//d3::Vertex3 a = {{50, 50, z - z_offset}, 0.f, 0.f, 0};
	//d3::Vertex3 b = {{50.f , window.height - 50.f, z - z_offset,}, 0.f, 1.f, 0};
	//d3::Vertex3 c = {{window_width - 50.f, 50.f, z - z_offset}, 1.f, 0.f, 0};
	//d3::Vertex3 d = {{window_width - 50.f, window_height - 50.f, z - z_offset}, 1.f, 1.f, 0};

	//d3::Vertex3 e = {{50, 50, z + z_offset}, 0.f, 0.f, 0};
	//d3::Vertex3 f = {{50.f , window.height - 50.f, z + z_offset,}, 0.f, 1.f, 0};
	//d3::Vertex3 g = {{window_width - 50.f, 50.f, z + z_offset}, 1.f, 0.f, 0};
	//d3::Vertex3 h = {{window_width - 50.f, window_height - 50.f, z + z_offset}, 1.f, 1.f, 0};


	//gmath::Vec3 middle = {window.width / 2.f, window.height / 2.f, 0.f};
	//gmath::Mat4 trans = gmath::Mat4::translation({-100, -100, 0});


	//a.pos.multiply(trans);
	//b.pos.multiply(trans);
	//c.pos.multiply(trans);
	//d.pos.multiply(trans);
	//e.pos.multiply(trans);
	//f.pos.multiply(trans);
	//g.pos.multiply(trans);
	//h.pos.multiply(trans);

	//a.pos = a.pos - middle;
	//b.pos = b.pos - middle;
	//c.pos = c.pos - middle;
	//d.pos = d.pos - middle;

	//a.pos.project();
	//b.pos.project();
	//c.pos.project();
	//d.pos.project();

	//a.pos.multiply(rotation);
	//b.pos.multiply(rotation);
	//c.pos.multiply(rotation);
	//d.pos.multiply(rotation);


	//a.pos = a.pos + middle;
	//b.pos = b.pos + middle;
	//c.pos = c.pos + middle;
	//d.pos = d.pos + middle;

	//e.pos = e.pos - middle;
	//f.pos = f.pos - middle;
	//g.pos = g.pos - middle;
	//h.pos = h.pos - middle;

	//e.pos.project();
	//f.pos.project();
	//g.pos.project();
	//h.pos.project();

	//e.pos.multiply(rotation);
	//f.pos.multiply(rotation);
	//g.pos.multiply(rotation);
	//h.pos.multiply(rotation);


	//e.pos = e.pos + middle;
	//f.pos = f.pos + middle;
	//g.pos = g.pos + middle;
	//h.pos = h.pos + middle;

	//renderer.fill_triangle_color(e, f, g, d3::RED);
	//renderer.fill_triangle_color(g, f, h, d3::RED);

	//renderer.fill_triangle_tex(a, b, c);
	//renderer.fill_triangle_tex(c, b, d);
	renderer.draw_triangles();

	window.end_frame();

    }

    return 0;
}
