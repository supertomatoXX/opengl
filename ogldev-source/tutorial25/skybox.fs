#version 330                                                                        
                                                                                    
in vec3 TexCoord0;                                                                  
                                                                                    
out vec4 FragColor;                                                                 

//我们这里是要使用’samplerCube’而不是’sampler2D’以获取cubemap的纹理。                                                                                    
uniform samplerCube gCubemapTexture;                                                
                                                                                    
void main()                                                                         
{                                                                                   
    FragColor = texture(gCubemapTexture, TexCoord0);                                
}
