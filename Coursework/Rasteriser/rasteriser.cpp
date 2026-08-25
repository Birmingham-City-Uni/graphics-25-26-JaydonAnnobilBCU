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

    // Triangle corners
    int x1 = 500, y1 = 250;
    int x2 = 900, y2 = 750;
    int x3 = 1300, y3 = 250;

    // Function to draw a line using interpolation
    auto drawLine = [&](int startX, int startY, int endX, int endY)
        {
            for (int i = 0; i <= 1000; i++)
            {
                float t = i / 1000.0f;

                int x = startX + t * (endX - startX);
                int y = startY + t * (endY - startY);

                int pixelIdx = x + y * width;

                drawPixel(imageBuffer, width, height, nChannels, x, y, 255, 255, 255);
            }
        };

    // Draw the three edges
    drawLine(x1, y1, x2, y2);
    drawLine(x2, y2, x3, y3);
    drawLine(x3, y3, x1, y1);

    // Fill the triangle using horizontal scanlines
    for (int y = y1; y <= y2; y++)
    {
        float t = float(y - y1) / float(y2 - y1);

        int leftX = x1 + t * (x2 - x1);
        int rightX = x3 + t * (x2 - x3);

        if (leftX > rightX)
            std::swap(leftX, rightX);

        for (int x = leftX; x <= rightX; x++)
        {
            drawPixel(imageBuffer, width, height, nChannels,
                x, y, 255, 255, 255);
        }
    }

    // Save the image
    int errorCode;
        errorCode = lodepng::encode(outputFilename, imageBuffer, width, height);
        if (errorCode) { // check the error code, in case an error occurred.
            std::cout << "lodepng error encoding image: " << lodepng_error_text(errorCode) << std::endl;
            return errorCode;
        }

    return 0;
}
