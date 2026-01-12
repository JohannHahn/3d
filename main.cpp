#include <cstdio>
#include <print>
#include <stdint.h>
#include "d3.hpp"
#include <gmath/gmath.hpp>


const char* window_title = "Hello World";

constexpr uint64_t window_width = 1024;
constexpr uint64_t window_height = 1024;


d3::Transform cube_transform = {0, 0, 2};
d3::Transform camera_transform = {0, 0, -10};
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
		      //
	float step = 0.05f;


	if (GetAsyncKeyState(VK_LEFT) & 0x8000) {
	}
	if (GetAsyncKeyState(VK_RIGHT) & 0x8000) {
	}

	if (GetAsyncKeyState(VK_UP) & 0x8000) {
	    camera_transform.position.y += step;
	}
	if (GetAsyncKeyState(VK_DOWN) & 0x8000) {
	    camera_transform.position.y -= step;
	}

	if (GetAsyncKeyState('W') & 0x8000) {
	    camera_transform.position.z += step;
	}
	if (GetAsyncKeyState('S') & 0x8000) {
	    camera_transform.position.z -= step;
	}
	if (GetAsyncKeyState('A') & 0x8000) {
	    camera_transform.position.x -= step;
	}
	if (GetAsyncKeyState('D') & 0x8000) {
	    camera_transform.position.x += step;
	}

	if (GetAsyncKeyState(VK_SPACE) & 0x8000) {
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
	cube_transform.angles.y -= 0.01f * dif_x;
	cube_transform.angles.x -= 0.01f * dif_y;
	mouse_prev = mouse;
}


int main() {
    //assert(index_test(0, 0, 2048, 2048) && "index test failed");
    //

    //std::string line = "0/1/2 3/4/5 6/7/8";
    //std::istringstream iss(line);
    //int j = -1;


    //int cur = 0;
    //int count = 0;
    //char c;
    //// only works for f 1/2/3 etc, f v_index/uv_index/normal_index
    //// find first two numbers
    //while (cur < line.size()) {
    //    c = line[cur];
    //    if ( c >= '0' && c <= '9') {
    //        std::istringstream iss(line.substr(cur, line.size() - cur));
    //        iss >> j;
    //        std::println("v_i = {}", j);
    //        iss >> c;
    //        //std::println("c = {}", j);
    //        iss >> j;
    //        std::println("uv_i = {}", j);

    //        cur += 6;
    //        continue;
    //    }
    //    cur++;
    //}

    //return 0;

    d3::Window window(window_width, window_height, window_title);
    d3::Renderer& renderer = window.renderer;
    d3::Timer timer;
    window.set_target_fps(60);

    {
	d3::Texture t;
	if(!t.load_from_file("res/johanndr.jpg")) {
		    std::println("ERROR: could not load johanndr");
		    exit(0);
	}
	renderer.textures.push_back(t);
    }
    size_t teapot_id;
    if (!renderer.loadOBJ("res/utah_teapot_3.obj", teapot_id, {0}, 0)) {
        std::println("ERROR: could not load utah teapot obj");
        exit(0);
    }

    //renderer.push_cube();
    //renderer.push_cube(1, cube_transform, 0);
    //size_t cube_id = renderer.push_cube(1, cube_transform, 0);

    while(window.is_open) {

	//timer.start();
	controls(window);

	window.begin_frame();

	renderer.clear_pixels(d3::BLACK);

	renderer.draw_rec(rec, {0xFF, 0x00, 0x22, 0xFF} );

	//renderer.obj_set_transform(cube_id, cube_transform);
	renderer.set_cam_transform(camera_transform);
	renderer.draw_triangles();

	window.end_frame();

	//int mills = timer.get_delta_mills();
	//std::println("mills = {}", mills);

    }

    return 0;
}
