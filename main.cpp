#include <GLFW/glfw3.h>
#include <bgfx/platform.h>
#include <bgfx/bgfx.h>
#include "bgfx_utils.h"
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

struct NormalColorVertex
{
    glm::vec2 position;
    uint32_t color;
};
int main() {
    if (!glfwInit()) return 1;

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* win = glfwCreateWindow(1280, 720, "bgfx triangle", nullptr, nullptr);

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
    init.type = bgfx::RendererType::OpenGL;
    init.platformData = pd;
    init.resolution.width = 1280;
    init.resolution.height = 720;
    init.resolution.reset = BGFX_RESET_VSYNC;
    bgfx::init(init);
    bgfx::setDebug(BGFX_DEBUG_TEXT);


    bgfx::VertexLayout color_vertex_layout;
    color_vertex_layout.begin()
                       .add(bgfx::Attrib::Position, 2, bgfx::AttribType::Float)
                       .add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true)
                       .end();

    NormalColorVertex kTriangleVertices[] =
        {
        {{-0.5f, -0.5f}, 0x339933FF},
        {{0.5f, -0.5f}, 0x993333FF},
        {{0.0f, 0.5f}, 0x333399FF},
};

    const uint16_t kTriangleIndices[] =
            {
        0, 1, 2,
};

    bgfx::VertexBufferHandle vertex_buffer = bgfx::createVertexBuffer(bgfx::makeRef(kTriangleVertices, sizeof(kTriangleVertices)), color_vertex_layout);
    bgfx::IndexBufferHandle index_buffer = bgfx::createIndexBuffer(bgfx::makeRef(kTriangleIndices, sizeof(kTriangleIndices)));

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

    while (!glfwWindowShouldClose(win)) {
        glfwPollEvents();
        bgfx::dbgTextClear();
        bgfx::dbgTextPrintf(0, 1, 0x4f, "Hello from bgfx!");
        int w, h;
        glfwGetFramebufferSize(win, &w, &h);
        bgfx::reset(w, h, BGFX_RESET_VSYNC);
        bgfx::setViewRect(0, 0, 0, w, h);
        bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x303030ff, 1.0f, 0);


        bgfx::setState(
        BGFX_STATE_WRITE_R
                | BGFX_STATE_WRITE_G
                | BGFX_STATE_WRITE_B
                | BGFX_STATE_WRITE_A
        );

        bgfx::setVertexBuffer(0, vertex_buffer);
        bgfx::setIndexBuffer(index_buffer); // not needed if you don't do indexed draws
        bgfx::submit(0, program);

        bgfx::frame();
    }

    bgfx::shutdown();
    glfwDestroyWindow(win);
    glfwTerminate();
    return 0;
}
