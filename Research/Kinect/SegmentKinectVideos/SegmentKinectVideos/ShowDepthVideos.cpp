#include "opencv\cv.h"
#include "opencv\highgui.h"
#include "DepthMapSkt.h"
#include "DepthMapSktBinFileIO.h"
#include "GaussBGM.h"
#include <string>
#include <iostream>
using std::string;
using std::to_string;


double getColorRelationWeight(int row, int col, IplImage *pFrame, int index);
void saveImage(int, IplImage*, IplImage*,IplImage*,IplImage*);
double getDepthRelationWeight(int row, int col, IplImage *pMask, double avgDepth, int index);
void generateLikelyhood(CvMat *pProbFrMask, IplImage *pMask, IplImage *pFrame, float avgMaskDepth);
void matMultipleNum(CvMat *pMat, double num);
void reAdjustFrMask(IplImage *pMask, CvMat *pProbFrMask);
double maxLikelyhood = 0.0;
int main(int argc, char** argv)
{
	IplImage* pScaledDepthImage;
	CvCapture* pCapture = NULL;
	IplImage* pFrame = NULL;
	IplImage* pBkImg = NULL;
	IplImage* pFrImg = NULL;
	IplImage* pFrResImg = NULL;
	IplImage* pMask = NULL;
	CvMat* pProbFrMask = NULL;
	IplImage* pProbForShow = NULL;
	char videoFileName[] = "D:\\code\\Data\\cheer up\\a08_s01_e02_rgb.avi";
	char depthFileName[] = "D:\\code\\Data\\cheer up\\a08_s01_e02_depth.bin";
	int nofs = 0; //number of frames conatined in the file (each file is a video sequence of depth maps)
	int ncols = 0;
	int nrows = 0;
	FILE * fp = fopen(depthFileName, "rb");

	if(fp == NULL)
	{
		fprintf(stderr, "Can't open video file %s\n", depthFileName);
		return -1;
	}
	if(!(pCapture = cvCaptureFromFile(videoFileName)))
	{
		fprintf(stderr, "Can't open video file %s\n", videoFileName);
		return -1;
	}
	
	ReadDepthMapSktBinFileHeader(fp, nofs, ncols, nrows);

	printf("number of frames=%i\n", nofs);

	/*cvNamedWindow("Depth Image", 1);
	cvNamedWindow("Foreground", 1);
	cvNamedWindow("Segment Result", 1);
	cvMoveWindow("Depth Image", 0, 0);
	cvMoveWindow("Foreground", 0, 640);
	cvMoveWindow("Segment Result", 480, 0);*/
	
	GaussBGM *bgModel= NULL;
	int frameCount = 0;
	while(pFrame = cvQueryFrame(pCapture))
	{
		frameCount ++;
		CDepthMapSkt depthMap;
		depthMap.SetSize(ncols, nrows); //it allocates space
		//the data will be stored in <depthMap>
		ReadDepthMapSktBinFileNextFrame(fp, ncols, nrows, depthMap);

		if(frameCount == 1)
		{
			bgModel = new GaussBGM(pFrame);
			pMask = cvCreateImage(cvSize(pFrame->width, pFrame->height), IPL_DEPTH_8U, 1);
			pProbForShow = cvCreateImage(cvSize(pFrame->width, pFrame->height), IPL_DEPTH_32F, 1);
			pFrResImg = cvCreateImage(cvSize(pFrame->width, pFrame->height), IPL_DEPTH_8U, 3);
		}
		else
		{
			float scaleWidthCoeff = pFrame->width/(float)ncols;
			float scaleHeightCoeff = pFrame->height/(float)nrows;
			depthMap.ScaleSizeNN(scaleWidthCoeff, scaleHeightCoeff);
			depthMap.ShiftNN(10,30);

			CvSize scaledDepthImgSize;
			scaledDepthImgSize.width = pFrame->width;
			scaledDepthImgSize.height = pFrame->height;
			pScaledDepthImage = cvCreateImage(scaledDepthImgSize,IPL_DEPTH_8U,1);
			depthMap.convertToChar((uchar *)pScaledDepthImage->imageData);
			pMask->imageData = (char *)depthMap.GetForegroundMask();

			pProbFrMask = cvCreateMat(scaledDepthImgSize.height, scaledDepthImgSize.width, CV_32FC1);
			generateLikelyhood(pProbFrMask, pMask, pFrame, depthMap.GetAvgMaskValue());
//ForDebug
//matMultipleNum(pProbFrMask, 255.0 / maxLikelyhood);	
//cvGetImage(pProbFrMask, pProbForShow);

			reAdjustFrMask(pMask, pProbFrMask);


			memset(pFrResImg->imageData, 0, pFrame->imageSize);
			cvCopy(pFrame, pFrResImg, pMask);
			bgModel->UpdateModel(pFrame);
			pBkImg = bgModel->GetBackgroundImg();
			pFrImg = bgModel->GetForegroundImg();

			/*cvShowImage("Depth Image", pProbForShow);
			cvShowImage("Foreground", pMask);
			cvShowImage("Segment Result", pFrResImg);*/
			saveImage(frameCount, pFrame, pMask, pFrImg, pFrResImg);
			std::cout<<"frame cout:"<<frameCount<<std::endl;

			cvWaitKey(2);
			cvReleaseImage(&pScaledDepthImage);
		}

		
	}

	fclose(fp);
	fp=NULL;

	return 0;
}

void generateLikelyhood(CvMat *pProbFrMask, IplImage *pMask, IplImage *pFrame, float avgMaskDepth)
{
	double colorRelationWeights[4] = {0.0}, depthRelationWeights[4];
	maxLikelyhood = 0.0;

	for(int i=0; i<pMask->height; i++)
	{
		for(int j=0; j<pMask->width; j++)
		{
			if(i<=5 || i>=pMask->height-6 || j<=5 || j>=pMask->width-6)
			{
				cvmSet(pProbFrMask, i, j, 0.0);
				continue;
			}
			double sumOfColorRelationWeight = 0.0;
			for(int k=0; k<4; k++)
			{
				colorRelationWeights[k] = getColorRelationWeight(i, j, pFrame, k);
				depthRelationWeights[k] = getDepthRelationWeight(i,j, pMask, avgMaskDepth, k);
				sumOfColorRelationWeight += colorRelationWeights[k];
			}
			double probWeight = 0.0;
			for(int k=0; k<4; k++)
			{
				probWeight += (colorRelationWeights[k]/sumOfColorRelationWeight*depthRelationWeights[k]);
			}
			cvmSet(pProbFrMask, i, j, probWeight);
			if(probWeight > maxLikelyhood)
				maxLikelyhood = probWeight;
		}
	}
}

double getColorRelationWeight(int row, int col, IplImage *pFrame, int index)
{
	CvScalar sCenter, sSide;
	sCenter = cvGet2D(pFrame, row, col);
	switch(index)
	{
	case 0:
		sSide = cvGet2D(pFrame, row, col-6);
		break;
	case 1:
		sSide = cvGet2D(pFrame, row-6, col);
		break;
	case 2:
		sSide = cvGet2D(pFrame, row, col+6);
		break;
	case 3:
		sSide = cvGet2D(pFrame, row+6, col);
		break;
	default:
		return 0.0;
		break;
	}
	double distance = 0.0, maxDistance = sqrt(3*255*255.0);
	for(int i=0; i<3; i++)
	{
		distance = distance + (sSide.val[i] - sCenter.val[i])*(sSide.val[i] - sCenter.val[i]);
	}
	distance = sqrt(distance);
	return 1/(1+distance)/(1/(1 + maxDistance));
	
}

double getDepthRelationWeight(int row, int col, IplImage *pMask, double avgDepth, int index)
{
	uchar *maskDepthData = (uchar *)pMask->imageData;
	int neighbourDepth = 0;
	switch(index)
	{
	case 0:
		neighbourDepth = maskDepthData[row*pMask->widthStep + col-6];
		break;
	case 1:
		neighbourDepth = maskDepthData[(row-6)*pMask->widthStep + col];
		break;
	case 2:
		neighbourDepth = maskDepthData[row*pMask->widthStep + col+6];
		break;
	case 3:
		neighbourDepth = maskDepthData[(row+6)*pMask->widthStep + col];
		break;
	default:
		return 0.0;
		break;
	}
	if(neighbourDepth == 0 || neighbourDepth > avgDepth * 1.2)
		return 0.0;

	return avgDepth/neighbourDepth;
}

void matMultipleNum(CvMat *pMat, double num)
{
	for(int i=0; i< pMat->rows; i++)
		for(int j=0; j< pMat->cols;j++)
		{
			double val = cvmGet(pMat, i, j);
			val = val * num;
			cvmSet(pMat, i, j, val);
		}
	return;
}

void reAdjustFrMask(IplImage *pMask, CvMat *pProbFrMask)
{
	uchar *maskData = (uchar *)pMask->imageData;
	for(int i=0; i<pProbFrMask->rows; i++)
	{
		for(int j=0; j<pProbFrMask->cols; j++)
		{
			if(cvmGet(pProbFrMask, i, j) > maxLikelyhood*0.7)
			{
				maskData[i*pMask->widthStep+j] = 255;
			}
			else
			{
				maskData[i*pMask->widthStep+j] = 0;
			}
		}
	}
}

string resultFolder = "D:\\Navigation\\Research\\Kinect\\SegmentKinectVideos\\Data\\";
string originalFolder = "Original\\";
string depthFolder = "DepthImage\\";
string foregroundFolder = "GBMForeground\\";
string segmentFolder = "SegmentResult\\";
void saveImage(int index, IplImage *pOriginal, IplImage *pDepthImage, IplImage *pForegroundImage, IplImage *pSegmentResult)
{
	std::stringstream ssIndex;
	ssIndex << index;
	string s = resultFolder + originalFolder + ssIndex.str() + ".bmp";
	const char *fileName = s.c_str();
	cvSaveImage(fileName, pOriginal);
	s = resultFolder + depthFolder + ssIndex.str() + ".bmp";
	fileName = s.c_str();
	cvSaveImage(fileName, pDepthImage);
	s = resultFolder + foregroundFolder + ssIndex.str() + ".bmp";
	fileName = s.c_str();
	cvSaveImage(fileName, pForegroundImage);
	s = resultFolder + segmentFolder + ssIndex.str() + ".bmp";
	fileName = s.c_str();
	cvSaveImage(fileName, pSegmentResult);
}