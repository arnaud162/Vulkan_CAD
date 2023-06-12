#version 440

layout(location = 0) in vec3 fragColor;

layout(location = 0) out vec4 outColor;

vec3 whatever=vec3(0.0,0.0,1.0);

void main()
{
    outColor = vec4(fragColor, 1.0);
    //outColor = vec4(whatever, 1.0); // avec whatever tout est en bleu
}

//layout(location = 0) in vec3 fragColor;
//layout(location = 1) in vec2 fragTexCoord;

//layout(location = 0) out vec4 outColor;

//void main() {
//    outColor = vec4(fragTexCoord, 0.0, 1.0);
//}
