#pragma once
#include "Singleton.h"

#define g_pDeviceManager cDeviceManager::Get()
#define g_pD3DDevice cDeviceManager::Get()->GetDevice()

class cDeviceManager : 
	public TSingleton<cDeviceManager>
{
public:
	LPDIRECT3DDEVICE9 GetDevice();
	void Destroy();

private:
	LPDIRECT3D9			m_pD3D;
	LPDIRECT3DDEVICE9	m_pD3DDevice;

protected:
	cDeviceManager( );
	virtual ~cDeviceManager( );
};

