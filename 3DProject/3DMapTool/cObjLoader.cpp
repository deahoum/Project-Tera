#include "StdAfx.h"
#include "cObjLoader.h"
#include "cMtlTex.h"
#include "cGroup.h"
#include "cTextureManager.h"
#include "StringUtil.h"
#include <numeric>

#pragma warning( disable: 4996 )

cObjLoader::cObjLoader(void) :
	m_vMin({ std::numeric_limits<float>::max( )-1,
		std::numeric_limits<float>::max( )-1,
		std::numeric_limits<float>::max( )-1}),
	m_vMax({ std::numeric_limits<float>::min( )+1,
		std::numeric_limits<float>::min( )+1,
		std::numeric_limits<float>::min( )+1})
{
}


cObjLoader::~cObjLoader(void)
{
}

void cObjLoader::Load(const char* szFullPath, std::vector<cGroup*>& vecGroup, D3DXMATRIXA16* mat /*= NULL*/)
{
	std::vector<D3DXVECTOR3> vecV;
	std::vector<D3DXVECTOR2> vecVT;
	std::vector<D3DXVECTOR3> vecVN;
	std::vector<ST_PNT_VERTEX> vecVertex;
	std::string sMtlName;

	FILE* fp = NULL;

	fopen_s(&fp, szFullPath, "r");

	assert(fp != NULL && "파일이 없습니다.");

	while (true)
	{
		if (feof(fp))
			break;

		char szBuf[1024] = { '\0', };
		fgets(szBuf, 1024, fp);

		if (strlen(szBuf) == 0)
			continue;

		//OutputDebugString(szBuf);
		if (szBuf[0] == '#')
		{
			continue;
		}
		else if (szBuf[0] == 'm')
		{
			std::string szMtlPath;
			szMtlPath.resize( 1024 );
			sscanf(szBuf, "%*s %s", &szMtlPath[0] );

			auto a = GetPathWithoutFileName( szFullPath );
			auto b = GetFileNameFromPath( szMtlPath );

			szMtlPath = 
				a+b
				;

			LoadMtlLib( szMtlPath.c_str() );
		}
		else if (szBuf[0] == 'g')
		{
			if (vecVertex.empty() == false)
			{
				if (mat)
				{
					for (size_t i = 0; i < vecVertex.size(); ++i)
					{
						D3DXVec3TransformCoord(&vecVertex[i].p, &vecVertex[i].p, mat);
						D3DXVec3TransformNormal(&vecVertex[i].n, &vecVertex[i].n, mat);
					}
				}

				cGroup* pGroup = new cGroup;
				pGroup->SetMtlTex(m_mapMtlTex[sMtlName]);
				pGroup->SetVertex(vecVertex);
				vecGroup.push_back(pGroup);

				vecVertex.clear();
			}
		}
		else if (szBuf[0] == 'v')
		{
			if (szBuf[1] == 't')
			{
				float u, v;
				sscanf(szBuf, "%*s %f %f %*f", &u, &v);
				vecVT.push_back(D3DXVECTOR2(u, v));
			}
			else if (szBuf[1] == 'n')
			{
				float x, y, z;
				sscanf(szBuf, "%*s %f %f %f", &x, &y, &z);
				vecVN.push_back(D3DXVECTOR3(x, y, z));
			}
			else
			{
				float x, y, z;
				sscanf(szBuf, "%*s %f %f %f", &x, &y, &z);
				
				m_vMin.x = std::min( m_vMin.x, x );
				m_vMin.y = std::min( m_vMin.y, y );
				m_vMin.z = std::min( m_vMin.z, z );
				
				m_vMax.x = std::max( m_vMax.x, x );
				m_vMax.y = std::max( m_vMax.y, y );
				m_vMax.z = std::max( m_vMax.z, z );

				vecV.push_back(D3DXVECTOR3(x, y, z));
			}
		}
		else if (szBuf[0] == 'u')
		{
			char szMtlName[1024];
			sscanf(szBuf, "%*s %s", szMtlName);
			sMtlName = std::string(szMtlName);
		}
		else if (szBuf[0] == 'f')
		{
			// Stored as PTN sequence
			// Obj Has no Normal vector
			if ( vecVN.size( ) == 0 )
			{
				MessageBox( GetFocus( ), 
					L"노말 벡터가 없는 Obj 파일에 대한 로딩은 지원되지 않습니다.",
					L"WARNING!", MB_OK | MB_ICONEXCLAMATION );
			}
			// Obj Has Normal vector
			else
			{
				std::vector<ST_PNT_VERTEX> parsedVertices;
				int faceVertexCount = 0;

				Parser p( szBuf );
				while ( p.Parse( ) )
				{
					if ( p.Get( )[0] == '\n' )
					{
						continue;
					}

					int aIndex[3] {-1,-1,-1};
					sscanf( p.Get( ), "%d/%d/%d", 
						&aIndex[0], &aIndex[1], &aIndex[2] );
				
					ST_PNT_VERTEX v;
					v.p = vecV[aIndex[0]-1];
					v.t = vecVT[aIndex[1]-1];
					v.n = vecVN[aIndex[2]-1];
					vecVertex.push_back( v );

					++faceVertexCount;
				}

				if ( faceVertexCount != 3 )
				{
					MessageBox( GetFocus( ),
						L"Triangle face 이외의 Obj 파일에 대한 로딩은 지원되지 않습니다.",
						L"WARNING!", MB_OK | MB_ICONEXCLAMATION );
				}

			}
		}
	}

	for (auto& iter : m_mapMtlTex)
	{
		SAFE_RELEASE(iter.second);
	}

	fclose(fp);
}

void cObjLoader::LoadMtlLib(const char* szFullPath)
{
	FILE* fp = NULL;
	fopen_s(&fp, szFullPath, "r");

	if ( !fp )
	{
		std::string str = "머테리얼 로딩에 실패했습니다. (";
		str += szFullPath;
		str += ')';

		MessageBoxA( GetFocus( ), str.c_str( ),
			"WARNING!", MB_OK | MB_ICONEXCLAMATION );
	}

	std::string sMtlName;

	while (true)
	{
		char szBuf[1024] = { '\0', };
		fgets(szBuf, 1024, fp);

		if (feof(fp))
			break;

		if (strlen(szBuf) == 0)
			continue;

		//OutputDebugString(szBuf);
		if (szBuf[0] == '#')
		{
			continue;
		}
		else if (szBuf[0] == 'n')
		{
			char szMtlName[1024];
			sscanf(szBuf, "%*s %s", szMtlName);
			sMtlName = std::string(szMtlName);
			SAFE_RELEASE(m_mapMtlTex[sMtlName]);
			m_mapMtlTex[sMtlName] = new cMtlTex;
		}
		else if (szBuf[0] == 'K')
		{
			if (szBuf[1] == 'a')
			{
				float r, g, b;
				sscanf(szBuf, "%*s %f %f %f", &r, &g, &b);

				D3DMATERIAL9& stMtl = m_mapMtlTex[sMtlName]->GetMtl();
				stMtl.Ambient.r = r;
				stMtl.Ambient.g = g;
				stMtl.Ambient.b = b;
				stMtl.Ambient.a = 1.0f;
			}
			else if (szBuf[1] == 'd')
			{
				float r, g, b;
				sscanf(szBuf, "%*s %f %f %f", &r, &g, &b);

				D3DMATERIAL9& stMtl = m_mapMtlTex[sMtlName]->GetMtl();
				stMtl.Diffuse.r = r;
				stMtl.Diffuse.g = g;
				stMtl.Diffuse.b = b;
				stMtl.Diffuse.a = 1.0f;
			}
			else if (szBuf[1] == 's')
			{
				float r, g, b;
				sscanf(szBuf, "%*s %f %f %f", &r, &g, &b);

				D3DMATERIAL9& stMtl = m_mapMtlTex[sMtlName]->GetMtl();
				stMtl.Specular.r = r;
				stMtl.Specular.g = g;
				stMtl.Specular.b = b;
				stMtl.Specular.a = 1.0f;
			}
		}
		else if (szBuf[0] == 'm')
		{
			std::string szTexPath;
			szTexPath.resize( 1024 );
			sscanf(szBuf, "%*s %s", &szTexPath[0]);

			auto a = GetPathWithoutFileName( szFullPath );
			auto b = GetFileNameFromPath( szTexPath );

			szTexPath =
				a +
				b;

			LPDIRECT3DTEXTURE9 pTexture = NULL;
			pTexture = g_pTextureManager->GetTexture(szTexPath);

			m_mapMtlTex[sMtlName]->SetTexture(pTexture);
		}
	}

	auto mtrl = m_mapMtlTex.find( sMtlName )->second->GetMtl( );

	fclose(fp);
}


#pragma warning( default: 4996 )