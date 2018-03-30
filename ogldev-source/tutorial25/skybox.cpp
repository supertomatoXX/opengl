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
*/

#include "skybox.h"
#include "ogldev_pipeline.h"
#include "ogldev_util.h"

SkyBox::SkyBox(const Camera* pCamera,
               const PersProjInfo& p)
{
    m_pCamera = pCamera;
    m_persProjInfo = p;            
    
    m_pSkyboxTechnique = NULL;
    m_pCubemapTex = NULL;
    m_pMesh = NULL;
}


SkyBox::~SkyBox()
{
    SAFE_DELETE(m_pSkyboxTechnique);
    SAFE_DELETE(m_pCubemapTex);
    SAFE_DELETE(m_pMesh);
}


bool SkyBox::Init(const string& Directory,
                  const string& PosXFilename,
                  const string& NegXFilename,
                  const string& PosYFilename,
                  const string& NegYFilename,
                  const string& PosZFilename,
                  const string& NegZFilename)
{
    m_pSkyboxTechnique = new SkyboxTechnique();

    if (!m_pSkyboxTechnique->Init()) {
        printf("Error initializing the skybox technique\n");
        return false;
    }

    m_pSkyboxTechnique->Enable();
    m_pSkyboxTechnique->SetTextureUnit(0);
    
    m_pCubemapTex = new CubemapTexture(Directory,
                                       PosXFilename,
                                       NegXFilename,
                                       PosYFilename,
                                       NegYFilename,
                                       PosZFilename,
                                       NegZFilename);

    if (!m_pCubemapTex->Load()) {
        return false;
    }
        
    m_pMesh = new Mesh();

    return m_pMesh->LoadMesh("../Content/sphere.obj"); 
}


void SkyBox::Render()
{
	//我们想在不影响其他部分的代码的前提下那样做。一个办法就是取出现在的状态，然后进行想要的改变，并且最后要将原本的状态值还原，这样其他系统就不需要知道这些状态的改变了。
    m_pSkyboxTechnique->Enable();
    
    GLint OldCullFaceMode;
    glGetIntegerv(GL_CULL_FACE_MODE, &OldCullFaceMode);
    GLint OldDepthFuncMode;
    glGetIntegerv(GL_DEPTH_FUNC, &OldDepthFuncMode);
    
    glCullFace(GL_FRONT);

	//深度测试函数模式。默认的，我们是告诉OpenGL，输入的片元如果比存储的片元Z值小就认为赢得深度测试而被渲染，但是对于天空盒子，Z值总是最远的边界，
	//如果深度测试函数模式设置为‘小于’，天空盒子会被裁剪掉，为了让盒子成为场景的一部分我们要将深度测试函数模式改为‘小于等于’。
    glDepthFunc(GL_LEQUAL);

    Pipeline p;    
    p.Scale(20.0f, 20.0f, 20.0f);
    p.Rotate(0.0f, 0.0f, 0.0f);

	//对于天空盒子来说，世界坐标系的中心位于相机处，从而保证相机始终在天空盒子中心。
	//之后cubemap的纹理贴图绑定到纹理单元0号上(天空盒子着色器初始化时设置的也是0号纹理单元)，然后球面网格被渲染，最后原本的剔除模式和深度测试函数被还原。
    p.WorldPos(m_pCamera->GetPos().x, m_pCamera->GetPos().y, m_pCamera->GetPos().z);
    p.SetCamera(m_pCamera->GetPos(), m_pCamera->GetTarget(), m_pCamera->GetUp());
    p.SetPerspectiveProj(m_persProjInfo);
    m_pSkyboxTechnique->SetWVP(p.GetWVPTrans());
    m_pCubemapTex->Bind(GL_TEXTURE0);
    m_pMesh->Render();  
    
    glCullFace(OldCullFaceMode);        
    glDepthFunc(OldDepthFuncMode);
}