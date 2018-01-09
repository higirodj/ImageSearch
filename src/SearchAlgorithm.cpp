// Copyright 2014 Julius D. Higiro

#include <omp.h>
#include <vector>
#include <iostream>
#include <string>
#include <tuple>
#include <cstdlib>
#include "PNG.h"
#include "Assert.h"

using SuperVector = std::vector<std::tuple<int, int>>;
SuperVector matchRegion;
PNG searchImage; PNG maskImage; PNG outputImage;
int srchImgHeight; int srchImgWidth;
int mskImgHeight; int mskImgWidth;
int tolerance; int percentage;
int redAvg; int greenAvg; int blueAvg;


// Draw a box around detected image
void drawBox(PNG& png, int row, int col, int width, int height) {
  // Draw horizontal lines
  for (int i = 0; (i < width); i++) {
      png.setRed(row, col + i); 
      png.setRed(row + height, col + i);
  }
  // Draw vertical lines 
  for (int i = 0; (i < height); i++) { 
       png.setRed(row + i, col); 
       png.setRed(row + i, col + width);
  }
}

int getPixelIndex(const int row, const int col, const int width) {
    return ((row * width) + col) * 4;
}

// Identify location of pixels w/ black background in mask image 
SuperVector backgroundColorInMask(PNG& maskImage) {
    int red = 0; int blue= 0; int green = 0;
    SuperVector coordinates;
    const std::vector<unsigned char>& bufferMask = maskImage.getBuffer();
    for (int row = 0; (row < mskImgHeight); row++) {
        for (int col = 0; (col < mskImgWidth); col++) {
            int index = getPixelIndex(row, col, mskImgWidth);
            red = static_cast<int>(bufferMask[index]);
            blue = static_cast<int>(bufferMask[index + 1]);
            green = static_cast<int>(bufferMask[index + 2]);
            if (red == 0 && blue == 0 && green == 0) {
               coordinates.push_back(std::make_tuple(row, col));   
            } 
        }
    }
    return coordinates;
}

// Identify location of pixels w/ white background in mask image 
SuperVector foregroundColorInMask(PNG& maskImage) {
    int red = 0; int blue= 0; int green = 0;
    SuperVector coordinates;
    const std::vector<unsigned char>& bufferMask = maskImage.getBuffer();
    for (int row = 0; (row < mskImgHeight); row++) {
        for (int col = 0; (col < mskImgWidth); col++) {
            int index = getPixelIndex(row, col, mskImgWidth);
            red = static_cast<int>(bufferMask[index]);
            blue = static_cast<int>(bufferMask[index + 1]);
            green = static_cast<int>(bufferMask[index + 2]);
            if (red != 0 && blue != 0 && green != 0) {
                coordinates.push_back(std::make_tuple(row, col));
            }
        }
    }
    return coordinates;
}

// Ensure that match regions is checked only once
bool insideMatchRegion(int row, int col) {
    for (unsigned int k = 0; k < matchRegion.size(); k++) {
        int locX = std::get<0>(matchRegion[k]);
        int locY = std::get<1>(matchRegion[k]);
        if ((row >= locX && row <= locX + mskImgHeight) && 
            (col >= locY && col <= locY + mskImgWidth)) {
            return true;  // R,C inside match region
        } else if ((row+mskImgHeight >= locX && 
            row+mskImgHeight <= locX+mskImgHeight) &&
            (col >= locY&&col <= locY+mskImgWidth)) {
            return  true;  // R+mask heiht,C inside match region
        } else if ((row >= locX && row<= locX + mskImgHeight) &&
            (col+mskImgWidth >= locY&&col+mskImgWidth <= locY+mskImgWidth)) {
            return true;  // R,C+mask width inside match region
        } else if ((row+mskImgHeight >= locX &&
             row+mskImgHeight <= locX+mskImgHeight) &&
            (col+mskImgWidth >= locY&&col+mskImgWidth <= locY+mskImgWidth)) {
            return true;  // R+mask height, C+mask width insde match region
        }   
    }
    return false;
}

// Calculate the average RGB value
void averageRGBcolor(int tot_red, int tot_blue, int tot_green) {
    SuperVector coordinatesBLK = backgroundColorInMask(maskImage);
    redAvg =  tot_red/coordinatesBLK.size();
    greenAvg = tot_green/coordinatesBLK.size();
    blueAvg = tot_blue/coordinatesBLK.size();
}

// Check if the RGB  value is a match with the foreground color
int foreGroundCheckMatch(int row, int col, int tot_red,
     int tot_green, int tot_blue) {
    averageRGBcolor(tot_red, tot_green, tot_blue); int locX = 0; int locY = 0;
    int match = 0; int red = 0; int blue = 0; int green = 0;
    const std::vector<unsigned char>& bufferSearch = searchImage.getBuffer();
    SuperVector coordinatesWHT = foregroundColorInMask(maskImage);
    for (unsigned int k = 0; k < coordinatesWHT.size(); k++) {
        locX = std::get<0>(coordinatesWHT[k]);
        locY = std::get<1>(coordinatesWHT[k]);
        int index = getPixelIndex(locX+row, locY+col, srchImgWidth);
        red = static_cast<int>(bufferSearch[index]);
        blue = static_cast<int>(bufferSearch[index + 1]);
        green = static_cast<int>(bufferSearch[index + 2]);
        if (((redAvg - tolerance) <= red &&
             red <= (redAvg + tolerance))  &&
            ((blueAvg - tolerance) <= blue &&
             blue <= (blueAvg + tolerance))  &&
            ((greenAvg - tolerance) <= green &&
             green <= (greenAvg + tolerance))) {   
        } else {
            match++;
        }    
    }
    return match;
}

// Check if the RGB value is a match with the background color
int backGroundCheckMatch(int row, int col, int tot_red,
     int tot_green, int tot_blue) {
    averageRGBcolor(tot_red, tot_green, tot_blue);
    int locX = 0; int locY = 0;
    int match = 0; int red = 0; int blue = 0; int green = 0;
    const std::vector<unsigned char>& bufferSearch = searchImage.getBuffer();
    SuperVector coordinatesBLK = backgroundColorInMask(maskImage);
    for (unsigned int k = 0; k < coordinatesBLK.size(); k++) {
        locX = std::get<0>(coordinatesBLK[k]);
        locY = std::get<1>(coordinatesBLK[k]);
        int index = getPixelIndex(locX+row, locY+col, srchImgWidth);
        red = static_cast<int>(bufferSearch[index]);
        blue = static_cast<int>(bufferSearch[index + 1]);
        green = static_cast<int>(bufferSearch[index + 2]);
        if (((redAvg - tolerance) <= red &&
             red <= (redAvg + tolerance))  &&
            ((blueAvg - tolerance) <= blue &&
             blue <= (blueAvg + tolerance))  &&
            ((greenAvg - tolerance) <= green &&
             green <= (greenAvg + tolerance))) {
                match++;
        }
    }
    return match;
}

// Calculate the net match and check if it the value
// is greater than the percentage pixel 
void checkMatch(int row, int col, int r, int g, int b) {
    int match = 0; int nonMatch; int netMatch = 0;
    match = foreGroundCheckMatch(row, col, r, g, b) 
           + backGroundCheckMatch(row, col, r, g, b);
    int totPixCount = mskImgWidth*mskImgHeight;
    nonMatch = totPixCount - match;
    netMatch = match - nonMatch;
    if (netMatch > totPixCount*(percentage/100.0)) {
        matchRegion.push_back(std::make_tuple(row, col));
    }
}

// Traverse the main image and extract pixel values for processing
void searchMainImage() {
    SuperVector crBLK = backgroundColorInMask(maskImage); int locX = 0;
    int locY = 0; int totRed = 0; int totBlue = 0; int totGreen = 0;
    const std::vector<unsigned char>& bufferSearch = searchImage.getBuffer();
    {
    for (int row = 0; (row < (srchImgHeight - mskImgHeight +1)); row++)
        for (int col = 0; (col < endIdx); col++) {
            totRed = 0; totBlue = 0; totGreen = 0; 
            if (!insideMatchRegion(row, col))     
                for (unsigned int k = 0; k < crBLK.size(); k++) {
                    locX = std::get<0>(crBLK[k]); locY = std::get<1>(crBLK[k]);
                    int index = getPixelIndex(locX+row, locY+col, srchImgWidth);
                    totRed += static_cast<int>(bufferSearch[index]);
                    totBlue += static_cast<int>(bufferSearch[index + 1]);
                    totGreen += static_cast<int>(bufferSearch[index + 2]);
                }     
            checkMatch(row, col, totRed, totBlue, totGreen);
        }   }
}

// Save image information to global variables for use throughout the program
// Don't make a habit of using global variables
void imageInformation(const std::string &imgsrch, const std::string &imgmsk) {
    maskImage.load(imgmsk);
    searchImage.load(imgsrch);
    outputImage = searchImage;
    mskImgWidth = maskImage.getWidth();
    mskImgHeight = maskImage.getHeight();
    srchImgWidth = searchImage.getWidth();
    srchImgHeight = searchImage.getHeight();
    matchRegion.push_back(std::make_tuple(srchImgHeight, srchImgHeight));
}

// Print the location of sub-image matches
void printMatches() {
  for (unsigned int k = 1; k < matchRegion.size(); k++) {
      int row = std::get<0>(matchRegion[k]);
      int col = std::get<1>(matchRegion[k]);
      drawBox(outputImage, row, col, mskImgWidth, mskImgHeight);
      std::cout << "sub-image matched at: " << row << ", " <<col;
      std::cout << ", " << row + mskImgHeight 
                << ", " << col + mskImgWidth << std::endl;
  }
}

// Print statement to user on proper input commands
void inputFormat() {
  std::cerr << "Specify one of the following 2 sets of parameters:\n";
  std::cerr << "Example #1: <Mask image> <Search image> <Output image>\n"
            << "Example #2: <Mask image> <Search image> "
            << "<Output image> <True> <Percentage> <Tolerance>\n";
}

int main(int argc, char *argv[]) {
    std::string imageSearch = argv[1]; std::string imageMask = argv[2];
    std::string resultImg = argv[3]; 
    imageInformation(imageSearch, imageMask);
    if (argc == 4 || argc ==5) {
        percentage = 32; tolerance =75;
        searchMainImage(); printMatches();
        outputImage.write(resultImg);
    } else if (argc == 6) {
        percentage = std::atoi(argv[5]);
        tolerance = 75;
        searchMainImage(); printMatches();
        outputImage.write(resultImg);
    } else if (argc == 7) {
        percentage = std::atoi(argv[5]);
        tolerance = std::atoi(argv[6]);
        searchMainImage(); printMatches();
        outputImage.write(resultImg);
    } else {
        inputFormat();
        return 1;
    }
    std::cout << "Number of matches: " << matchRegion.size()-1 << std::endl;
    return 0;
}


