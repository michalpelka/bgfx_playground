#pragma once
#include <bgfx/bgfx.h>
#include <iostream>
#include <fstream>
#include <vector>

inline bgfx::ShaderHandle loadShader(const std::string& path) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file) {
        std::cerr << "Failed to load shader: " << path << std::endl;
        return BGFX_INVALID_HANDLE;
    }

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);
    auto mem = bgfx::alloc((uint32_t)size + 1);
    file.read(reinterpret_cast<char*>(mem->data), size);
    mem->data[size] = '\0';
    return bgfx::createShader(mem);
}

inline bgfx::ProgramHandle loadProgram(const std::string& vs, const std::string& fs) {
    return bgfx::createProgram(loadShader(vs), loadShader(fs), true);
}
