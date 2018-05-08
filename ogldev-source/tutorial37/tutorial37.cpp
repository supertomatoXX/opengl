/*

	Copyright 2011 Etay Meiri

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

    Tutorial 37 - Deferred Shading - Part 3
*/

#include <math.h>
#include <GL/glew.h>
#include <GL/freeglut.h>

#include "ogldev_engine_common.h"
#include "ogldev_app.h"
#include "ogldev_camera.h"
#include "ogldev_util.h"
#include "ogldev_pipeline.h"
#include "ogldev_glut_backend.h"
#include "ogldev_basic_mesh.h"
#include "ogldev_lights_common.h"
#include "gbuffer.h"
#include "null_technique.h"
#include "ds_geom_pass_tech.h"
#include "ds_point_light_pass_tech.h"
#include "ds_dir_light_pass_tech.h"

//模板缓存中还有一点值得我们注意的是——模板缓存并不是一个单独的缓存，实际上它是深度缓存的一部分，
//对于深度缓存中的每个像素你可以分配 24 位或者 32 位的空间用来存放深度信息，而利用另外 8 位空间用来存放模板数据。

#define WINDOW_WIDTH  1280
#define WINDOW_HEIGHT 1024

class Tutorial37 : public ICallbacks, public OgldevApp
{
public:

    Tutorial37() 
    {
        m_pGameCamera = NULL;
        m_scale = 0.0f;

        m_persProjInfo.FOV = 60.0f;
        m_persProjInfo.Height = WINDOW_HEIGHT;
        m_persProjInfo.Width = WINDOW_WIDTH;
        m_persProjInfo.zNear = 1.0f;
        m_persProjInfo.zFar = 100.0f;  
        
        InitLights();
        InitBoxPositions();
    }


    ~Tutorial37()
    {
        SAFE_DELETE(m_pGameCamera);
    }    

    bool Init()
    {
        if (!m_gbuffer.Init(WINDOW_WIDTH, WINDOW_HEIGHT)) {
            return false;
        }

        m_pGameCamera = new Camera(WINDOW_WIDTH, WINDOW_HEIGHT);

        if (!m_DSGeomPassTech.Init()) {
            printf("Error initializing DSGeomPassTech\n");
            return false;
        }      

		m_DSGeomPassTech.Enable();
		m_DSGeomPassTech.SetColorTextureUnit(COLOR_TEXTURE_UNIT_INDEX);

		if (!m_DSPointLightPassTech.Init()) {
			printf("Error initializing DSPointLightPassTech\n");
			return false;
		}
	
		m_DSPointLightPassTech.Enable();

		m_DSPointLightPassTech.SetPositionTextureUnit(GBuffer::GBUFFER_TEXTURE_TYPE_POSITION);
		m_DSPointLightPassTech.SetColorTextureUnit(GBuffer::GBUFFER_TEXTURE_TYPE_DIFFUSE);
		m_DSPointLightPassTech.SetNormalTextureUnit(GBuffer::GBUFFER_TEXTURE_TYPE_NORMAL);
        m_DSPointLightPassTech.SetScreenSize(WINDOW_WIDTH, WINDOW_HEIGHT);

		if (!m_DSDirLightPassTech.Init()) {
			printf("Error initializing DSDirLightPassTech\n");
			return false;
		}
	
		m_DSDirLightPassTech.Enable();

		m_DSDirLightPassTech.SetPositionTextureUnit(GBuffer::GBUFFER_TEXTURE_TYPE_POSITION);
		m_DSDirLightPassTech.SetColorTextureUnit(GBuffer::GBUFFER_TEXTURE_TYPE_DIFFUSE);
		m_DSDirLightPassTech.SetNormalTextureUnit(GBuffer::GBUFFER_TEXTURE_TYPE_NORMAL);
		m_DSDirLightPassTech.SetDirectionalLight(m_dirLight);
        m_DSDirLightPassTech.SetScreenSize(WINDOW_WIDTH, WINDOW_HEIGHT);
        Matrix4f WVP;
        WVP.InitIdentity();
        m_DSDirLightPassTech.SetWVP(WVP);

		if (!m_nullTech.Init()) {
			return false;
		}
        
        if (!m_quad.LoadMesh("../Content/quad.obj")) {
            return false;
        }

        if (!m_box.LoadMesh("../Content/box.obj")) {
			return false;
		}

        if (!m_bsphere.LoadMesh("../Content/sphere.obj")) {
			return false;
		}

#ifndef WIN32
        if (!m_fontRenderer.InitFontRenderer()) {
            return false;
        }
#endif        	
		return true;
    }

    void Run()
    {
        GLUTBackendRun(this);
    }
    

    virtual void RenderSceneCB()
    {   
        CalcFPS();
        
        m_scale += 0.05f;

        m_pGameCamera->OnRender();

		m_gbuffer.StartFrame();

        DSGeometryPass();

		// We need stencil to be enabled in the stencil pass to get the stencil buffer
		// updated and we also need it in the light pass because we render the light
		// only if the stencil passes.
		glEnable(GL_STENCIL_TEST);


		//对于每个光源我们都要先执行模板阶段（标记相关像素），紧接着根据模板阶段的结果针对这个光源进行光照计算。
		//这里我们将每个光源分开计算是因为当某个像素的模板值大于 0 时，我们无法确定其是因为哪一个光源导致的，所以我们需要分开计算。

		//在我们对每个光源的光照计算结束之后先禁用模板测试，因为对于平行光源来说我们需要对每个像素都进行光照计算。
		for (unsigned int i = 0 ; i < ARRAY_SIZE_IN_ELEMENTS(m_pointLight); i++) {
			DSStencilPass(i);
			DSPointLightPass(i);
		}

		// The directional light does not need a stencil test because its volume
		// is unlimited and the final pass simply copies the texture.
		glDisable(GL_STENCIL_TEST);

		DSDirectionalLightPass();

		//为什么在 GBuffer 中添加一个颜色缓存来存放渲染结果而不是直接将图像绘制到屏幕上。
		//因为 GBuffer 中包含了场景的属性数据和深度/模板缓存，是一个整体。
		//当进入点光源阶段之后我们需要从深度缓存中获取数据，这样就产生了一个问题——如果直接将图像渲染到默认的 FBO 中那么我们就没有权限去访问 GBuffer 中的深度缓存。
		//但是 GBuffer 必须拥有自己的深度缓存因为当我们想 GBuffer 中渲染场景时我们也无法访问默认 FBO 的深度缓存。因此解决方案就是在 GBuffer 的 FBO 中添加一个颜色缓存来存放最终渲染的结果，
		//并在 FinalPass 中通过 Blit 操作将这个颜色缓存中的图像拷贝到默认 FBO 的颜色缓存中。
		DSFinalPass();
        
        RenderFPS();
        
        glutSwapBuffers();
    }
    

    void DSGeometryPass()
    {
		m_DSGeomPassTech.Enable();

        m_gbuffer.BindForGeomPass();

		// Only the geometry pass updates the depth buffer
		glDepthMask(GL_TRUE);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glEnable(GL_DEPTH_TEST);

		Pipeline p;
        p.SetCamera(m_pGameCamera->GetPos(), m_pGameCamera->GetTarget(), m_pGameCamera->GetUp());
        p.SetPerspectiveProj(m_persProjInfo);        
        p.Rotate(0.0f, m_scale, 0.0f);
        
        for (unsigned int i = 0 ; i < ARRAY_SIZE_IN_ELEMENTS(m_boxPositions) ; i++) {
            p.WorldPos(m_boxPositions[i]);
            m_DSGeomPassTech.SetWVP(p.GetWVPTrans());
        	m_DSGeomPassTech.SetWorldMatrix(p.GetWorldTrans());
            m_box.Render();            
        }

		// When we get here the depth buffer is already populated and the stencil pass
		// depends on it, but it does not write to it.
		glDepthMask(GL_FALSE);		
    }

	//虽然在之后的点光源阶段我们会再次禁用深度测试，但是这里由于模板操作依赖于深度测试，所以我们还需要它。
	//之后我们禁用背面剔除，因为对于光源体来说，我们希望对其正反面都进行处理。
	//在这之后我们清空模板缓存并将模板测试设置为 “始终通过”，并根据 “背景” 中的描述为其设置模板操作。
	//之后的操作就很平常了——我们根据光源的参数渲染光源的包围球，渲染完成之后模板缓存中只有被真正处于光源体内部的对象所覆盖的像素才会存放正值。
	//这样我们就可以对这些像素进行光照计算了。
	void DSStencilPass(unsigned int PointLightIndex)
	{
		m_nullTech.Enable();

		// Disable color/depth write and enable stencil
		m_gbuffer.BindForStencilPass();
		glEnable(GL_DEPTH_TEST);

        glDisable(GL_CULL_FACE);

		glClear(GL_STENCIL_BUFFER_BIT);


		//将模板测试的结果设置为始终成功，我们真正需要关注的是模板操作；
		//配置反面的模板操作，当反面的深度测试失败时使模板缓存中的值加一，当深度测试或者模板测试成功时，保持模板缓存中的值不变；
		//配置正面的模板操作，当正面的深度测试失败时使模板缓存中的值减一，当深度测试或者模板测试成功时，保持模板缓存中的值不变；

		// We need the stencil test to be enabled but we want it
		// to succeed always. Only the depth test matters.
		glStencilFunc(GL_ALWAYS, 0, 0);

		glStencilOpSeparate(GL_BACK, GL_KEEP, GL_INCR_WRAP, GL_KEEP);
		glStencilOpSeparate(GL_FRONT, GL_KEEP, GL_DECR_WRAP, GL_KEEP);

		Pipeline p;
		p.WorldPos(m_pointLight[PointLightIndex].Position);
        float BBoxScale = CalcPointLightBSphere(m_pointLight[PointLightIndex]);
		p.Scale(BBoxScale, BBoxScale, BBoxScale);		
        p.SetCamera(m_pGameCamera->GetPos(), m_pGameCamera->GetTarget(), m_pGameCamera->GetUp());
        p.SetPerspectiveProj(m_persProjInfo);

		m_nullTech.SetWVP(p.GetWVPTrans());
		m_bsphere.Render();  

		//之后的操作就很平常了——我们根据光源的参数渲染光源的包围球，渲染完成之后模板缓存中只有被真正处于光源体内部的对象所覆盖的像素才会存放正值。这样我们就可以对这些像素进行光照计算了。
	}

    
    void DSPointLightPass(unsigned int PointLightIndex)
    {
		m_gbuffer.BindForLightPass();

        m_DSPointLightPassTech.Enable();
        m_DSPointLightPassTech.SetEyeWorldPos(m_pGameCamera->GetPos());        

		//对模板测试进行设置——只有当模板值不等于 0 时才通过
		glStencilFunc(GL_NOTEQUAL, 0, 0xFF);
      	
		//禁用深度测试（因为在这一阶段我们不需要进行深度测试）并启用混合
		glDisable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		glBlendEquation(GL_FUNC_ADD);
		glBlendFunc(GL_ONE, GL_ONE);
        
        glEnable(GL_CULL_FACE);
        glCullFace(GL_FRONT);

        Pipeline p;
        p.WorldPos(m_pointLight[PointLightIndex].Position);
        float BBoxScale = CalcPointLightBSphere(m_pointLight[PointLightIndex]);        
		p.Scale(BBoxScale, BBoxScale, BBoxScale);		
        p.SetCamera(m_pGameCamera->GetPos(), m_pGameCamera->GetTarget(), m_pGameCamera->GetUp());
        p.SetPerspectiveProj(m_persProjInfo);               
        m_DSPointLightPassTech.SetWVP(p.GetWVPTrans());
        m_DSPointLightPassTech.SetPointLight(m_pointLight[PointLightIndex]);
        m_bsphere.Render(); 
        glCullFace(GL_BACK);
       
		glDisable(GL_BLEND);
    }
	

	void DSDirectionalLightPass()
	{
		m_gbuffer.BindForLightPass();

        m_DSDirLightPassTech.Enable();
        m_DSDirLightPassTech.SetEyeWorldPos(m_pGameCamera->GetPos());

		glDisable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		glBlendEquation(GL_FUNC_ADD);
		glBlendFunc(GL_ONE, GL_ONE);

        m_quad.Render();  

		glDisable(GL_BLEND);
	}


	void DSFinalPass()
	{
		m_gbuffer.BindForFinalPass();
        glBlitFramebuffer(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, 
                          0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, GL_COLOR_BUFFER_BIT, GL_LINEAR);
	}
	
  
	void KeyboardCB(OGLDEV_KEY OgldevKey, OGLDEV_KEY_STATE State)
	{
		switch (OgldevKey) {
		case OGLDEV_KEY_ESCAPE:
		case OGLDEV_KEY_q:
			GLUTBackendLeaveMainLoop();
			break;
		default:
			m_pGameCamera->OnKeyboard(OgldevKey);
		}
	}


	virtual void PassiveMouseCB(int x, int y)
	{
		m_pGameCamera->OnMouse(x, y);
	}
    
    
private:

    // The calculation solves a quadratic equation (see http://en.wikipedia.org/wiki/Quadratic_equation)
    float CalcPointLightBSphere(const PointLight& Light)
    {
        float MaxChannel = fmax(fmax(Light.Color.x, Light.Color.y), Light.Color.z);
        
        float ret = (-Light.Attenuation.Linear + sqrtf(Light.Attenuation.Linear * Light.Attenuation.Linear - 4 * Light.Attenuation.Exp * (Light.Attenuation.Exp - 256 * MaxChannel * Light.DiffuseIntensity))) 
                    /
                    (2 * Light.Attenuation.Exp);
        
        return ret;
    }    
        
    void InitLights()
    {
        m_spotLight.AmbientIntensity = 0.0f;
        m_spotLight.DiffuseIntensity = 0.9f;
		m_spotLight.Color = COLOR_WHITE;
        m_spotLight.Attenuation.Linear = 0.01f;
        m_spotLight.Position  = Vector3f(-20.0, 20.0, 5.0f);
        m_spotLight.Direction = Vector3f(1.0f, -1.0f, 0.0f);
        m_spotLight.Cutoff =  20.0f;

		m_dirLight.AmbientIntensity = 0.1f;
		m_dirLight.Color = COLOR_CYAN;
		m_dirLight.DiffuseIntensity = 0.5f;
		m_dirLight.Direction = Vector3f(1.0f, 0.0f, 0.0f);

		m_pointLight[0].DiffuseIntensity = 0.2f;
		m_pointLight[0].Color = COLOR_GREEN;
        m_pointLight[0].Position = Vector3f(0.0f, 1.5f, 5.0f);
		m_pointLight[0].Attenuation.Constant = 0.0f;
        m_pointLight[0].Attenuation.Linear = 0.0f;
        m_pointLight[0].Attenuation.Exp = 0.3f;

		m_pointLight[1].DiffuseIntensity = 0.2f;
		m_pointLight[1].Color = COLOR_RED;
        m_pointLight[1].Position = Vector3f(2.0f, 0.0f, 5.0f);
		m_pointLight[1].Attenuation.Constant = 0.0f;
        m_pointLight[1].Attenuation.Linear = 0.0f;
        m_pointLight[1].Attenuation.Exp = 0.3f;
        
		m_pointLight[2].DiffuseIntensity = 0.2f;
		m_pointLight[2].Color = COLOR_BLUE;
        m_pointLight[2].Position = Vector3f(0.0f, 0.0f, 3.0f);
		m_pointLight[2].Attenuation.Constant = 0.0f;
        m_pointLight[2].Attenuation.Linear = 0.0f;        
        m_pointLight[2].Attenuation.Exp = 0.3f;
    }
    
      
    void InitBoxPositions()
    {
        m_boxPositions[0] = Vector3f(0.0f, 0.0f, 5.0f);
        m_boxPositions[1] = Vector3f(6.0f, 1.0f, 10.0f);
        m_boxPositions[2] = Vector3f(-5.0f, -1.0f, 12.0f);
        m_boxPositions[3] = Vector3f(4.0f, 4.0f, 15.0f);
        m_boxPositions[4] = Vector3f(-4.0f, 2.0f, 20.0f);
    }
    
	DSGeomPassTech m_DSGeomPassTech;
	DSPointLightPassTech m_DSPointLightPassTech;
    DSDirLightPassTech m_DSDirLightPassTech;
    NullTechnique m_nullTech;
    Camera* m_pGameCamera;
    float m_scale;
    SpotLight m_spotLight;
	DirectionalLight m_dirLight;
	PointLight m_pointLight[3];
    BasicMesh m_box;
    BasicMesh m_bsphere;
    BasicMesh m_quad;
    PersProjInfo m_persProjInfo;
    GBuffer m_gbuffer;
    Vector3f m_boxPositions[5];
};


int main(int argc, char** argv)
{
    GLUTBackendInit(argc, argv, true, false);

    if (!GLUTBackendCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, false, "Tutorial 37")) {
        return 1;
    }
    
    Tutorial37* pApp = new Tutorial37();

    if (!pApp->Init()) {
        return 1;
    }       
    
    pApp->Run();

    delete pApp;
 
    return 0;
}