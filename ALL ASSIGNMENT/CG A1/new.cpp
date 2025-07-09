#include <iostream>
#include <fstream>
#include <string>
#include <limits>

using namespace std;

// Global variables for image data
unsigned char* redChannel = nullptr;
unsigned char* greenChannel = nullptr;
unsigned char* blueChannel = nullptr;
int imgWidth = 0;
int imgHeight = 0;
int maxColorValue = 255;
string magicNumber = "P6";

// Function prototypes
void skipComments(ifstream& file);
void freeMemory();
bool readPPM(const string& filename);
bool writePPM(const string& filename);
void applyColorFilter(int choice);
void createNegative();
void convertToGrayscale();
void subtractImages();
void combineImages();
void morphImages();
void displayMenu();

// Skip comments in PPM files
void skipComments(ifstream& file) {
    char ch;
    file >> ch;
    while (ch == '#') {
        file.ignore(numeric_limits<streamsize>::max(), '\n');
        file >> ch;
    }
    file.putback(ch);
}

// Free allocated memory
void freeMemory() {
    delete[] redChannel;
    delete[] greenChannel;
    delete[] blueChannel;
    redChannel = greenChannel = blueChannel = nullptr;
    imgWidth = imgHeight = 0;
}

// Read PPM file
bool readPPM(const string& filename) {
    ifstream file(filename, ios::binary);
    if (!file.is_open()) {
        cerr << "Error: Could not open file " << filename << endl;
        return false;
    }

    // Free any existing image data
    freeMemory();

    // Read header
    file >> magicNumber;
    if (magicNumber != "P6") {
        cerr << "Error: Not a binary PPM file (P6)" << endl;
        return false;
    }

    skipComments(file);
    file >> imgWidth >> imgHeight;
    skipComments(file);
    file >> maxColorValue;
    file.ignore(numeric_limits<streamsize>::max(), '\n');

    // Allocate memory
    int pixelCount = imgWidth * imgHeight;
    redChannel = new unsigned char[pixelCount];
    greenChannel = new unsigned char[pixelCount];
    blueChannel = new unsigned char[pixelCount];

    // Read pixel data
    for (int i = 0; i < pixelCount; i++) {
        redChannel[i] = file.get();
        greenChannel[i] = file.get();
        blueChannel[i] = file.get();
        
        if (file.fail()) {
            cerr << "Error reading pixel data at position " << i << endl;
            freeMemory();
            return false;
        }
    }

    file.close();
    return true;
}

// Write PPM file
bool writePPM(const string& filename) {
    if (!redChannel) {
        cerr << "Error: No image data to write" << endl;
        return false;
    }

    ofstream file(filename, ios::binary);
    if (!file.is_open()) {
        cerr << "Error: Could not create file " << filename << endl;
        return false;
    }

    // Write header
    file << magicNumber << "\n";
    file << imgWidth << " " << imgHeight << "\n";
    file << maxColorValue << "\n";

    // Write pixel data
    int pixelCount = imgWidth * imgHeight;
    for (int i = 0; i < pixelCount; i++) {
        file.put(redChannel[i]);
        file.put(greenChannel[i]);
        file.put(blueChannel[i]);
    }

    file.close();
    return true;
}

// Apply color filter
void applyColorFilter(int choice) {
    if (!redChannel) {
        cerr << "Error: No image loaded" << endl;
        return;
    }

    int pixelCount = imgWidth * imgHeight;
    for (int i = 0; i < pixelCount; i++) {
        switch (choice) {
            case 1: // Red
                greenChannel[i] = 0;
                blueChannel[i] = 0;
                break;
            case 2: // Green
                redChannel[i] = 0;
                blueChannel[i] = 0;
                break;
            case 3: // Blue
                redChannel[i] = 0;
                greenChannel[i] = 0;
                break;
            case 4: // Cyan
                redChannel[i] = 0;
                break;
            case 5: // Magenta
                greenChannel[i] = 0;
                break;
            case 6: // Yellow
                blueChannel[i] = 0;
                break;
            case 7: // White (no change)
                break;
            case 8: // Black
                redChannel[i] = 0;
                greenChannel[i] = 0;
                blueChannel[i] = 0;
                break;
        }
    }
}

// Create negative image
void createNegative() {
    if (!redChannel) {
        cerr << "Error: No image loaded" << endl;
        return;
    }

    int pixelCount = imgWidth * imgHeight;
    for (int i = 0; i < pixelCount; i++) {
        redChannel[i] = maxColorValue - redChannel[i];
        greenChannel[i] = maxColorValue - greenChannel[i];
        blueChannel[i] = maxColorValue - blueChannel[i];
    }
}

// Convert to grayscale
void convertToGrayscale() {
    if (!redChannel) {
        cerr << "Error: No image loaded" << endl;
        return;
    }

    int pixelCount = imgWidth * imgHeight;
    for (int i = 0; i < pixelCount; i++) {
        unsigned char gray = static_cast<unsigned char>(
            0.299 * redChannel[i] + 
            0.587 * greenChannel[i] + 
            0.114 * blueChannel[i]);
        
        redChannel[i] = gray;
        greenChannel[i] = gray;
        blueChannel[i] = gray;
    }
}

// Helper function to load second image
bool loadSecondImage(unsigned char*& red2, unsigned char*& green2, unsigned char*& blue2, int& width2, int& height2) {
    string filename;
    cout << "Enter second image filename: ";
    cin >> filename;
    
    ifstream file(filename, ios::binary);
    if (!file.is_open()) {
        cerr << "Error: Could not open file " << filename << endl;
        return false;
    }

    string magic;
    int maxVal;
    file >> magic;
    if (magic != "P6") {
        cerr << "Error: Not a binary PPM file" << endl;
        return false;
    }

    skipComments(file);
    file >> width2 >> height2;
    skipComments(file);
    file >> maxVal;
    file.ignore(numeric_limits<streamsize>::max(), '\n');

    if (width2 != imgWidth || height2 != imgHeight) {
        cerr << "Error: Image dimensions don't match" << endl;
        return false;
    }

    int pixelCount = width2 * height2;
    red2 = new unsigned char[pixelCount];
    green2 = new unsigned char[pixelCount];
    blue2 = new unsigned char[pixelCount];

    for (int i = 0; i < pixelCount; i++) {
        red2[i] = file.get();
        green2[i] = file.get();
        blue2[i] = file.get();
        
        if (file.fail()) {
            cerr << "Error reading pixel data" << endl;
            delete[] red2;
            delete[] green2;
            delete[] blue2;
            return false;
        }
    }

    file.close();
    return true;
}

// Subtract two images
void subtractImages() {
    if (!redChannel) {
        cerr << "Error: No primary image loaded" << endl;
        return;
    }

    unsigned char* red2 = nullptr, *green2 = nullptr, *blue2 = nullptr;
    int width2, height2;
    
    if (!loadSecondImage(red2, green2, blue2, width2, height2)) {
        return;
    }

    // Create result image
    string outFilename;
    cout << "Enter output filename: ";
    cin >> outFilename;
    
    ofstream outFile(outFilename, ios::binary);
    if (!outFile.is_open()) {
        cerr << "Error: Could not create output file" << endl;
        delete[] red2;
        delete[] green2;
        delete[] blue2;
        return;
    }

    // Write header
    outFile << magicNumber << "\n";
    outFile << imgWidth << " " << imgHeight << "\n";
    outFile << maxColorValue << "\n";

    // Subtract images
    int pixelCount = imgWidth * imgHeight;
    for (int i = 0; i < pixelCount; i++) {
        outFile.put(max(0, redChannel[i] - red2[i]));
        outFile.put(max(0, greenChannel[i] - green2[i]));
        outFile.put(max(0, blueChannel[i] - blue2[i]));
    }

    outFile.close();
    delete[] red2;
    delete[] green2;
    delete[] blue2;
    cout << "Subtracted image saved as " << outFilename << endl;
}

// Combine two images
void combineImages() {
    if (!redChannel) {
        cerr << "Error: No primary image loaded" << endl;
        return;
    }

    unsigned char* red2 = nullptr, *green2 = nullptr, *blue2 = nullptr;
    int width2, height2;
    
    if (!loadSecondImage(red2, green2, blue2, width2, height2)) {
        return;
    }

    // Create result image
    string outFilename;
    cout << "Enter output filename: ";
    cin >> outFilename;
    
    ofstream outFile(outFilename, ios::binary);
    if (!outFile.is_open()) {
        cerr << "Error: Could not create output file" << endl;
        delete[] red2;
        delete[] green2;
        delete[] blue2;
        return;
    }

    // Write header
    outFile << magicNumber << "\n";
    outFile << imgWidth << " " << imgHeight << "\n";
    outFile << maxColorValue << "\n";

    // Combine images (average)
    int pixelCount = imgWidth * imgHeight;
    for (int i = 0; i < pixelCount; i++) {
        outFile.put((redChannel[i] + red2[i]) / 2);
        outFile.put((greenChannel[i] + green2[i]) / 2);
        outFile.put((blueChannel[i] + blue2[i]) / 2);
    }

    outFile.close();
    delete[] red2;
    delete[] green2;
    delete[] blue2;
    cout << "Combined image saved as " << outFilename << endl;
}

// Morph between two images
void morphImages() {
    if (!redChannel) {
        cerr << "Error: No primary image loaded" << endl;
        return;
    }

    unsigned char* red2 = nullptr, *green2 = nullptr, *blue2 = nullptr;
    int width2, height2;
    
    if (!loadSecondImage(red2, green2, blue2, width2, height2)) {
        return;
    }

    int numFrames;
    cout << "Enter number of frames to generate: ";
    cin >> numFrames;
    
    if (numFrames <= 0) {
        cerr << "Error: Invalid number of frames" << endl;
        delete[] red2;
        delete[] green2;
        delete[] blue2;
        return;
    }

    int pixelCount = imgWidth * imgHeight;
    for (int frame = 0; frame <= numFrames; frame++) {
        float weight = static_cast<float>(frame) / numFrames;
        string outFilename = "morph_" + to_string(frame) + ".ppm";
        
        ofstream outFile(outFilename, ios::binary);
        if (!outFile.is_open()) {
            cerr << "Error creating frame " << frame << endl;
            continue;
        }

        // Write header
        outFile << magicNumber << "\n";
        outFile << imgWidth << " " << imgHeight << "\n";
        outFile << maxColorValue << "\n";

        // Create morphed frame
        for (int i = 0; i < pixelCount; i++) {
            outFile.put(static_cast<unsigned char>(weight * redChannel[i] + (1 - weight) * red2[i]));
            outFile.put(static_cast<unsigned char>(weight * greenChannel[i] + (1 - weight) * green2[i]));
            outFile.put(static_cast<unsigned char>(weight * blueChannel[i] + (1 - weight) * blue2[i]));
        }

        outFile.close();
        cout << "Created frame " << frame << " as " << outFilename << endl;
    }

    delete[] red2;
    delete[] green2;
    delete[] blue2;
}

// Display menu
void displayMenu() {
    cout << "\nPPM Image Processor\n";
    cout << "1. Read PPM Image\n";
    cout << "2. Write PPM Image\n";
    cout << "3. Apply Color Filter\n";
    cout << "4. Create Negative Image\n";
    cout << "5. Convert to Grayscale\n";
    cout << "6. Subtract Two Images\n";
    cout << "7. Combine Two Images\n";
    cout << "8. Morph Between Two Images\n";
    cout << "9. Exit\n";
    cout << "Enter your choice: ";
}

int main() {
    int choice;
    
    do {
        displayMenu();
        cin >> choice;
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        
        string filename;
        int colorChoice;
        
        switch (choice) {
            case 1:
                cout << "Enter input filename: ";
                getline(cin, filename);
                if (readPPM(filename)) {
                    cout << "Image loaded successfully\n";
                }
                break;
            case 2:
                cout << "Enter output filename: ";
                getline(cin, filename);
                if (writePPM(filename)) {
                    cout << "Image saved successfully\n";
                }
                break;
            case 3:
                cout << "Color options:\n";
                cout << "1. Red\n2. Green\n3. Blue\n4. Cyan\n";
                cout << "5. Magenta\n6. Yellow\n7. White\n8. Black\n";
                cout << "Enter choice: ";
                cin >> colorChoice;
                applyColorFilter(colorChoice);
                break;
            case 4:
                createNegative();
                cout << "Negative created in memory (use option 2 to save)\n";
                break;
            case 5:
                convertToGrayscale();
                cout << "Grayscale created in memory (use option 2 to save)\n";
                break;
            case 6:
                subtractImages();
                break;
            case 7:
                combineImages();
                break;
            case 8:
                morphImages();
                break;
            case 9:
                cout << "Exiting...\n";
                break;
            default:
                cout << "Invalid choice\n";
        }
    } while (choice != 9);
    
    freeMemory();
    return 0;
}