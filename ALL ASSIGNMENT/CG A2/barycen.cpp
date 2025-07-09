// Assignment description is included in lecture slides of lecture4-5. 
// Input text files for single triangle and a sphere consisted of multiple triangles is attached here. 
// You must submit your own text file of your artwork with exactly the same format as that of given data files along with your code and report.
// Start early and don't wait till the last moment to submit your assignment. No late submission is allowed.
// No excuses of internet issue / msteams  issue / visual studio or hardware issue or electricity issue will be entertained.
#include <iostream>
#include <fstream>
#include <sstream>

using namespace std;

// Structure to store vertex information (x, y coordinates)
struct Vertex {
    int x, y;
};

// Structure to store face information (triangle defined by 3 vertices and their colors)
struct Face {
    int v1, v2, v3; // Vertex indices (1-based)
    int colors[9];  // RGB colors for each vertex (3 vertices × 3 RGB values)
};

// Function to read the input file
void readInputFile(const string& filename, int& width, int& height, Vertex*& vertices, int& numVertices, Face*& faces, int& numFaces) {
    ifstream file(filename); // Open the input file
    if (!file.is_open()) {
        cerr << "Error: Could not open file " << filename << endl;
        exit(1); // Exit if the file cannot be opened
    }

    string line;
    while (getline(file, line)) { // Read the file line by line
        if (line.empty() || line[0] == '#') continue; // Skip comments and empty lines

        // Read image size (width and height)
        if (width == 0 && height == 0) {
            stringstream ss(line);
            ss >> width >> height; // Extract width and height
            continue;
        }

        // Read vertex list
        if (vertices == nullptr) {
            stringstream ss(line);
            ss >> numVertices; // Number of vertices
            vertices = new Vertex[numVertices]; // Allocate memory for vertices
            for (int i = 0; i < numVertices; ++i) {
                getline(file, line);
                stringstream ss(line);
                ss >> vertices[i].x >> vertices[i].y; // Read vertex coordinates
            }
            continue;
        }

        // Read face list
        if (faces == nullptr) {
            stringstream ss(line);
            ss >> numFaces; // Number of faces (triangles)
            faces = new Face[numFaces]; // Allocate memory for faces
            for (int i = 0; i < numFaces; ++i) {
                getline(file, line);
                stringstream ss(line);
                ss >> faces[i].v1 >> faces[i].v2 >> faces[i].v3; // Read vertex indices
                for (int j = 0; j < 9; ++j) {
                    ss >> faces[i].colors[j]; // Read RGB colors for each vertex
                }
            }
        }
    }
}

// Function to compute barycentric coordinates for a point (x, y) with respect to a triangle (a, b, c)
void computeBarycentricCoordinates(int x, int y, const Vertex& a, const Vertex& b, const Vertex& c, double& alpha, double& beta, double& gamma) {
    // Compute the area of the main triangle (abc)
    double areaABC = (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);

    // Compute the area of sub-triangles (pbc, apc, abp)
    double areaPBC = (b.x - x) * (c.y - y) - (b.y - y) * (c.x - x);
    double areaAPC = (x - a.x) * (c.y - a.y) - (y - a.y) * (c.x - a.x);
    double areaABP = (b.x - a.x) * (y - a.y) - (b.y - a.y) * (x - a.x);

    // Compute barycentric coordinates
    beta = areaAPC / areaABC;
    gamma = areaABP / areaABC;
    alpha = 1 - beta - gamma;
}

// Function to interpolate color using barycentric coordinates
void interpolateColor(double alpha, double beta, double gamma, const int* colorA, const int* colorB, const int* colorC, int* result) {
    // Interpolate RGB values using barycentric coordinates
    result[0] = static_cast<int>(alpha * colorA[0] + beta * colorB[0] + gamma * colorC[0]); // Red
    result[1] = static_cast<int>(alpha * colorA[1] + beta * colorB[1] + gamma * colorC[1]); // Green
    result[2] = static_cast<int>(alpha * colorA[2] + beta * colorB[2] + gamma * colorC[2]); // Blue
}

// Function to render a triangle on the image
void renderTriangle(int* image, int width, int height, const Vertex* vertices, const Face& face) {
    // Get the vertices of the triangle
    Vertex a = vertices[face.v1 - 1];
    Vertex b = vertices[face.v2 - 1];
    Vertex c = vertices[face.v3 - 1];

    // Get the colors for each vertex
    const int* colorA = &face.colors[0]; // RGB for vertex A
    const int* colorB = &face.colors[3]; // RGB for vertex B
    const int* colorC = &face.colors[6]; // RGB for vertex C

    // Iterate over all pixels in the image
    for (int x = 0; x < width; ++x) {
        for (int y = 0; y < height; ++y) {
            double alpha, beta, gamma;
            // Compute barycentric coordinates for the current pixel
            computeBarycentricCoordinates(x, y, a, b, c, alpha, beta, gamma);

            // If the pixel lies inside the triangle, interpolate its color
            if (alpha >= 0 && beta >= 0 && gamma >= 0) {
                int color[3];
                interpolateColor(alpha, beta, gamma, colorA, colorB, colorC, color);
                // Set the pixel color in the image
                int index = (y * width + x) * 3; // Calculate the index for the pixel
                image[index + 0] = color[0]; // Red
                image[index + 1] = color[1]; // Green
                image[index + 2] = color[2]; // Blue
            }
        }
    }
}

// Function to write the image to a .ppm file
void writePPMFile(const string& filename, int* image, int width, int height) {
    ofstream file(filename); // Open the output file
    if (!file.is_open()) {
        cerr << "Error: Could not create file " << filename << endl;
        exit(1); // Exit if the file cannot be created
    }

    // Write the PPM header
    file << "P3" << endl; // PPM format (text-based)
    file << width << " " << height << endl; // Image dimensions
    file << "255" << endl; // Maximum color value

    // Write the pixel data
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int index = (y * width + x) * 3; // Calculate the index for the pixel
            file << image[index + 0] << " " 
                 << image[index + 1] << " " 
                 << image[index + 2] << " "; // RGB values
        }
        file << endl;
    }
}

int main() {
    // Prompt the user for the input file name
    string inputFile;
    cout << "Enter the input file name: ";
    cin >> inputFile;

    // Variables to store image size, vertices, and faces
    int width = 0, height = 0;
    Vertex* vertices = nullptr;
    int numVertices = 0;
    Face* faces = nullptr;
    int numFaces = 0;

    // Read the input file
    readInputFile(inputFile, width, height, vertices, numVertices, faces, numFaces);

    // Create a blank image (1D array: width * height * 3 for RGB)
    int* image = new int[width * height * 3]{0}; // Initialize to black

    // Render each triangle
    for (int i = 0; i < numFaces; ++i) {
        renderTriangle(image, width, height, vertices, faces[i]);
    }

    // Save the output image as a .ppm file
    string outputFile = inputFile.substr(0, inputFile.find_last_of('.')) + ".ppm";
    writePPMFile(outputFile, image, width, height);
    cout << "Image saved as " << outputFile << endl;

    // Free dynamically allocated memory/ de-allocating
    delete[] image;
    delete[] vertices;
    delete[] faces;

    return 0;
}