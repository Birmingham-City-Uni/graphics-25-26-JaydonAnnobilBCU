#include <iostream>
#include <lodepng.h>

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

            imageBuffer[pixelIdx * nChannels + 0] = 0;
            imageBuffer[pixelIdx * nChannels + 1] = 0;
            imageBuffer[pixelIdx * nChannels + 2] = 0;
            imageBuffer[pixelIdx * nChannels + 3] = 255;
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

                imageBuffer[pixelIdx * nChannels + 0] = 255;
                imageBuffer[pixelIdx * nChannels + 1] = 255;
                imageBuffer[pixelIdx * nChannels + 2] = 255;
                imageBuffer[pixelIdx * nChannels + 3] = 255;
            }
        };

    // Draw the three edges
    drawLine(x1, y1, x2, y2);
    drawLine(x2, y2, x3, y3);
    drawLine(x3, y3, x1, y1);

    // Save the image
    int errorCode;
        errorCode = lodepng::encode(outputFilename, imageBuffer, width, height);
        if (errorCode) { // check the error code, in case an error occurred.
            std::cout << "lodepng error encoding image: " << lodepng_error_text(errorCode) << std::endl;
            return errorCode;
        }

    return 0;
}
