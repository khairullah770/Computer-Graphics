#include <iostream>
#include <fstream>
#include <cstdlib>
#include <string>
#include <cmath>
#include <algorithm>
#include <limits>
using namespace std;

struct ColorPixel {
	unsigned char r;
	unsigned char g;
	unsigned char b;
};

void skipWhitespaceAndComments(ifstream& fileStream) {
	char currentChar;
	while (fileStream.get(currentChar)) {
		if (isspace(currentChar)) {
			continue;
		} else if (currentChar == '#') {
			fileStream.ignore(numeric_limits<streamsize>::max(), '\n');
		} else {
			fileStream.putback(currentChar);
			break;
		}
	}
}

ColorPixel** loadPPM(const string& filePath, int& imgWidth, int& imgHeight) {
	ifstream fileInput(filePath, ios::binary);
	if (!fileInput) {
		cerr << "Error opening file: " << filePath << endl;
		exit(1);
	}

	string magicNumber;
	fileInput >> magicNumber;

	if (magicNumber != "P6") {
		cerr << "Unsupported PPM format. Expected P6: " << magicNumber << endl;
		exit(1);
	}

	skipWhitespaceAndComments(fileInput);
	fileInput >> imgWidth;

	skipWhitespaceAndComments(fileInput);
	fileInput >> imgHeight;

	skipWhitespaceAndComments(fileInput);
	int maxPixelValue;
	fileInput >> maxPixelValue;
	fileInput.ignore();

	ColorPixel** pixelMatrix = new ColorPixel*[imgHeight];
	for (int row = 0; row < imgHeight; ++row) {
		pixelMatrix[row] = new ColorPixel[imgWidth];
	}

	for (int row = 0; row < imgHeight; ++row) {
		fileInput.read(reinterpret_cast<char*>(pixelMatrix[row]), imgWidth * sizeof(ColorPixel));
		if (!fileInput) {
			cerr << "Error: Unexpected end of file" << endl;
			exit(EXIT_FAILURE);
		}
	}

	fileInput.close();
	return pixelMatrix;
}

void savePPM(const string& filePath, ColorPixel** pixelMatrix, int imgWidth, int imgHeight) {
	ofstream fileOutput(filePath, ios::binary);
	if (!fileOutput) {
		cerr << "Error: Could not open file " << filePath << endl;
		exit(EXIT_FAILURE);
	}

	fileOutput << "P6\n" << imgWidth << " " << imgHeight << "\n255\n";

	for (int row = 0; row < imgHeight; ++row) {
		fileOutput.write(reinterpret_cast<char*>(pixelMatrix[row]), imgWidth * sizeof(ColorPixel));
		if (!fileOutput) {
			cerr << "Error: Could not write pixel data to file." << endl;
			exit(EXIT_FAILURE);
		}
	}

	fileOutput.close();
}

void deallocateImage(ColorPixel** pixelMatrix, int imgHeight) {
	for (int row = 0; row < imgHeight; ++row) {
		delete[] pixelMatrix[row];
	}
	delete[] pixelMatrix;
}

void applyAffine(const float* matrix, float x, float y, float& transformedX, float& transformedY) {
	transformedX = matrix[0] * x + matrix[1] * y + matrix[2];
	transformedY = matrix[3] * x + matrix[4] * y + matrix[5];
}

void computeBoundingBox(int width, int height, float* matrix, int& outWidth, int& outHeight) {
	float origX1 = 0, origY1 = 0; //top left
	float origX2 = width - 1, origY2 = height - 1; //bottom right

	float tx1, ty1, tx2, ty2;
applyAffine(matrix, origX1, origY1, tx1, ty1);   // Top-left
applyAffine(matrix, origX2, origY1, tx2, ty1);   // Top-right
applyAffine(matrix, origX1, origY2, tx1, ty2);   // Bottom-left
applyAffine(matrix, origX2, origY2, tx2, ty2);   // Bottom-right

	float minX = min(min(tx1, tx2), min(tx1, tx2));
	float maxX = max(max(tx1, tx2), max(tx1, tx2));
	float minY = min(min(ty1, ty2), min(ty1, ty2));
	float maxY = max(max(ty1, ty2), max(ty1, ty2));

	outWidth = static_cast<int>(maxX - minX + 1);
	outHeight = static_cast<int>(maxY - minY + 1);
}

void inverseAffineTransformAndMap(ColorPixel** srcImage, int srcWidth, int srcHeight, ColorPixel** destImage, int destWidth, int destHeight, float* affineMatrix) {
	for (int row = 0; row < destHeight; ++row) {
		for (int col = 0; col < destWidth; ++col) {
			float x = col;
			float y = row;
			float originalX, originalY;

			float determinant = affineMatrix[0] * affineMatrix[4] - affineMatrix[1] * affineMatrix[3];
			if (determinant == 0) {
				continue;
			}

			float invMatrix[6] = {
				affineMatrix[4] / determinant,
				-affineMatrix[1] / determinant,
				(affineMatrix[1] * affineMatrix[5] - affineMatrix[2] * affineMatrix[4]) / determinant,
				-affineMatrix[3] / determinant,
				affineMatrix[0] / determinant,
				(affineMatrix[2] * affineMatrix[3] - affineMatrix[0] * affineMatrix[5]) / determinant
			};

			applyAffine(invMatrix, x, y, originalX, originalY);

			int srcX = static_cast<int>(originalX);
			int srcY = static_cast<int>(originalY);

			if (srcX >= 0 && srcX < srcWidth && srcY >= 0 && srcY < srcHeight) {
				destImage[row][col] = srcImage[srcY][srcX];
			} else {
				destImage[row][col].r = destImage[row][col].g = destImage[row][col].b = 0;
			}
		}
	}
}

int main() {
	string inputFileName;
	cout << "Enter the Image name: ";
	cin >> inputFileName;

	string outputFileName = "output.ppm";

	int imgWidth, imgHeight;

	ColorPixel** originalImage = loadPPM(inputFileName, imgWidth, imgHeight);
	cout << "Image read successfully from " << inputFileName << endl;

	float transformParams[6];
	cout << "Enter the 6 affine transformation parameters (a1, a2, b1, a3, a4, b2): ";
	for (int i = 0; i < 6; ++i) {
		cin >> transformParams[i];
	}

	int transformedWidth, transformedHeight;
	computeBoundingBox(imgWidth, imgHeight, transformParams, transformedWidth, transformedHeight);

	ColorPixel** transformedImage = new ColorPixel*[transformedHeight];
	for (int row = 0; row < transformedHeight; ++row) {
		transformedImage[row] = new ColorPixel[transformedWidth];
	}

	inverseAffineTransformAndMap(originalImage, imgWidth, imgHeight, transformedImage, transformedWidth, transformedHeight, transformParams);

	savePPM(outputFileName, transformedImage, transformedWidth, transformedHeight);
	cout << "Transformed image saved to " << outputFileName << endl;

	deallocateImage(originalImage, imgHeight);
	deallocateImage(transformedImage, transformedHeight);

	return 0;
}
