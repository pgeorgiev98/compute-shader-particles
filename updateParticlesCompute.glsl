#version 430
layout(binding = 1, r32ui) uniform coherent uimage2D particleCountTexture;
layout(binding = 2, rgba32f) uniform image2D particlePositionTexture;
layout(binding = 3, rgba32f) uniform image2D imageTexture;
layout(binding = 4, rgba32f) uniform image2D particleDestinationTexture;
//layout(binding = 3, r32f) uniform image2D particleMassTexture;
layout(binding = 5, r32ui) uniform coherent uimage2D colorTextureR;
layout(binding = 6, r32ui) uniform coherent uimage2D colorTextureG;
layout(binding = 7, r32ui) uniform coherent uimage2D colorTextureB;
layout(local_size_x = 16, local_size_y = 16) in;
uniform vec2 imageSize;
uniform float forceMultiplier;
uniform vec2 cursorPos;
uniform float timeSinceLastFrame;
uniform bool mousePressed;
float w = ${width}, h = ${height};

void main() {
    ivec2 id = ivec2(gl_GlobalInvocationID.xy);
    vec4 v = imageLoad(particlePositionTexture, id);
    vec4 d = imageLoad(particleDestinationTexture, id);
//    float mass = imageLoad(particleMassTexture, id).r;
    float mass = 1;

    vec4 destinationColor = imageLoad(imageTexture, ivec2(d.x * imageSize.x/${width}, (${height} - d.y) * imageSize.y/${height}));
    if (mousePressed) {
        vec2 distVec = v.xy - cursorPos;
        float cursorDist = sqrt(dot(distVec, distVec));
        if (cursorDist < 80) {
            vec2 normDistVec = normalize(distVec);
            v.xy = clamp(cursorPos + normDistVec * 80, vec2(0,0), vec2(w, h));
//            v.zw -= normDistVec * 50;
        }
    }

    vec2 diff = v.xy - d.xy;
    float dist = dot(diff, diff);

    v.xy -= v.zw * timeSinceLastFrame;

//    if (dist < ${minimumDistance} && !mousePressed){
    if (dist < ${minimumDistance}){
        dist = 0.0;
        v.xy = d.xy;
        v.zw=vec2(0,0);
        uint p = imageAtomicAdd(particleCountTexture, ivec2(v.x, v.y), 1);
        imageAtomicAdd(colorTextureR, ivec2(v.xy), int(destinationColor.r));
        imageAtomicAdd(colorTextureG, ivec2(v.xy), int(destinationColor.g));
        imageAtomicAdd(colorTextureB, ivec2(v.xy), int(destinationColor.b));
    }else{
        float c;

        c = timeSinceLastFrame / mass;
//        v.z += c * dx;// * (dist/(w*w+h*h));
//        v.w += c * dy;// * (dist/(w*w+h*h));
//        if (!mousePressed) {
            v.zw = v.zw * 0.2 + c * diff * 0.8 * 500;
//        }

        uint p = imageAtomicAdd(particleCountTexture, ivec2(v.x, v.y), 1);
        imageAtomicAdd(colorTextureR, ivec2(v.x, v.y), int(destinationColor.r));
        imageAtomicAdd(colorTextureG, ivec2(v.x, v.y), int(destinationColor.g));
        imageAtomicAdd(colorTextureB, ivec2(v.x, v.y), int(destinationColor.b));

        float drag1 = ${drag1};
        float drag2 = ${drag2} * (v.z*v.z + v.w*v.w);
        float drag3 = ${drag3} * float(p);
        float drag = timeSinceLastFrame * (drag1 + drag2 + drag3);

        if (v.z > drag) v.z -= drag;
        else if (v.z < -drag) v.z += drag;
        else v.z = 0.0;

        if (v.w > drag) v.w -= drag;
        else if (v.w < -drag) v.w += drag;
        else v.w = 0.0;

#if ${wallCollisionAction} == 1

        if (v.x < 0.0) { v.x = 0.0; v.z = 0.0; }
        if (v.x > ${width}) { v.x = ${width}; v.z = 0.0; }
        if (v.y < 0.0) { v.y = 0.0; v.w = 0.0; }
        if (v.y > ${height}) { v.y = ${height}; v.w = 0.0; }

#elif ${wallCollisionAction} == 2

        float s = ${collisionReflectSpeed};
        if (v.x < 0.0) { v.x = 0.0; v.z *= -s; v.w *= s; }
        if (v.x > ${width}) { v.x = ${width}; v.z *= -s; v.w *= s; }
        if (v.y < 0.0) { v.y = 0.0; v.w *= -s; v.z *= s; }
        if (v.y > ${height}) { v.y = ${height}; v.w *= -s; v.z *= s; }

#endif
    }

    imageStore(particlePositionTexture, id, v);
}
