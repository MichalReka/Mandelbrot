#define _CRT_SECURE_NO_DEPRECATE
#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif
#include <stdio.h>
#include <math.h>
#include <thread>
#include <vector>
#include <iostream>
int const noThreads = 10;
int itSum[noThreads];
using namespace std;
FILE* fp;
const int iYmax = 800;
/* screen ( integer) coordinate */
const int iXmax = 800;
/* world ( double) coordinate = parameter plane*/
const double CxMin = -2.5;
const double CxMax = 1.5;
const double CyMin = -2.0;
const double CyMax = 2.0;
/* */
unsigned char ImageMatrix[800][800][3];
/* color component ( R or G or B) is coded from 0 to 255 */
/* it is 24 bit color RGB file */
const int MaxColorComponentValue = 255;
char filename[] = "sierpinski.ppm";
char comment[] = "# ";/* comment should start with # */
/* Z=Zx+Zy*i  ;   Z0 = 0 */

class Sierpinski
{
private:
	double Cx, Cy;
	int iCurr = 0;
	int iDest = 0;
	int Iteration;
	const int IterationMax = 200;
	double Zx, Zy;
	double Zx2, Zy2; /* Zx2=Zx*Zx;  Zy2=Zy*Zy  */
	/*  */
	double PixelWidth = (CxMax - CxMin) / iXmax;
	double PixelHeight;

	/* bail-out value , radius of circle ;  */
	const double EscapeRadius = 2;
	double ER2 = EscapeRadius * EscapeRadius;
	int threadNumber;
	int iterationCount = 0;

public:
	Sierpinski(int i, int part, int num)
	{
		threadNumber = num;
		Cx = 0;
		Cy = 0;
		iCurr = i;
		iDest = iCurr + part;
		PixelHeight = (CyMax - CyMin) / iYmax;
		Zx = 0.0;
		Zy = 0.0;
		Zx2 = Zx * Zx;
		Zy2 = Zy * Zy;
		Iteration = 0;
	}
	void operator()()
	{
		for (int iY = 0;iY < iYmax;iY++)
		{
			if ((iY % noThreads + 1) == threadNumber)
			{
				Cy = CyMin + iY * PixelHeight;
				if (fabs(Cy) < PixelHeight / 2) Cy = 0.0; /* Main antenna */
				for (int iX = 0;iX < iXmax;iX++)
				{
					Cx = CxMin + iX * PixelWidth;
					/* initial value of orbit = critical point Z= 0 */
					Zx = 0.0;
					Zy = 0.0;
					Zx2 = Zx * Zx;
					Zy2 = Zy * Zy;
					/* */
					for (Iteration = 0;Iteration < IterationMax && ((Zx2 + Zy2) < ER2);Iteration++)
					{
						Zy = 2 * Zx * Zy + Cy;
						Zx = Zx2 - Zy2 + Cx;
						Zx2 = Zx * Zx;
						Zy2 = Zy * Zy;
					};
					/* compute  pixel color (24 bit = 3 bytes) */
					iterationCount = iterationCount + Iteration;

					if (Iteration == IterationMax)
					{ /*  interior of Mandelbrot set = black */
						ImageMatrix[iX][iY][0] = 0;
						ImageMatrix[iX][iY][1] = 0;
						ImageMatrix[iX][iY][2] = 0;
					}
					else
					{ /* exterior of Mandelbrot set = white */
						/*
						color[0] = 255;  Red
						color[1] = 255;  Green
						color[2] = 255; Blue
						*/
						ImageMatrix[iX][iY][0] = 255;
						ImageMatrix[iX][iY][1] = 255;
						ImageMatrix[iX][iY][2] = 255;
					};
					/*write color to the file*/
				}
			}
			
		}
		itSum[threadNumber - 1] = iterationCount;
	}
};
void divideWork()
{
	//watki robia sie rekurencyjnie - trzeba to wystartowac
	vector<thread> threadsCon;
	int curr;
	int part = (int)(iYmax / noThreads);
	for (int i = 0;i < noThreads;i++)
	{
		curr = i * part;
		if (noThreads == (i + 1))
		{
			part = iYmax - curr;
		}
		//thread tempT;
		Mandelbrot tempF(curr, part, i+1);
		//tempT = thread(tempF);
		threadsCon.push_back(thread(tempF));
	}
	//przepisywanie
	for (int i = 0;i < iYmax;i++)
	{
		for (int j = 0;j < iXmax;j++)
		{
			fwrite(ImageMatrix[j][i], 1, 3,fp);
		}
	}
}
int main()
{
	/*create new file,give it a name and open it in binary mode  */
	fp = fopen(filename, "wb"); /* b -  binary mode */
	/*write ASCII header to the file*/
	fprintf(fp, "P6\n %s\n %d\n %d\n %d\n", comment, iXmax, iYmax, MaxColorComponentValue);
	/* compute and write image data bytes to the file*/
	divideWork();
	fclose(fp);
	return 0;
}