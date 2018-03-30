#version 330                                                                        
                                                                                    
in vec3 TexCoord0;                                                                  
                                                                                    
out vec4 FragColor;                                                                 

//����������Ҫʹ�á�samplerCube�������ǡ�sampler2D���Ի�ȡcubemap������                                                                                    
uniform samplerCube gCubemapTexture;                                                
                                                                                    
void main()                                                                         
{                                                                                   
    FragColor = texture(gCubemapTexture, TexCoord0);                                
}
