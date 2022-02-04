#pragma warning(disable:4996)
#include <iostream>
#include <cmath>
#include <fstream>
using namespace std;

#define height 288
#define width 352
#define block_size 4
#define clamp(x) (((x)<0)?0:(((x)>255)?255:(x)))

struct MV {
	int x, y;
};

void MatchBlock(int xCur, int yCur, int xPos, int xNeg, int yPos, int yNeg, unsigned char* cur, unsigned char* pre, unsigned char* prediction, MV* mv, int Cb, int Cr) {
	double MAE = 99999;
	int xMV=0, yMV=0;
	for (int i = yCur - 16 + yNeg; i < yCur + 17 - yPos; i++) {
		for (int j = xCur - 16 + xNeg; j < xCur + 17 - xPos; j++) {
			int sum = 0;
			for (int m = 0; m < 4; m++) {
				for (int n = 0; n < 4; n++) {
					sum += (int)abs(cur[(yCur + m) * width + xCur + n] - pre[(i + m) * width + j + n]);
					sum += (int)abs(cur[Cb + (yCur / 2 + m / 2) * width / 2 + xCur / 2 + n / 2] - pre[Cb + (i / 2 + m / 2) * width / 2 + j / 2 + n / 2]);
					sum += (int)abs(cur[Cr + (yCur / 2 + m / 2) * width / 2 + xCur / 2 + n / 2] - pre[Cr + (i / 2 + m / 2) * width / 2 + j / 2 + n / 2]);
					// sum += (int)abs(cur[0][yCur + m][xCur + n] - pre[0][i+m][j+n]);
					// sum += (int)abs(cur[1][yCur + m][xCur + n] - pre[1][i+m][j+n]);
					// sum += (int)abs(cur[2][yCur + m][xCur + n] - pre[2][i+m][j+n]);
				}
			}
			if (MAE > sum) {
				MAE = sum;
				yMV = i - yCur;
				xMV = j - xCur;
			}
		}
	}
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			prediction[(yCur + i) * width + xCur + j] = pre[(yCur + yMV + i) * width + xCur + xMV + j];
			if (i % 2 == 0) {
				if (j % 2 == 0) {
					prediction[Cb + (yCur / 2 + i / 2) * width / 2 + xCur / 2 + j / 2] = pre[Cb + (yCur / 2 + yMV / 2 + i / 2) * width / 2 + xCur / 2 + xMV / 2 + j / 2];
					prediction[Cr + (yCur / 2 + i / 2) * width / 2 + xCur / 2 + j / 2] = pre[Cr + (yCur / 2 + yMV / 2 + i / 2) * width / 2 + xCur / 2 + xMV / 2 + j / 2];
				}
			}
			// prediction[0][yCur + i][xCur + j] = pre[0][yCur + yMV + i][xCur + xMV + j];
			// prediction[1][yCur + i][xCur + j] = pre[1][yCur + yMV + i][xCur + xMV + j];
			// prediction[2][yCur + i][xCur + j] = pre[2][yCur + yMV + i][xCur + xMV + j];
		}
	}
	mv->x = xMV;
	mv->y = yMV;
}

void InterPrediction(unsigned char* cur, unsigned char* pre, unsigned char* prediction) {
	MV mv[height / block_size][width / block_size];
	int Cb = height * width, Cr = height * width * 5 / 4;
	for (int i = 0; i < height / block_size; i++) {
		for (int j = 0; j < width / block_size; j++) {
			int xPos = 0, xNeg = 0, yPos = 0, yNeg = 0;
			if (i < 4) {
				yNeg = 4 - i;
			}
			else if (i > height / block_size - 5) {
				yPos = i - height / block_size + 5;
			}
			if (j < 4) {
				xNeg = 4 - j;
			}
			else if (j > width / block_size - 5) {
				xPos = j - width / block_size + 5;
			}
			MatchBlock(4 * j, 4 * i, 4 * xPos, 4 * xNeg, 4 * yPos, 4 * yNeg, cur, pre, prediction, &mv[i][j], Cb, Cr);
		}
	}
};

int main() {
	ifstream inputFile;
	ofstream outputFile;
	inputFile.open("./Suzie_CIF_current(352x288).rgb", ios::binary);
	// unsigned char*** curBuffer;
	// curBuffer = new unsigned char** [3];
	// for (int i = 0; i < 3; i++) {
	// 	curBuffer[i] = new unsigned char* [height];
	// 	for (int j = 0; j < height; j++) {
	// 		curBuffer[i][j] = new unsigned char [width];
	// 	}
	// }
	// for (int i = 0; i < 3; i++) {
	// 	for (int j = 0; j < height; j++) {
	// 		for (int k = 0; k < width; k++) {
	// 			curBuffer[i][j][k]=inputFile.get();
	// 		}
	// 	}
	// }
	// inputFile.close();

	unsigned char* rgbCur = new unsigned char[height * width * 3];
	unsigned char* yuvCur = new unsigned char[height * width * 3 / 2];
	inputFile.read((char*)rgbCur, height * width * 3);
	inputFile.close();

	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			int rTemp = rgbCur[i * width + j];
			int gTemp = rgbCur[i * width + j + height * width];
			int bTemp = rgbCur[i * width + j + height * width * 2];
			yuvCur[i * width + j] = ((66 * rTemp + 129 * gTemp + 25 * bTemp + 128) >> 8) + 16;

			if (i % 2 == 0) {
				if (j % 2 == 0) {
					yuvCur[height * width + i / 2 * width / 2 + j / 2] = ((-38 * rTemp - 74 * gTemp + 112 * bTemp + 128) >> 8) + 128;
					yuvCur[height * width * 5 / 4 + i / 2 * width / 2 + j / 2] = ((112 * rTemp - 94 * gTemp - 18 * bTemp) >> 8) + 128;
				}
			}
		}
	}

	outputFile.open("./Suzie_CIF_current(352x288).yuv", ios::binary);
	outputFile.write((char*)yuvCur, height * width * 3 / 2);
	outputFile.close();

	delete[] rgbCur;



	inputFile.open("./Suzie_CIF_previous(352x288).rgb", ios::binary);
	// unsigned char*** preBuffer;
	// preBuffer = new unsigned char** [3];
	// for (int i = 0; i < 3; i++) {
	// 	preBuffer[i] = new unsigned char* [height];
	// 	for (int j = 0; j < height; j++) {
	// 		preBuffer[i][j] = new unsigned char[width];
	// 	}
	// }
	// for (int i = 0; i < 3; i++) {
	// 	for (int j = 0; j < height; j++) {
	// 		for (int k = 0; k < width; k++) {
	// 			preBuffer[i][j][k] = inputFile.get();
	// 		}
	// 	}
	// }
	// inputFile.close();

	unsigned char* rgbPre = new unsigned char[height * width * 3];
	unsigned char* yuvPre = new unsigned char[height * width * 3 / 2];
	inputFile.read((char*)rgbPre, height * width * 3);
	inputFile.close();

	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			int rTemp = rgbPre[i * width + j];
			int gTemp = rgbPre[i * width + j + height * width];
			int bTemp = rgbPre[i * width + j + height * width * 2];
			yuvPre[i * width + j] = ((66 * rTemp + 129 * gTemp + 25 * bTemp + 128) >> 8) + 16;

			if (i % 2 == 0) {
				if (j % 2 == 0) {
					yuvPre[height * width + i / 2 * width / 2 + j / 2] = ((-38 * rTemp - 74 * gTemp + 112 * bTemp + 128) >> 8) + 128;
					yuvPre[height * width * 5 / 4 + i / 2 * width / 2 + j / 2] = ((112 * rTemp - 94 * gTemp - 18 * bTemp) >> 8) + 128;
				}
			}
		}
	}

	outputFile.open("./Suzie_CIF_previous(352x288).yuv", ios::binary);
	outputFile.write((char*)yuvPre, height * width * 3 / 2);
	outputFile.close();

	delete[] rgbPre;

	unsigned char* prediction = new unsigned char[height * width * 3 / 2];


	// unsigned char*** prediction;
	// prediction = new unsigned char** [3];
	// for (int i = 0; i < 3; i++) {
	// 	prediction[i] = new unsigned char* [height];
	// 	for (int j = 0; j < height; j++) {
	// 		prediction[i][j] = new unsigned char[width];
	// 	}
	// }

	InterPrediction(yuvCur, yuvPre, prediction);
	double MAE = 0;
	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			MAE += abs(yuvCur[i * width + j] - prediction[i * width + j]);
			MAE += abs(yuvCur[height * width + i / 2 * width / 2 + j / 2] - prediction[height * width + i / 2 * width / 2 + j / 2]);
			MAE += abs(yuvCur[height * width * 5 / 4 + i / 2 * width / 2 + j / 2] - prediction[height * width * 5 / 4 + i / 2 * width / 2 + j / 2]);
			// MAE += abs(curBuffer[0][i][j] - prediction[0][i][j]);
			// MAE += abs(curBuffer[1][i][j] - prediction[1][i][j]);
			// MAE += abs(curBuffer[2][i][j] - prediction[2][i][j]);
		}
	}
	MAE /= height * width * 3 / 2;
	cout << "prediction MAE : " << MAE << endl;


	outputFile.open("./Suzie_CIF_prediction(352x288).yuv", ios::binary);
	outputFile.write((char*)prediction, height * width * 3 / 2);
	// for (int i = 0; i < 3; i++) {
	// 	for (int j = 0; j < height; j++) {
	// 		for (int k = 0; k < width; k++) {
	// 			outputFile.put(prediction[i][j][k]);
	// 		}
	// 	}
	// }
	outputFile.close();

	unsigned char* residual = new unsigned char[height * width * 3 / 2];
	for (int i = 0; i < height * width * 3 / 2; i++) {
		residual[i] = yuvCur[i] - prediction[i];
	}

	outputFile.open("./Suzie_CIF_residual(352x288).yuv", ios::binary);
	outputFile.write((char*)residual, height * width * 3 / 2);
	outputFile.close();

	unsigned char* reconstruction = new unsigned char[height * width * 3 / 2];
	for (int i = 0; i < height * width * 3 / 2; i++) {
		reconstruction[i] = residual[i] + prediction[i];
	}

	MAE = 0;
	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			MAE += abs(yuvCur[i * width + j] - reconstruction[i * width + j]);
			MAE += abs(yuvCur[height * width + i / 2 * width / 2 + j / 2] - reconstruction[height * width + i / 2 * width / 2 + j / 2]);
			MAE += abs(yuvCur[height * width * 5 / 4 + i / 2 * width / 2 + j / 2] - reconstruction[height * width * 5 / 4 + i / 2 * width / 2 + j / 2]);
		}
	}
	MAE /= height * width * 3 / 2;
	cout << "reconstruction MAE : " << MAE << endl;

	outputFile.open("./Suzie_CIF_reconstruction(352x288).yuv", ios::binary);
	outputFile.write((char*)reconstruction, height * width * 3 / 2);
	outputFile.close();

	// unsigned char* reconBuffer = new unsigned char[height * width * 3];
	// for (int i = 0; i < height; i++) {
	// 	for (int j = 0; j < width; j++) {
	// 		int cTemp = prediction[i * width + j] - 16;
	// 		int dTemp = prediction[height * width + i / 2 * width / 2 + j / 2] - 128;
	// 		int eTemp = prediction[height * width * 5 / 4 + i / 2 * width / 2 + j / 2] - 128;
	// 		reconBuffer[i * width + j] = clamp((298 * cTemp + 409 * eTemp + 128) >> 8);
	// 		reconBuffer[i * width + j + height * width] = clamp((298 * cTemp - 100 * dTemp - 208 * eTemp + 128) >> 8);
	// 		reconBuffer[i * width + j + height * width * 2] = clamp((298 * cTemp + 516 * dTemp + 128) >> 8);
	// 	}
	// }
	// outputFile.open("./recon_Suzie_CIF_current(352x288).rgb", ios::binary);
	// outputFile.write((char*)reconBuffer, height * width * 3);
	// outputFile.close();

	delete[] yuvCur;
	delete[] yuvPre;
	delete[] prediction;
	delete[] residual;
	delete[] reconstruction;
	//delete[] reconBuffer;

	// for (int i = 0; i < 3; i++) {
	// 	for (int j = 0; j < height; j++) {
	// 		delete[] curBuffer[i][j];
	// 	}
	// }
	// for (int i = 0; i < 3; i++) {
	// 	delete[] curBuffer[i];
	// }
	// delete[] curBuffer;

	// for (int i = 0; i < 3; i++) {
	// 	for (int j = 0; j < height; j++) {
	// 		delete[] preBuffer[i][j];
	// 	}
	// }
	// for (int i = 0; i < 3; i++) {
	// 	delete[] preBuffer[i];
	// }
	// delete[] preBuffer;

	// for (int i = 0; i < 3; i++) {
	// 	for (int j = 0; j < height; j++) {
	// 		delete[] prediction[i][j];
	// 	}
	// }
	// for (int i = 0; i < 3; i++) {
	// 	delete[] prediction[i];
	// }
	// delete[] prediction;
}