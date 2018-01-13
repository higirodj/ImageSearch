/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   ImageSearchAlgorithm.h
 * Author: Julius Higiro
 *
 * Created on January 9, 2018, 2:13 PM
 */

#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>
#include <tuple>
#include "omp.h"
#include "PNG.h"
#include "Assert.h"

#ifndef IMAGESEARCHALGORITHM_H
#define IMAGESEARCHALGORITHM_H

struct Color {
    int red;
    int blue;
    int green;
};

class ImageSearchAlgorithm {
public:

    /**
     * Constructor
     * @param search_mask: sub-image (mask) to be searched for in the larger image.
     * @param search_image: image to search-in.
     * @param output_image: image generated by the program w/red box around occurences of sub-image.
     * @param match_percentage: percentage number of pixels that must match in order to conclude that
     * a region of the sub-image is a match to the mask.
     * @param color_tolerance: RGB values must match within a given tolerance.
     */
    ImageSearchAlgorithm(const std::string& search_mask,
            const std::string& search_image, const std::string& output_image,
            const int match_percentage, const int color_tolerance);

    /**
     * Convert row, col to an index in the flat buffer.
     * @param row: y-coordinate of image.
     * @param col: x-coordinate of image.
     * @param width: width of the image.
     * @return: index in the buffer.
     */
    int getPixelIndex(const int row, const int col, const int width);

    /**
     * Averages the color of pixels in the image corresponding to the black pixels in the sub-image.
     * @param img_row: y-coordinate of image.
     * @param img_col: x-coordinate of image.
     * @return: average background color of pixels in a given region determined by the mask.
     */
    Color average_background_color(const int img_row, const int img_col);

    /**
     * A helper function to check if an image pixel matches the sub-image.
     * @param buffer_image: flat buffer of the image.
     * @param img_index: index in the buffer.
     * @param avg_background_color: average background color of pixels in a given region.
     * @return: boolean value indicating match or mismatch.
     */
    bool check_match_helper(const std::vector<unsigned char>& buffer_image,
            const int img_index, Color avg_background_color, int rbg_value);

    /**
     * Ensures image search excludes areas that have already been identified as a matching region.
     * @param row: y-coordinate of matching region in image.
     * @param col: x-coordinate of matching region in image.
     * @return: boolean value indicating whether or not current search is occuring inside or outside matched region.
     */
    bool inside_match_region(const int row, const int col);
    
    /**
     * Determine if image pixels matches the sub-image and saves the matching regions in a data structure.
     * @param row: y-coordinate of matching region in image.
     * @param col: x-coordinate of matching region in image.
     */
    void check_match(const int row, const int col);
    
    /**
     * Searches each occurence of the sub-image in a larger image.
     */
    void search();
    
    /**
     * Draws a red box around each occurence of the sub-image in the larger image.
     * @param png: A copy of the larger image.
     * @param row: y-coordinate of matching region in image.
     * @param col: x-coordinate of matching region in image.
     * @param width: width of sub-image.
     * @param height: height of sub-image.
     */
    void drawBox(PNG& png, const int row, const int col,
            const int width, const int height);
    
    /**
     * Generates a PNG file that contains a copy of the larger image and red boxes drawn around each occurence of the sub-image.
     */
    void print();
    
private:
    PNG mask;
    PNG image;
    PNG output;
    const std::string output_img;
    const int percentage;
    const int tolerance;  
    std::vector<std::tuple<int, int>> matched_regions;

};



#endif /* IMAGESEARCHALGORITHM_H */

