#version 330                                                                        
                                                                                    
layout (location = 0) in vec3 Position;                                             
layout (location = 1) in vec2 TexCoord;                                             
layout (location = 2) in vec3 Normal;                                               
layout (location = 3) in mat4 WVP;                                                  
layout (location = 7) in mat4 World;                                                
                                                                                    
out vec2 TexCoord0;                                                                 
out vec3 Normal0;                                                                   
out vec3 WorldPos0;                                                                 
flat out int InstanceID;                           
                                 
//'gl_InstanceID' 是一个内置变量，它能在 VS 中使用，但是因为我们打算在 FS 中使用它，所以这里我们借助一个输出变量将其传入到 FS 中。
//gl_InstanceID 是一个整型，所以我们也使用一个整型的输出变量，由于整型数据不能再光栅化阶段进行插值，所以我们需要将这个输出变量标记为 'flat'（如果不这么做会产生编译错误）     \
                                                                               
void main()                                                                         
{                                                                                   
    gl_Position = WVP * vec4(Position, 1.0);                                        
    TexCoord0   = TexCoord;                                                         
    Normal0     = (World * vec4(Normal, 0.0)).xyz;                                  
    WorldPos0   = (World * vec4(Position, 1.0)).xyz;                                
    InstanceID = gl_InstanceID;                                                     
}