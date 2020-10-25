#version 430
layout(binding = 0, rgba32f) uniform writeonly image2D outputTexture;
layout(binding = 1, r32ui) uniform uimage2D particleCountTexture;
layout(binding = 5, r32ui) uniform uimage2D colorTextureR;
layout(binding = 6, r32ui) uniform uimage2D colorTextureG;
layout(binding = 7, r32ui) uniform uimage2D colorTextureB;
layout(local_size_x = 16, local_size_y = 16) in;

void main() {
    ivec2 pos = ivec2(gl_GlobalInvocationID.xy);
    float v = imageAtomicExchange(particleCountTexture, pos, 0);
    float r = imageAtomicExchange(colorTextureR, pos, 0);
    float g = imageAtomicExchange(colorTextureG, pos, 0);
    float b = imageAtomicExchange(colorTextureB, pos, 0);
    vec3 color = vec3(r,g,b);

//    if (v > 0) {
//        imageStore(outputTexture, pos, vec4(1,0,0,1.0));
//        return;
//    }

    color /= v;
    color /= 255;
    imageStore(outputTexture, pos, vec4(color, 1.0));
}
