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
     * 
     * @param search_mask
     * @param search_image
     * @param output_image
     * @param is_mask
     * @param match_percentage
     * @param color_tolerance
     */
    ImageSearchAlgorithm(const std::string& search_mask,
            const std::string& search_image, const std::string& output_image,
            const int match_percentage, const int color_tolerance);

    /**
     * 
     * @param row
     * @param col
     * @param width
     * @return 
     */
    int getPixelIndex(const int row, const int col, const int width);

    /**
     * 
     * @param img_row
     * @param img_col
     * @return 
     */
    Color average_background_color(const int img_row, const int img_col);

    /**
     * 
     * @param buffer_image
     * @param img_index
     * @param avg_background_color
     * @return 
     */
    bool check_match_helper(const std::vector<unsigned char>& buffer_image,
            const int img_index, Color avg_background_color, int rbg_value);

    /**
     * 
     * @param row
     * @param col
     * @return 
     */
    bool inside_match_region(const int row, const int col);
    
    /**
     * 
     * @param row
     * @param col
     */
    void check_match(const int row, const int col);
    
    /**
     * 
     */
    void search();
    
    /**
     * 
     * @param png
     * @param row
     * @param col
     * @param width
     * @param height
     */
    void drawBox(PNG& png, const int row, const int col,
            const int width, const int height);
    
    /**
     *
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

