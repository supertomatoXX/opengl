#version 330

layout (location = 0) in vec3 Position;
layout (location = 1) in vec2 TexCoord;

/*
（1）我们可以像这里我们所做的一样明确设置它（为0），那样我们就可以在我们的应用中使用一个写死的编码值（使用第一个参数调用glVertexAttributePointer）； 
（2）或者我们可以不管它（简单的在shader中声明’in vec3 Position’）然后使用glGetAttribLocation检索应用中这个实时的位置。在那种情况下，我们还需要为glVertexAttributePointer提供返回值，而不是使用写死的编码值。这里我们选择简单的方式，但是在复杂的应用中最好让编译器定义属性的索引标志然后实时检索他们。这样可以更容易的从多个来源整合shader着色器而不用使他们适应你的缓冲器布局。
*/
uniform mat4 gWVP;

out vec2 TexCoord0;

void main()
{
    gl_Position = gWVP * vec4(Position, 1.0);
    TexCoord0 = TexCoord;
}
