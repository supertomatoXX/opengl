#version 330                                                                        
                                                                                    
uniform sampler2D gColorMap;                                                        
                                                                                    
in vec2 TexCoord;                                                                   
out vec4 FragColor;                                                                 
                                                                                    
void main()                                                                         
{                                                                                   
    FragColor = texture2D(gColorMap, TexCoord);                                     
    

    //内置的关键字’discard’用于在某些情况下将某些像素片元完全丢弃                                                                                
    if (FragColor.r == 0 && FragColor.g == 0 && FragColor.b == 0) {
        discard;                                                                    
    }                                                                               
}