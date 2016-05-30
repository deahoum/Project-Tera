#include "stdafx.h"
#include "cTextureManager.h"


cTextureManager::cTextureManager()
{
}


cTextureManager::~cTextureManager()
{
	for ( auto& elem : m_mapTexture )
	{
		SAFE_RELEASE( elem.second );
	}
}

LPDIRECT3DTEXTURE9 cTextureManager::GetTexture(char* szFullPath)
{
	return GetTexture(std::string(szFullPath));
}

LPDIRECT3DTEXTURE9 cTextureManager::GetTexture(std::string szFullPath)
{
	if (m_mapTexture.find(szFullPath) == m_mapTexture.end())
	{
		LPDIRECT3DTEXTURE9 pTexture = NULL;
		HRESULT hr = D3DXCreateTextureFromFile(g_pD3DDevice, szFullPath.c_str(), &pTexture);

		if (hr == D3D_OK)
		{
			m_mapTexture[szFullPath] = pTexture;
		}

		else
		{
			return NULL;
		}
	}

	return m_mapTexture[szFullPath];
}
