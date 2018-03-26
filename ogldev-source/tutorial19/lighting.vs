#version 330                                                                        
                                                                                    
layout (location = 0) in vec3 Position;                                             
layout (location = 1) in vec2 TexCoord;                                             
layout (location = 2) in vec3 Normal;                                               
                                                                                    
uniform mat4 gWVP;                                                                  
uniform mat4 gWorld;                                                                
                                                                                    
out vec2 TexCoord0;                                                                 
out vec3 Normal0;      
//经过完整的变换（world-view-projection变换）后结果传递给系统变量’gl_Position’，然后GPU负责将它变换到屏幕空间坐标系并用来进行实际的光栅化操作。
//局部变换到世界空间的结果传给了一个用户自定义的属性，这个属性在光栅化阶段被进行了简单的插值，所以片段着色器中激活的每一个像素都会提供它自己的世界空间位置坐标                                                             
out vec3 WorldPos0;                                                                 
                                                                                    
void main()                                                                         
{                                                                                   
    gl_Position = gWVP * vec4(Position, 1.0);                                       
    TexCoord0   = TexCoord;                                                         
    Normal0     = (gWorld * vec4(Normal, 0.0)).xyz;                                 
    WorldPos0   = (gWorld * vec4(Position, 1.0)).xyz;                               
}
