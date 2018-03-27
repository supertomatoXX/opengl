#version 330

in vec2 TexCoord0;

out vec4 FragColor;

struct DirectionalLight
{
    vec3 Color;
    float AmbientIntensity;
};

//颜色值和环境光强度通过上面的方式获得：一个是在shader重定义的一致变量的名字（gDirectionalLight），
//然后用点运算符获取结构体中两个区域的变量。同时设置结构体中的这些变量的值也和其他普通变量一样进行直接赋值。
uniform DirectionalLight gDirectionalLight;
uniform sampler2D gSampler;

void main()
{
    FragColor = texture2D(gSampler, TexCoord0.xy) * 
                vec4(gDirectionalLight.Color, 1.0f) *
                gDirectionalLight.AmbientIntensity;
}
