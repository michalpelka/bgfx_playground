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

#include "3rd/bgfx.cmake/bgfx/src/bgfx_p.h"
#include "3rd/bgfx.cmake/bx/include/bx/math.h"


struct NormalColorVertex
{
    glm::vec3 position;
    uint32_t color;
};
int main() {
    std::string fn = "/home/michal/Downloads/4979_281539_M-34-100-B-d-1-2-3.laz";


    auto points = mandeye::load(fn);
    std::cout << "Loaded " << points.size() << " points from " << fn << std::endl;


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

        bx::packRgba8U(&kTriangleVertices[i].color, colors);
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

        float view[16];
        float proj[16];
        bx::mtxLookAt(view, bx::Vec3{0.0f, 0.0f, -2.0f}, bx::Vec3{0.0f, 0.0f, 0.0f});
        bx::mtxProj(proj, 60.0f, float(w) / float(h), 0.1f, 100.0f, bgfx::getCaps()->homogeneousDepth);
        float model[16];
        bx::mtxIdentity(model);
        bx::mtxRotateY(model, bx::kPi * (float)glfwGetTime() * 0.1f);
        float mvp[16];
        bx::mtxMul(mvp, model, view);
        bx::mtxMul(mvp, mvp, proj);
        bgfx::setUniform(u_mvp, mvp);

        bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_PT_POINTS);
        bgfx::setVertexBuffer(0, vertex_buffer);
        bgfx::submit(0, program);

        bgfx::frame();
    }
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
