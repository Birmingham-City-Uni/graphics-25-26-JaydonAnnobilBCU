#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <lodepng.h>
#include <algorithm>


// --- draws pixel into an RGBA image buffer --
void drawPixel(std::vector<uint8_t>& imageBuffer, int width, int height, int nChannels,
    int x, int y, uint8_t r, uint8_t g, uint8_t b)
{
    // --- stops crashes if it tries to draw outside image --
    if (x < 0 || x >= width || y < 0 || y >= height)
        return;

    int pixelIdx = x + y * width;

    imageBuffer[pixelIdx * nChannels + 0] = r;
    imageBuffer[pixelIdx * nChannels + 1] = g;
    imageBuffer[pixelIdx * nChannels + 2] = b;
    imageBuffer[pixelIdx * nChannels + 3] = 255;
}

// --- draw lines  --
// -- two lines that connect from two points --
// --- interpolation -- 
void drawLine(std::vector<uint8_t>& imageBuffer,
    int width, int height, int nChannels,
    int x1, int y1, int x2, int y2)
{
    for (int i = 0; i <= 1000; i++)
    {
        float t = i / 1000.0f;

        int x = x1 + t * (x2 - x1);
        int y = y1 + t * (y2 - y1);

        drawPixel(imageBuffer, width, height, nChannels,
            x, y, 255, 255, 255);
    }
}

// --- fills triangles by using barycentric edge tests --
void fillTriangle(std::vector<uint8_t>& imageBuffer,
    int width, int height, int nChannels,
    int x1, int y1, int x2, int y2, int x3, int y3)
{
    // --- calculates smallest bounding box around triangles --
    int minX = std::max(0, std::min({ x1, x2, x3 }));
    int maxX = std::min(width - 1, std::max({ x1, x2, x3 }));

    int minY = std::max(0, std::min({ y1, y2, y3 }));
    int maxY = std::min(height - 1, std::max({ y1, y2, y3 }));

    float area = (x2 - x1) * (y3 - y1) - (y2 - y1) * (x3 - x1);

    for (int y = minY; y <= maxY; y++)
    {
        for (int x = minX; x <= maxX; x++)
        {
            float w0 = (x2 - x1) * (y - y1) - (y2 - y1) * (x - x1);
            float w1 = (x3 - x2) * (y - y2) - (y3 - y2) * (x - x2);
            float w2 = (x1 - x3) * (y - y3) - (y1 - y3) * (x - x3);

            // --- draw a pixel if its inside the trinagle --
            if ((w0 >= 0 && w1 >= 0 && w2 >= 0) ||
                (w0 <= 0 && w1 <= 0 && w2 <= 0))
            {
                drawPixel(imageBuffer, width, height, nChannels,
                    x, y, 255, 255, 255);
            }
        }
    }
}


// --- stores a single 3D position from the OBJ file --
struct Vertex
{
    float x, y, z;
};

// --- stores one triangle --
struct Face
{
    int v1, v2, v3;
};

// --- convert 3D to 2D --
void projectVertex(const Vertex& v, int& sx, int& sy)
{
    float scale = 400.0f;
    float cameraZ = 40.0f;

    // --- move model away from camera --
    float z = v.z + cameraZ;

    // --- perspective divide --
    sx = int((v.x / z) * scale + 960);
    sy = int(540 - (v.y / z) * scale);
}

// --- main loop --
int main()
{
	std::string outputFilename = "output.png";
    // --- stores models geometry --
    std::vector<Vertex> vertices;
    std::vector<Face> faces;

    // --- opens the exported blender model from files --
    std::ifstream file("../../../Outlast room update2.obj");

    if (!file.is_open())
    {
        std::cout << "Failed to open OBJ file!" << std::endl;
        return 1;
    }

    std::string line;

    // --- reads the OBJ file --
    while (std::getline(file, line))
    {
        std::stringstream ss(line);
        std::string prefix;
        ss >> prefix;

        // -- reads vertex --
        if (prefix == "v")
        {
            Vertex v;
            ss >> v.x >> v.y >> v.z;
            vertices.push_back(v);
        }
        // --- reads faces --
        else if (prefix == "f")
        {
            std::vector<std::string> tokens;
            std::string token;

            while (ss >> token)
                tokens.push_back(token);

            if (tokens.size() >= 3)
            {
                auto getIndex = [](const std::string& s)
                    {
                        return std::stoi(s.substr(0, s.find('/'))) - 1;
                    };

                // --- First triangle --
                faces.push_back({
                    getIndex(tokens[0]),
                    getIndex(tokens[1]),
                    getIndex(tokens[2])
                    });

                // --- If its a quad, split it into a second triangle --
                if (tokens.size() == 4)
                {
                    faces.push_back({
                        getIndex(tokens[0]),
                        getIndex(tokens[2]),
                        getIndex(tokens[3])
                        });
                }
            }
        }
    }

    std::cout << "Loaded "
        << vertices.size() << " vertices and "
        << faces.size() << " faces." << std::endl;


	const int width = 1920, height = 1080;
	const int nChannels = 4;

	// --- allocates memory for the output image --
	std::vector<uint8_t> imageBuffer(height*width*nChannels);

    // --- Fill the background in black ---
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            int pixelIdx = x + y * width;

            drawPixel(imageBuffer, width, height, nChannels, x, y, 0, 0, 0);
        }
    }


    // --- drawing two triangles -- ///
   // fillTriangle(imageBuffer, width, height, nChannels,
       // 500, 250,
       // 900, 750,
       // 500, 750);

    // fillTriangle(imageBuffer, width, height, nChannels,
       // 900, 250,
       // 900, 750,
       // 500, 250);

    // --- draw OBJ wireframe ---
    for (const Face& face : faces)
    {
        // --- skips invalid indices ---
        if (face.v1 < 0 || face.v1 >= vertices.size() ||
            face.v2 < 0 || face.v2 >= vertices.size() ||
            face.v3 < 0 || face.v3 >= vertices.size())
        {
            continue;
        }


        Vertex a = vertices[face.v1];
        Vertex b = vertices[face.v2];
        Vertex c = vertices[face.v3];

        int ax, ay, bx, by, cx, cy;

        projectVertex(a, ax, ay);
        projectVertex(b, bx, by);
        projectVertex(c, cx, cy);

        drawLine(imageBuffer, width, height, nChannels, ax, ay, bx, by);
        drawLine(imageBuffer, width, height, nChannels, bx, by, cx, cy);
        drawLine(imageBuffer, width, height, nChannels, cx, cy, ax, ay);
    }


    // --- Save the final image ---
    int errorCode;
        errorCode = lodepng::encode(outputFilename, imageBuffer, width, height);
        if (errorCode) { // --- check the error code, in case an error occurred --
            std::cout << "lodepng error encoding image: " << lodepng_error_text(errorCode) << std::endl;
            return errorCode;
        }

    return 0;
}
