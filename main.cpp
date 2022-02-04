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

void MatchBlock(int xCur, int yCur, int xPos, int xNeg, int yPos, int yNeg, unsigned char*** cur, unsigned char*** pre, unsigned char*** prediction, MV *mv) {
	double MAE = 99999;
	int xMV, yMV;
	//cout << xCur << " " << yCur << endl;
	for (int i = yCur - 16 + yNeg; i < yCur + 17 - yPos; i++) {
		for (int j = xCur - 16 + xNeg; j < xCur + 17 - xPos; j++) {
			int sum = 0;
			/*cout << i << endl << j << endl;
			cout << xCur - 16 + xNeg << endl;
			cout << xCur + 16 - xPos << endl;*/
			for (int m = 0; m < 4; m++) {
				for (int n = 0; n < 4; n++) {
					sum += (int)abs(cur[0][yCur + m][xCur + n] - pre[0][i+m][j+n]);
					sum += (int)abs(cur[1][yCur + m][xCur + n] - pre[1][i+m][j+n]);
					sum += (int)abs(cur[2][yCur + m][xCur + n] - pre[2][i+m][j+n]);
				}
			}
			if (MAE > sum) {
				MAE = sum;
				yMV = i-yCur;
				xMV = j-xCur;
			}
		}
	}
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			prediction[0][yCur + i][xCur + j] = pre[0][yCur + yMV + i][xCur + xMV + j];
			prediction[1][yCur + i][xCur + j] = pre[1][yCur + yMV + i][xCur + xMV + j];
			prediction[2][yCur + i][xCur + j] = pre[2][yCur + yMV + i][xCur + xMV + j];
		}
	}
	mv->x = xMV;
	mv->y = yMV;
}

void InterPrediction(unsigned char***cur,unsigned char***pre, unsigned char*** prediction) {
	MV mv[height / block_size][width / block_size];
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
			MatchBlock(4*j, 4*i, 4 * xPos, 4 * xNeg, 4 * yPos, 4 * yNeg, cur, pre, prediction, &mv[i][j]);
		}
	}
	/*for (int i = 0; i < height / block_size; i++) {
		for (int j = 0; j < width / block_size; j++) {
			cout << mv[i][j].x << " " << mv[i][j].y << endl;
		}
	}*/
};

int main() {
	ifstream inputFile;
	inputFile.open("./Suzie_CIF_current(352x288).rgb", ios::binary);
	unsigned char*** curBuffer;
	curBuffer = new unsigned char** [3];
	for (int i = 0; i < 3; i++) {
		curBuffer[i] = new unsigned char* [height];
		for (int j = 0; j < height; j++) {
			curBuffer[i][j] = new unsigned char [width];
		}
	}
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < height; j++) {
			for (int k = 0; k < width; k++) {
				curBuffer[i][j][k]=inputFile.get();
			}
		}
	}
	inputFile.close();
	
	inputFile.open("./Suzie_CIF_previous(352x288).rgb", ios::binary);
	unsigned char*** preBuffer;
	preBuffer = new unsigned char** [3];
	for (int i = 0; i < 3; i++) {
		preBuffer[i] = new unsigned char* [height];
		for (int j = 0; j < height; j++) {
			preBuffer[i][j] = new unsigned char[width];
		}
	}
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < height; j++) {
			for (int k = 0; k < width; k++) {
				preBuffer[i][j][k] = inputFile.get();
			}
		}
	}
	inputFile.close();

	unsigned char*** prediction;
	prediction = new unsigned char** [3];
	for (int i = 0; i < 3; i++) {
		prediction[i] = new unsigned char* [height];
		for (int j = 0; j < height; j++) {
			prediction[i][j] = new unsigned char[width];
		}
	}

	InterPrediction(curBuffer,preBuffer,prediction);
	double MAE = 0;
	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			MAE += abs(curBuffer[0][i][j] - prediction[0][i][j]);
			MAE += abs(curBuffer[1][i][j] - prediction[1][i][j]);
			MAE += abs(curBuffer[2][i][j] - prediction[2][i][j]);
		}
	}
	MAE /= height * width;
	cout << MAE;

	ofstream outputFile;
	outputFile.open("./Suzie_CIF_prediction(352x288).rgb", ios::binary);
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < height; j++) {
			for (int k = 0; k < width; k++) {
				outputFile.put(prediction[i][j][k]);
			}
		}
	}
	outputFile.close();

	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < height; j++) {
			delete[] curBuffer[i][j];
		}
	}
	for (int i = 0; i < 3; i++) {
		delete[] curBuffer[i];
	}
	delete[] curBuffer;

	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < height; j++) {
			delete[] preBuffer[i][j];
		}
	}
	for (int i = 0; i < 3; i++) {
		delete[] preBuffer[i];
	}
	delete[] preBuffer;

	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < height; j++) {
			delete[] prediction[i][j];
		}
	}
	for (int i = 0; i < 3; i++) {
		delete[] prediction[i];
	}
	delete[] prediction;
}