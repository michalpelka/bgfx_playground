#include <GLFW/glfw3.h>
#include <bgfx/platform.h>
#include <bgfx/bgfx.h>
#include "bgfx_utils.h"
#include "bx/pixelformat.h"
#include "color_las_loader.h"
#if defined(__linux__)
    #define GLFW_EXPOSE_NATIVE_X11
    #include <GLFW/glfw3native.h>
#endif
#include <bx/math.h>
#include "shaders/glsl/vs_triangle.sc.bin.h"
#include "shaders/glsl/fs_triangle.sc.bin.h"
#include "shaders/essl/vs_triangle.sc.bin.h"
#include "shaders/essl/fs_triangle.sc.bin.h"
#include "shaders/spirv/vs_triangle.sc.bin.h"
#include "shaders/spirv/fs_triangle.sc.bin.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "3rd/bgfx.cmake/bgfx/examples/common/imgui/imgui.h"
#include "3rd/bgfx.cmake/bgfx/src/bgfx_p.h"
#include "3rd/bgfx.cmake/bx/include/bx/math.h"

#include "common/imgui/imgui.h"


struct NormalColorVertex
{
    glm::vec3 position;
    uint32_t color;
};


float translate_x =0.0, translate_y = 0.0;
float translate_z = -500.0;
float rotate_x = 0.0, rotate_y = 0.0;
float mouse_old_x, mouse_old_y = 0.0;
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset) {
    translate_z += yoffset * 10.0f;
}

void mouse_callback(GLFWwindow *window, double x, double y) {
    float dx, dy;
    dx = (float)(x - mouse_old_x);
    dy = (float)(y - mouse_old_y);
    mouse_old_x = x;
    mouse_old_y = y;

    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
        rotate_x += dy * 0.2f; // * mouse_sensitivity;
        rotate_y += dx * 0.2f; // * mouse_sensitivity;
    }
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
        translate_x += dx * 0.05f ;//* mouse_sensitivity;
        translate_y -= dy * 0.05f ;//* mouse_sensitivity;
    }
}

int main() {
    std::string fn = "/home/michal/Downloads/4979_281539_M-34-100-B-d-1-2-3.laz";


    auto points = mandeye::load(fn);
    std::cout << "Loaded " << points.size() << " points from " << fn << std::endl;


    if (!glfwInit()) return 1;

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* win = glfwCreateWindow(1280, 720, "bgfx triangle", nullptr, nullptr);
    glfwSetCursorPosCallback(win, mouse_callback);
    glfwSetScrollCallback(win, scroll_callback);


    bgfx::PlatformData pd{};
#if defined(_WIN32)
    pd.nwh = glfwGetWin32Window(win);
#elif defined(__APPLE__)
    pd.nwh = glfwGetCocoaWindow(win);
#else
    pd.ndt = glfwGetX11Display();
    pd.nwh = (void*)(uintptr_t)glfwGetX11Window(win);
#endif

    bgfx::Init init;
    init.type = bgfx::RendererType::Vulkan;
    init.platformData = pd;
    init.resolution.width = 1280;
    init.resolution.height = 720;
    init.resolution.reset = BGFX_RESET_VSYNC;
    bgfx::init(init);
    bgfx::setDebug(BGFX_DEBUG_TEXT);

    imguiCreate();

    bgfx::VertexLayout color_vertex_layout;
    color_vertex_layout.begin()
                       .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
                       .add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true)
                       .end();

    // NormalColorVertex kTriangleVertices[] ;
//     const uint16_t kTriangleIndices[] =
//             {
//         0, 1, 2,
// };
    std::vector<NormalColorVertex> kTriangleVertices(points.size());

    auto fp = points.front();
    for (size_t i = 0; i < points.size(); i++)
    {
        Eigen::Vector3d p = points[i].point- fp.point;
        kTriangleVertices[i].position.x = p.x();
        kTriangleVertices[i].position.y = p.y();
        kTriangleVertices[i].position.z = p.z();

        float colors[4] = {points[i].rgb[0]/255.f, points[i].rgb[1]/255.f, points[i].rgb[2]/255.f, points[i].rgb[3]/255.f};

        bx::packRgba8(&kTriangleVertices[i].color, colors);
    }
    bgfx::VertexBufferHandle vertex_buffer = bgfx::createVertexBuffer(bgfx::makeRef(kTriangleVertices.data(), sizeof(NormalColorVertex)*kTriangleVertices.size()), color_vertex_layout);
    //bgfx::IndexBufferHandle index_buffer = bgfx::createIndexBuffer(bgfx::makeRef(kTriangleIndices, sizeof(kTriangleIndices)));

    const bgfx::Memory* vs_mem = nullptr;
    const bgfx::Memory* fs_mem = nullptr;

    switch (bgfx::getRendererType())
    {
        case bgfx::RendererType::OpenGL:
            vs_mem = bgfx::copy(vs_triangle_glsl, sizeof(vs_triangle_glsl));
        fs_mem = bgfx::copy(fs_triangle_glsl, sizeof(fs_triangle_glsl));
        break;
        case bgfx::RendererType::OpenGLES:
            vs_mem = bgfx::copy(vs_triangle_essl, sizeof(vs_triangle_essl));
        fs_mem = bgfx::copy(fs_triangle_essl, sizeof(fs_triangle_essl));
        break;
        case bgfx::RendererType::Vulkan:
            vs_mem = bgfx::copy(vs_triangle_spv, sizeof(vs_triangle_spv));
        fs_mem = bgfx::copy(fs_triangle_spv, sizeof(fs_triangle_spv));
        break;
        default:
            std::cerr << "Unsupported renderer type!" << std::endl;
        return 1;
    }

    bgfx::ShaderHandle vsh = bgfx::createShader(vs_mem);
    bgfx::ShaderHandle fsh = bgfx::createShader(fs_mem);
    bgfx::ProgramHandle program = bgfx::createProgram(vsh, fsh, true);


    bgfx::UniformHandle u_mvp = bgfx::createUniform("u_mvp", bgfx::UniformType::Mat4);

    while (!glfwWindowShouldClose(win)) {
        glfwPollEvents();
        bgfx::dbgTextClear();
        bgfx::dbgTextPrintf(0, 1, 0x4f, "Hello from bgfx!");
        int w, h;
        glfwGetFramebufferSize(win, &w, &h);
        bgfx::reset(w, h, BGFX_RESET_VSYNC);
        bgfx::setViewRect(0, 0, 0, w, h);
        bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x303030ff, 1.0f, 0);
        auto view = glm::mat4(1.0f);
        view = glm::translate(view, glm::vec3(translate_x, translate_y, translate_z));
        view = glm::rotate(view, glm::radians(rotate_x), glm::vec3(1.0f, 0.0f, 0.0f));
        view = glm::rotate(view, glm::radians(rotate_y), glm::vec3(0.0f, 0.0f, 1.0f));

        glm::mat4 projection =glm::perspective(glm::radians(45.0f), (float)w / (float)h, 0.1f, 100000.0f);
        glm::mat4 mvp = projection * view;
        bgfx::setUniform(u_mvp, &mvp[0]);

        bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_PT_POINTS);
        bgfx::setVertexBuffer(0, vertex_buffer);
        bgfx::submit(0, program);

        {
            // Get GLFW window size and mouse position
            int win_width, win_height;
            glfwGetFramebufferSize(win, &win_width, &win_height);

            double mouse_x, mouse_y;
            glfwGetCursorPos(win, &mouse_x, &mouse_y);

            // Compose mouse button bitmask
            uint8_t mouse_buttons = 0;
            if (glfwGetMouseButton(win, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) mouse_buttons |= IMGUI_MBUT_LEFT;
            if (glfwGetMouseButton(win, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) mouse_buttons |= IMGUI_MBUT_RIGHT;
            if (glfwGetMouseButton(win, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS) mouse_buttons |= IMGUI_MBUT_MIDDLE;
            imguiBeginFrame(static_cast<int>(mouse_x), static_cast<int>(mouse_y), mouse_buttons, 0, w, h);

        }


        ImGui::Begin("Controls");
        ImGui::Text("ImGui FPS: %.1f (%.3f ms/frame)", ImGui::GetIO().Framerate, 1000.0f / ImGui::GetIO().Framerate);

        ImGui::End();
        imguiEndFrame();
        bgfx::frame();
    }
    imguiDestroy();
    bgfx::destroy(vertex_buffer);
    // bgfx::destroy(index_buffer);
    bgfx::destroy(vsh);
    bgfx::destroy(fsh);
    bgfx::destroy(program);
    bgfx::release(vs_mem);
    bgfx::release(fs_mem);

    bgfx::shutdown();
    glfwDestroyWindow(win);
    glfwTerminate();
    return 0;
}
