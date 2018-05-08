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

#include <limits.h>
#include <string.h>

#include "null_technique.h"
#include "ogldev_util.h"

//VS 只是将位置向量通过 WVP 矩阵变换到规范化空间中，FS 则什么都没有做，因为我们并不需要更新颜色缓存，需要更新的仅仅是模板缓存，所以我们仅仅是需要进入到光栅化阶段而已。
NullTechnique::NullTechnique()
{   
}

bool NullTechnique::Init()
{
    if (!Technique::Init()) {
        return false;
    }

    if (!AddShader(GL_VERTEX_SHADER, "shaders/null_technique.vs")) {
        return false;
    }

    if (!AddShader(GL_FRAGMENT_SHADER, "shaders/null_technique.fs")) {
        return false;
    }

    if (!Finalize()) {
        return false;
    }

    m_WVPLocation = GetUniformLocation("gWVP");

	if (m_WVPLocation == INVALID_UNIFORM_LOCATION) {
		return false;
	}

	return true;
}


void NullTechnique::SetWVP(const Matrix4f& WVP)
{
    glUniformMatrix4fv(m_WVPLocation, 1, GL_TRUE, (const GLfloat*)WVP.m);    
}

