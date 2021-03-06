// CameraController.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "CameraController.h"


// This is the constructor of a class that has been exported.
// see CameraController.h for the class definition
CCameraController::CCameraController()
{
	return;
}


int CCameraController::addCamera(int cameraId, int comNum, int baudRate)
{
	Camera *pCamera = new Camera(comNum, baudRate);
	pCamera->Open();
	cameraList[cameraId] = *pCamera;
	return S_OK;
}

int CCameraController::deleteCamera(int cameraId, Camera& camera)
{
	camera = cameraList[cameraId];
	cameraList.erase(cameraId);
	return S_OK;
}

int CCameraController::SetCameraProtocol(int cameraId, int protocol)
{
	if(cameraList.find(cameraId) != cameraList.end())
	{
		cameraList[cameraId].SetProtocol(protocol);
		return S_OK;
	}
	return E_FAIL;
}

int CCameraController::SetCameraVelocity(int cameraId, int velocity)
{
	if(cameraList.find(cameraId) != cameraList.end())
	{
		cameraList[cameraId].SetVelocity(velocity);
		return S_OK;
	}
	return E_FAIL;
}

int CCameraController::TurnLeft(int cameraId)
{
	if(cameraList.find(cameraId) != cameraList.end())
	{
		cameraList[cameraId].TurnLeft();
		return S_OK;
	}
	return E_FAIL;
}

int CCameraController::TurnRight(int cameraId)
{
	if(cameraList.find(cameraId) != cameraList.end())
	{
		cameraList[cameraId].TurnRight();
		return S_OK;
	}
	return E_FAIL;
}

int CCameraController::TurnUp(int cameraId)
{
	if(cameraList.find(cameraId) != cameraList.end())
	{
		cameraList[cameraId].TurnUp();
		return S_OK;
	}
	return E_FAIL;
}

int CCameraController::TurnDown(int cameraId)
{
	if(cameraList.find(cameraId) != cameraList.end())
	{
		cameraList[cameraId].TurnDown();
		return S_OK;
	}
	return E_FAIL;
}

int CCameraController::ZoomIn(int cameraId)
{
	if(cameraList.find(cameraId) != cameraList.end())
	{
		cameraList[cameraId].ZoomIn();
		return S_OK;
	}
	return E_FAIL;
}

int CCameraController::ZoomOut(int cameraId)
{
	if(cameraList.find(cameraId) != cameraList.end())
	{
		cameraList[cameraId].ZoomOut();
		return S_OK;
	}
	return E_FAIL;
}

int CCameraController::Stop(int cameraId)
{
	if(cameraList.find(cameraId) != cameraList.end())
	{
		cameraList[cameraId].Stop();
		return S_OK;
	}
	return E_FAIL;
}

int CCameraController::SetPreSetPos( int cameraId, int locId)
{
	if(locId < 0)
	{
		return -1;
	}
	Location loc(locId);
	if(cameraList.find(cameraId) != cameraList.end())
	{
		cameraList[cameraId].AddPreSetLocation(loc, FALSE, NULL, NULL);
		return S_OK;
	}
	return E_FAIL;
}

int CCameraController::RestorePreSetPos( int cameraId, int locId, const unsigned char *posCode, const unsigned char *focalCode )
{
	if(locId < 0)
	{
		return -1;
	}
	Location loc(locId);
	if(cameraList.find(cameraId) != cameraList.end())
	{
		cameraList[cameraId].AddPreSetLocation(loc, TRUE, posCode, focalCode);
		return S_OK;
	}
	return E_FAIL;
}

int CCameraController::RecallPreSetPos(int cameraId, int locId)
{
	if(locId < 0)
	{
		return -1;
	}
	if(cameraList.find(cameraId) != cameraList.end())
	{
		cameraList[cameraId].RecallSpecificLocation(locId);
		return S_OK;
	}
	return E_FAIL;
}

int CCameraController::ClearPreSetPos( int cameraId )
{
	if(cameraList.find(cameraId) != cameraList.end())
	{
		cameraList[cameraId].ClearPreSetLocation();
		return S_OK;
	}
	return E_FAIL;
}

int CCameraController::GetSpecificCameraLocations(int cameraId, CameraLocDict **locDict)
{
	if(cameraList.find(cameraId) != cameraList.end())
	{
		return cameraList[cameraId].GetLocationDict(locDict);
	}
	return E_FAIL;
}

int CCameraController::GetSpecificCameraLocCode( CameraLocDict *locDict, int locId, unsigned char *pos, unsigned char *focal )
{
	if(locId < 0 || locDict->find(locId) == locDict->end())
	{
		return -1;
	}
	Location loc = (*locDict)[locId];
	int cmdLen = 0;
	loc.GetCommand(pos, cmdLen);
	loc.GetFocalCommand(focal, cmdLen);
	return 0;
}

int CCameraController::TurnToSpecificLocation(int cameraId, const Location& loc)
{
	return -1; //NO IMP
}
