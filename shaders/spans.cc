#include <span>

#include <cstdint>

#include "shaders/spans.hh"

namespace {

uint32_t trig_vertex_shader_code[] = {
#include "vertex.glsl.spv.hex"
};

uint32_t trig_fragment_shader_code[] = {
#include "fragment.glsl.spv.hex"
};

uint32_t trsq_vertext_shader_code[] = {
#include "trsq_vertex.glsl.spv.hex"
};

uint32_t trsq_fragment_shader_code[] = {
#include "trsq_fragment.glsl.spv.hex"
};

}; // namespace

std::span<uint32_t> get_shader_vert_trig()
{
    return trig_vertex_shader_code;
}

std::span<uint32_t> get_shader_frag_trig()
{
    return trig_fragment_shader_code;
}

std::span<uint32_t> get_shader_vert_trsq()
{
    return trsq_vertext_shader_code;
}

std::span<uint32_t> get_shader_frag_trsq()
{
    return trsq_fragment_shader_code;
}
