/*

	Copyright 2010 Etay Meiri

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

#include "ogldev_pipeline.h"

const Matrix4f& Pipeline::GetProjTrans() 
{
    m_ProjTransformation.InitPersProjTransform(m_persProjInfo);
    return m_ProjTransformation;
}


const Matrix4f& Pipeline::GetVPTrans()
{
    GetViewTrans();
    GetProjTrans();
       
    m_VPtransformation = m_ProjTransformation * m_Vtransformation;
    return m_VPtransformation;
}

const Matrix4f& Pipeline::GetWorldTrans()
{
    Matrix4f ScaleTrans, RotateTrans, TranslationTrans;

    ScaleTrans.InitScaleTransform(m_scale.x, m_scale.y, m_scale.z);
    RotateTrans.InitRotateTransform(m_rotateInfo.x, m_rotateInfo.y, m_rotateInfo.z);
    TranslationTrans.InitTranslationTransform(m_worldPos.x, m_worldPos.y, m_worldPos.z);

    m_Wtransformation = TranslationTrans * RotateTrans * ScaleTrans;
    return m_Wtransformation;
}

const Matrix4f& Pipeline::GetViewTrans()
{
    Matrix4f CameraTranslationTrans, CameraRotateTrans;

	//M1*V1 = M2 * V2    v2 = M2(-1)*M1*V1   M1 = E  V2 = M2(-1)*V1  正交变货  V2=M2(T)*V1
	//E*V1 = TR*V2  V2 = (TR)(-1)*V1    V2 =  R(-1)*T(-1)*V1 = R(T)*T(-1)*V1
	//T(-1)  -Tx  -Ty -Tz
	//R(T)  行运算 还是 列运算
    CameraTranslationTrans.InitTranslationTransform(-m_camera.Pos.x, -m_camera.Pos.y, -m_camera.Pos.z);

	//列储存，转置成行储存
	//ux uy uz 0       1 0 0 -Tx        ux uy uz -U*T
	//vx vy vz 0   *   0 1 0 -Ty    =   vx vy vz -V*T
	//nx ny nz 0       0 0 1 -Tz        nx ny nz -N*T
	//0  0  0  1       0 0 0  1         0  0  0   1
    CameraRotateTrans.InitCameraTransform(m_camera.Target, m_camera.Up);
    
    m_Vtransformation = CameraRotateTrans * CameraTranslationTrans;

    return m_Vtransformation;
}

const Matrix4f& Pipeline::GetWVPTrans()
{
    GetWorldTrans();
    GetVPTrans();

    m_WVPtransformation = m_VPtransformation * m_Wtransformation;
    return m_WVPtransformation;
}


const Matrix4f& Pipeline::GetWVOrthoPTrans()
{
    GetWorldTrans();
    GetViewTrans();

    Matrix4f P;
    P.InitOrthoProjTransform(m_orthoProjInfo);
    
    m_WVPtransformation = P * m_Vtransformation * m_Wtransformation;
    return m_WVPtransformation;
}


const Matrix4f& Pipeline::GetWVTrans()
{
	GetWorldTrans();
    GetViewTrans();
	
	m_WVtransformation = m_Vtransformation * m_Wtransformation;
	return m_WVtransformation;
}


const Matrix4f& Pipeline::GetWPTrans()
{
	Matrix4f PersProjTrans;

	GetWorldTrans();
	PersProjTrans.InitPersProjTransform(m_persProjInfo);

	m_WPtransformation = PersProjTrans * m_Wtransformation;
	return m_WPtransformation;
}