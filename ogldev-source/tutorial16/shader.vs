#version 330

layout (location = 0) in vec3 Position;
layout (location = 1) in vec2 TexCoord;

/*
��1�����ǿ�������������������һ����ȷ��������Ϊ0�����������ǾͿ��������ǵ�Ӧ����ʹ��һ��д���ı���ֵ��ʹ�õ�һ����������glVertexAttributePointer���� 
��2���������ǿ��Բ��������򵥵���shader��������in vec3 Position����Ȼ��ʹ��glGetAttribLocation����Ӧ�������ʵʱ��λ�á�����������£����ǻ���ҪΪglVertexAttributePointer�ṩ����ֵ��������ʹ��д���ı���ֵ����������ѡ��򵥵ķ�ʽ�������ڸ��ӵ�Ӧ��������ñ������������Ե�������־Ȼ��ʵʱ�������ǡ��������Ը����׵ĴӶ����Դ����shader��ɫ��������ʹ������Ӧ��Ļ��������֡�
*/
uniform mat4 gWVP;

out vec2 TexCoord0;

void main()
{
    gl_Position = gWVP * vec4(Position, 1.0);
    TexCoord0 = TexCoord;
}
