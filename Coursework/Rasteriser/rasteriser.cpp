#include <iostream>
#include <algorithm>
#include <lodepng.h>


// --- draw pixels --
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

// --- re usable object --
void fillTriangle(std::vector<uint8_t>& imageBuffer,
    int width, int height, int nChannels,
    int x1, int y1, int x2, int y2, int x3, int y3)
{
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

            if ((w0 >= 0 && w1 >= 0 && w2 >= 0) ||
                (w0 <= 0 && w1 <= 0 && w2 <= 0))
            {
                drawPixel(imageBuffer, width, height, nChannels,
                    x, y, 255, 255, 255);
            }
        }
    }
}

// --- main loop --
int main()
{
	std::string outputFilename = "output.png";

	const int width = 1920, height = 1080;
	const int nChannels = 4;

	// Set up an image buffer
	std::vector<uint8_t> imageBuffer(height*width*nChannels);

    // Fill the background black
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            int pixelIdx = x + y * width;

            drawPixel(imageBuffer, width, height, nChannels, x, y, 0, 0, 0);
        }
    }

    // --- drawing two triangles --
    fillTriangle(imageBuffer, width, height, nChannels,
        500, 250,
        900, 750,
        500, 750);

    fillTriangle(imageBuffer, width, height, nChannels,
        900, 250,
        900, 750,
        500, 250);


    // Save the image
    int errorCode;
        errorCode = lodepng::encode(outputFilename, imageBuffer, width, height);
        if (errorCode) { // check the error code, in case an error occurred.
            std::cout << "lodepng error encoding image: " << lodepng_error_text(errorCode) << std::endl;
            return errorCode;
        }

    return 0;
}
