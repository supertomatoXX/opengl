#version 330


//���ǲ�ֵ֮��Ľ��
//�����ڽ��ܵ���ֵ���ڶ�����ɫ����ת��������ռ�Ķ��㷨����
in vec2 TexCoord0;
in vec3 Normal0;                                                                    
                                                                                    
out vec4 FragColor;                                                                 
                                                                                    
struct DirectionalLight                                                             
{                                                                                   
    vec3 Color;                                                                     
    float AmbientIntensity;                                                         
    float DiffuseIntensity;                                                         
    vec3 Direction;                                                                 
};                                                                                  
                                                                                    
uniform DirectionalLight gDirectionalLight;                                         
uniform sampler2D gSampler;                                                         
                                                                                    
void main()                                                                         
{                  


 //   1. �Ӷ�����ɫ�������ķ�������ʹ��֮ǰ�Ǿ�����λ���˵ģ���Ϊ������ֵ֮���������ĳ��ȿ��ܻ�仯�����ǵ�λ�����ˣ�
 //   2.��Դ�ķ��򱻷������ˣ���Ϊ�������ߴ�ֱ���䵽����ʱ�ķ���ͷ�������ʵ�����෴�ĳ�180�Ƚǣ������ʱ�򽫹�Դ����ȡ����ô��ֱ����ʱ�ͷ��߼н�Ϊ0��
 //��ʱ�ź����ǵļ�������ϡ�
 //   3.��Դ�������ǵ�λ���ġ�������������ص�ͬһ�����������з�����λ������˷�GPU��Դ���������ֻҪ��֤Ӧ�ó��򴫵ݵ�������draw call֮ǰ����λ�����ɡ�
                                                                 
    vec4 AmbientColor = vec4(gDirectionalLight.Color, 1.0f) *                       
                        gDirectionalLight.AmbientIntensity;                         
                                                                                    
    float DiffuseFactor = dot(normalize(Normal0), -gDirectionalLight.Direction);    
                                                                                    
    vec4 DiffuseColor;                                                              
                                                                                    
    if (DiffuseFactor > 0) {                                                        
        DiffuseColor = vec4(gDirectionalLight.Color, 1.0f) *                        
                       gDirectionalLight.DiffuseIntensity *                         
                       DiffuseFactor;                                               
    }                                                                               
    else {                                                                          
        DiffuseColor = vec4(0, 0, 0, 0);                                            
    }                                                                               
                                  
// // �������Ǹ��ݹ����ɫ�������ǿ�Ⱥ͹�ķ��������������Ĳ��֡������������Ǹ��Ļ���Ϊ0��ζ�Ź�������һ���۽��䵽�������ģ���ˮƽһ����߱���ı��棩,
//��ʱ�������û��Ч����ͬʱ��������ɫ�����ᱻ��ʼ������Ϊ�㡣����нǴ���0���ǾͿ��Խ��м�����������ɫֵ�ˣ�
//����������ɫֵ�������ǿ�ȳ�����ˣ����ʹ���������DiffuseFactor��������������š�������Ǵ�ֱ������ô�����������1������������
                                                  
    FragColor = texture2D(gSampler, TexCoord0.xy) *                                 
                (AmbientColor + DiffuseColor);                                      
}
