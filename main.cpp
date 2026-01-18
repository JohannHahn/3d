#include <cstdio>
#include <print>
#include <stdint.h>
#include "d3.hpp"
#include <gmath/gmath.hpp>
#include <winuser.h>


const char* window_title = "3D";
POINT mouse_prev = {0, 0};

//constexpr float aspect_ratio = 16.f / 9.f;
constexpr uint64_t window_width = 900;
constexpr uint64_t window_height = 600;


d3::Transform cube_transform = {{0, 0, 2}, {0}};
d3::Transform camera_transform = {{0, 0, -20}, {0}};
d3::Transform surf_transform = {{0, 0, 0}, {0}};

float camera_speed = 0.1f;
gmath::Vec3 camera_dir = {0, 0, 0};

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

void reduce_angles(gmath::Vec3& angles) {
    float pi2 = gmath::PI * 2.f;
	if (angles.x >= pi2) {
	    angles.x -= pi2;
	}
	if (angles.y >= pi2) {
	    angles.y -= pi2;
	}
	if (angles.z >= pi2) {
	    angles.z -= pi2;
	}
	if (angles.x <= pi2) {
	    angles.x += pi2;
	}
	if (angles.y <= pi2) {
	    angles.y += pi2;
	}
	if (angles.z <= pi2) {
	    angles.z += pi2;
	}
    
}

void reduce_angles_all() {
    reduce_angles(surf_transform.angles);    
    reduce_angles(camera_transform.angles);    
    reduce_angles(cube_transform.angles);    
}

void controls(d3::Window& window) {

	float step = 0.05f;
	camera_dir = {0, 0, 0};


	if (GetAsyncKeyState(VK_LEFT) & 0x8000) {
	    cube_transform.position.x -= step;
	    surf_transform.angles.z += step;
	}
	if (GetAsyncKeyState(VK_RIGHT) & 0x8000) {
	    cube_transform.position.x += step;
	    surf_transform.angles.z -= step;
	}

	if (GetAsyncKeyState(VK_UP) & 0x8000) {
	    //camera_transform.position.y += step;
	    //camera_dir.y = 
	    surf_transform.angles.x += step;
	}
	if (GetAsyncKeyState(VK_DOWN) & 0x8000) {
	    //camera_transform.position.y -= step;
	    surf_transform.angles.x -= step;
	}

	if (GetAsyncKeyState('W') & 0x8000) {
	    //camera_transform.position.z += step;
	    camera_dir.z = 1.f;
	}
	if (GetAsyncKeyState('S') & 0x8000) {
	    //camera_transform.position.z -= step;
	    camera_dir.z = -1.f;
	}
	if (GetAsyncKeyState('A') & 0x8000) {
	    //camera_transform.position.x -= step;
	    camera_dir.x = -1.f;
	}
	if (GetAsyncKeyState('D') & 0x8000) {
	    //camera_transform.position.x += step;
	    camera_dir.x = 1.f;
	}

	if (GetAsyncKeyState(VK_SPACE) & 0x8000) {
	    camera_transform.position.y += step;
	}
	if (GetAsyncKeyState(VK_SHIFT) & 0x8000) {
	    camera_transform.position.y -= step;
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

	if (GetAsyncKeyState(VK_RBUTTON) & 0x8000) {
	    int dif_x = mouse_prev.x - mouse.x;
	    int dif_y = mouse_prev.y - mouse.y;
	    camera_transform.angles.y -= dif_x * step / 4.f;
	    camera_transform.angles.x -= dif_y * step / 4.f;
	}
	mouse_prev = mouse;

	//reduce_angles_all();
}

size_t load_teapot(d3::Renderer& renderer, const d3::Transform& t = {0}, size_t tex_id = -1) {

    size_t teapot_id;
    if (!renderer.loadOBJ("res/utah_teapot_3.obj", teapot_id, t, tex_id)) {
        std::println("ERROR: could not load utah teapot obj");
        exit(0);
    }
    std::println("Loaded teapot");
    return teapot_id;
}

// 4 verts anti-clockwise, start top left
size_t push_surface(d3::Renderer& renderer, const d3::Vertex3* verts, gmath::Vec3 normal, int tex_id = -1) {
    size_t v_start = renderer.vertices.size();
    size_t uv_start = renderer.uvs.size();
    size_t n_start = renderer.normals.size();

    constexpr size_t v_size = 4;

    renderer.push_vertices(verts, v_size);

    d3::IndexRange range;
    range.start = renderer.faces.size();
    range.count = 2;

    renderer.push_uvs(d3::cube_uvs, d3::cube_uvs_size);
    renderer.push_normals(&normal, 1);
    //uvs:
    //0.f, 0.f,
    //0.f, 1.f, 
    //1.f, 0.f,  
    //1.f, 1.f
    constexpr size_t faces_size = 2;
    d3::Face faces[faces_size] = {
	{{{0 + v_start, 0 + uv_start, 0 + n_start}, {1 + v_start, 1 + uv_start, 0 + n_start}, {2 + v_start, 2 + uv_start, 0 + n_start}}, tex_id},
	{{{2 + v_start, 2 + uv_start, 0}, {1 + v_start, 1 + uv_start, 0 + n_start}, {3 + v_start, 3 + uv_start, 0 + n_start}}, tex_id}
    };
    renderer.push_faces(faces, faces_size);
    size_t id = renderer.push_object({0}, range);

    return id;
}

size_t push_pyramid(d3::Renderer& renderer, gmath::Vec3 origin, gmath::Vec3 dir, float width, float height) {
    d3::Transform t = {origin, {0}};
    d3::IndexRange range;
    range.start = renderer.faces.size();
    range.count = 5;

    size_t id = renderer.push_object(t, range);
    constexpr size_t vert_count = 5;
    d3::Vertex3 verts[vert_count] = {
	{},
	{},
	{},
	{},
	{},
    };

    renderer.push_vertices(verts, vert_count);
    return id;
}

int main() {

    d3::Window window(window_width, window_height, window_title);
    d3::Renderer& renderer = window.renderer;
    renderer.far_clip = 100.f;
    d3::Timer timer;
    window.set_target_fps(60);

    {
	d3::Texture t;
	if(!t.load_from_file("res/johanndr.jpg")) {
		    std::println("ERROR: could not load johanndr");
		    exit(0);
	}
	renderer.textures.push_back(t);
	d3::Texture t2;
	if(!t2.load_from_file("res/puto.jpg")) {
		    std::println("ERROR: could not load puto");
		    exit(0);
	}
	renderer.textures.push_back(t2);
    }

    size_t teapot_id = load_teapot(renderer, {0, 5, 0}, 1);
    renderer.push_cube(.5f, {{0, 0, -2}}, 0);
    size_t cube_id = renderer.push_cube(3, cube_transform, 1);


    float surf_size = 10.f;
    d3::Vertex3 surf_verts[4] = {
        {{-surf_size / 2.f,  surf_size / 2.f, -1.f }},
        {{ surf_size / 2.f,  surf_size / 2.f, -1.f }},
        {{-surf_size / 2.f, -surf_size / 2.f, -1.f }},
        {{ surf_size / 2.f, -surf_size / 2.f, -1.f }}
    };
    size_t surf_id = push_surface(renderer, surf_verts, {0, 0, -1});



    while(window.is_open) {

	//timer.start();
	controls(window);


	window.begin_frame();

	renderer.clear_pixels(d3::GRAY);

	renderer.draw_rec(rec, {0xFF, 0x00, 0x22, 0xFF} );

	renderer.obj_set_transform(cube_id, cube_transform);
	renderer.obj_set_transform(surf_id, surf_transform);
	camera_transform.move_dir(camera_dir, camera_speed);
	renderer.set_cam_transform(camera_transform);
	renderer.draw_triangles();
	renderer.draw_triangles_wireframe(d3::WHITE);

	window.end_frame();

	//int mills = timer.get_delta_mills();
	//std::println("mills = {}", mills);

    }

    return 0;
}
