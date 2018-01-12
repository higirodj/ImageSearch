#include "ImageSearchAlgorithm.h"

ImageSearchAlgorithm::ImageSearchAlgorithm(const std::string& search_mask,
        const std::string& search_image, const std::string& output_image,
        const int match_percentage, const int color_tolerance) : 
        output_img(output_image),
        percentage(match_percentage),
        tolerance(color_tolerance) {
    mask.load(search_mask);
    image.load(search_image);
    output = image;     
}

int
ImageSearchAlgorithm::getPixelIndex(const int row, const int col,
        const int width) {
    return ((row * width) + col) * 4;
}

Color
ImageSearchAlgorithm::average_background_color(const int img_row,
        const int img_col) {
    Color color_value = {0, 0, 0};
    int red = 0, blue = 0, green = 0, pixel_counter = 0;
    const std::vector<unsigned char>& buffer_mask = mask.getBuffer();
    const std::vector<unsigned char>& buffer_image = image.getBuffer();
    const int mask_width = mask.getWidth();
    const int mask_height = mask.getHeight();
    const int img_width = image.getWidth();
    for (int row = 0; (row < mask_height); row++) {
        for (int col = 0; (col < mask_width); col++) {
            int mask_index = getPixelIndex(row, col, mask_width);
            int img_index = getPixelIndex(row + img_row, col +
            img_col, img_width);
            red = static_cast<int> (buffer_mask[mask_index]);
            blue = static_cast<int> (buffer_mask[mask_index + 1]);
            green = static_cast<int> (buffer_mask[mask_index + 2]);
            int rbg_value = (red + blue + green);
            if (rbg_value == 0) {
                color_value.red += static_cast<int> (buffer_image[img_index]);
                color_value.blue += static_cast<int> (buffer_image[img_index + 1]);
                color_value.green += static_cast<int> (buffer_image[img_index + 2]);
                pixel_counter++;
            }
        }
    }
    color_value.red /= pixel_counter;
    color_value.blue /= pixel_counter;
    color_value.green /= pixel_counter;
    return color_value;
}

bool
ImageSearchAlgorithm::inside_match_region(const int row, const int col) {
    int mask_height = mask.getHeight();
    int mask_width = mask.getWidth();
    for (unsigned int k = 0; k < matched_regions.size(); k++) {
        int locX = std::get<0>(matched_regions[k]);
        int locY = std::get<1>(matched_regions[k]);
        if ((row >= locX && row <= locX + mask_height) &&
                (col >= locY && col <= locY + mask_width)) {
            return true;
        } else if ((row + mask_height >= locX &&
                row + mask_height <= locX + mask_height) &&
                (col >= locY && col <= locY + mask_width)) {
            return true;
        } else if ((row >= locX && row <= locX + mask_height) &&
                (col + mask_width >= locY && 
                col + mask_width <= locY + mask_width)) {
            return true;
        } else if ((row + mask_height >= locX &&
                row + mask_height <= locX + mask_height) &&
                (col + mask_width >= locY &&
                col + mask_width <= locY + mask_width)) {
            return true;
        }
    }
    return false;
}

bool
ImageSearchAlgorithm::check_match_helper(const std::vector<unsigned char>& buffer_image,
        const int img_index, Color avg_background_color) {
    int red_avg = avg_background_color.red;
    int blue_avg = avg_background_color.blue;
    int green_avg = avg_background_color.green;
    int red = static_cast<int>(buffer_image[img_index]);
    int blue = static_cast<int>(buffer_image[img_index + 1]);;
    int green = static_cast<int>(buffer_image[img_index + 2]);;
    if ( ((red_avg - tolerance) <= red &&
            red <= (red_avg + tolerance)) &&
        ((blue_avg - tolerance) <= blue &&
            blue <= (blue_avg + tolerance)) &&
        ((green_avg - tolerance) <= green &&
            green <= (green_avg + tolerance)) ) {
        return true;
    }
    return false;
}

void
ImageSearchAlgorithm::check_match(const int img_row, const int img_col) {
    int net_match = 0, match_pix_count = 0, mis_match_pix_count = 0,
            img_index = 0;
    const std::vector<unsigned char>& buffer_image = image.getBuffer();
    const int mask_height = mask.getHeight();
    const int mask_width = mask.getWidth();
    const int img_width = image.getWidth();
    Color avg_back_ground_color = average_background_color(img_row, img_col);
    for (int row = 0; (row < mask_height); row++) {
        for (int col = 0; (col < mask_width); col++) {
            img_index = getPixelIndex(row + img_row, col + img_col, img_width);
            if (check_match_helper(buffer_image, img_index,
                    avg_back_ground_color)) {
                match_pix_count++;
            } else {
                mis_match_pix_count++;
            }
        }
    }
    net_match = match_pix_count - mis_match_pix_count;
    if ( net_match > mask_width * mask_height * (percentage / 100.0) ) {
        matched_regions.push_back(std::make_pair(img_row, img_col));
    }
}

void
ImageSearchAlgorithm::search() {
    const int mask_height = mask.getHeight();
    const int mask_width = mask.getWidth();
    const int image_height = image.getHeight();
    const int image_width = image.getWidth();
    for (int img_row = 0; (img_row < (image_height - mask_height + 1));
            img_row++) {
        for (int img_col = 0; (img_col < (image_width - mask_width +1));
                img_col++) {
            if (!inside_match_region(img_row, img_col)) {
                check_match(img_row, img_col);
            }
        }
    }
}

void
ImageSearchAlgorithm::drawBox(PNG& png, int row, int col,
        int width, int height) {
    for (int i = 0; (i < width); i++) {
        png.setRed(row, col + i);
        png.setRed(row + height, col + i);
    }
    for (int i = 0; (i < height); i++) {
        png.setRed(row + i, col);
        png.setRed(row + i, col + width);
    }
}

void
ImageSearchAlgorithm::print() {
    for (unsigned int k = 1; k < matched_regions.size(); k++) {
        int row = std::get<0>(matched_regions[k]);
        int col = std::get<1>(matched_regions[k]);
        drawBox(output, row, col, mask.getWidth(), mask.getHeight());
        std::printf("sub-image matched at: %d %d %d %d\n" , row, col,
                row + mask.getHeight(), col + mask.getWidth());
    }
    output.write(output_img);
    std::cout << "Number of matches: " << (matched_regions.size() - 1) << std::endl;
}
