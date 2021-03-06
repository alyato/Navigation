#pragma once
#include "stdafx.h"
#include <streams.h>
#include "TrackingAlgUIDs.h"
#include "TrackingAlg.h"

const static int CheckSoftDogInterval = 25*5;

class CTrackingAlgInputPin : public CTransInPlaceInputPin
{
public:
	CTrackingAlgInputPin(TCHAR *pObjectName, CTransInPlaceFilter *pTransInPlaceFilter, HRESULT *phr, LPCTSTR pName)
		: CTransInPlaceInputPin(pObjectName, pTransInPlaceFilter, phr, pName)
	{

	}

	HRESULT CheckMediaType(const CMediaType *pmt);
};

class CTrackingAlgOutputPin : public CTransInPlaceOutputPin
{
public:
	CTrackingAlgOutputPin(TCHAR *pObjectName, CTransInPlaceFilter *pTransInPlaceFilter, HRESULT *phr, LPCTSTR pName)
		: CTransInPlaceOutputPin(pObjectName, pTransInPlaceFilter, phr, pName)
	{

	}

	HRESULT CheckMediaType(const CMediaType *pmt);
};

class CTrackingAlgFilter : public CTransInPlaceFilter, public ITrackingControl
{
	friend class CTrackingAlgInputPin;
	friend class CTrackingAlgOutputPin;

public:
	static CUnknown * WINAPI CreateInstance(LPUNKNOWN punk, HRESULT *phr);

	DECLARE_IUNKNOWN;

	//
	// --- CTransInPlaceFilter Overrides --
	//

	virtual CBasePin *GetPin(int n);
	HRESULT CheckInputType(const CMediaType *mtIn)
	{
		UNREFERENCED_PARAMETER(mtIn); return S_OK;
	}

	STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void ** ppv);

	HRESULT SetResolution(LONG biWidht, LONG biHeight);
	LONG GetWidth();
	LONG GetHeight();

	//
	// --- ITrackingControl Implement --
	//
	STDMETHODIMP StartTracking(BOOL bShowTrackingRes);
	STDMETHODIMP StopTracking();
	STDMETHODIMP ConfigTrackingArea(int beginX, int beginY, int beginWidth,  int beginHeight,
		                            int stopX,  int stopY,  int stopWidth,  int stopHeight);
	STDMETHODIMP ConfigHuman( int leastHumanGap,  int humanWidth);
	STDMETHODIMP ConfigVariousThresh( int disappearFrameThresh,  int centerWeightThresh,  int fgLowThresh, 
		                              int fgHighThresh,  double fgHistThresh);
	STDMETHODIMP ConfigMiscellaneous( double gbmLearningRate,  int trackInterval);

	STDMETHODIMP CacheAndShowBZoneVertex(int xPix, int yPix);
	STDMETHODIMP EraseCachedVertexes();
	STDMETHODIMP AddBZone(int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4);
	STDMETHODIMP ClearBlindZones();
	STDMETHODIMP SetFullScrStrategy(int fullScrMinDur, int noPersonMaxDur);

private:

	CTrackingAlgFilter(TCHAR *tszName, LPUNKNOWN punk, HRESULT *phr);

	HRESULT Transform(IMediaSample *pSample);
	HRESULT checkSoftDog();
	BOOL m_bSoftDogChecked;
	int m_nFrameCount;

	static int m_nInstanceCount;
	int m_nThisInstance;

	CMediaType m_mtPreferred;
	CCritSec m_TransAlgLock;

	LONG m_biWidth;
	LONG m_biHeight;

	TrackingAlg *m_pTrackingAlg;
	BOOL m_bTracking;

	void *m_pPosAnalyzer;

};


