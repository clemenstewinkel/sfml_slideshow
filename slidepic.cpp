#include "slidepic.h"
#include <cstdlib>

SlidePic::SlidePic()
{

}

void SlidePic::update()
{
    this->move(this->speed_x , this->speed_y);
    this->scale(this->scale_speed, this->scale_speed);
}

/**
 * @brief SlidePic::prepare_slide Sets start-position of image on screen, direction and speed of movement and speed of scaling
 * @param screen_res_x
 * @param screen_res_y
 * @param pic_width
 * @param pic_height
 * @param frames_to_show
 */
void SlidePic::prepare_slide(int screen_res_x, int screen_res_y, int pic_width, int pic_height, int frames_to_show)
{
    // Two random values between -0.5 and + 0.5
    float rdx = ((float)rand() / (float)RAND_MAX) - 0.5;
    float rdy = ((float)rand() / (float)RAND_MAX) - 0.5;
    float scale_x = (float)screen_res_x / (float)pic_width;
    float scale_y = (float)screen_res_y / (float)pic_height;
    float max_scale = (scale_x > scale_y)?scale_x:scale_y;
    float min_scale = (scale_x > scale_y)?scale_y:scale_x;
    min_scale = min_scale * 1.2;
    float mid_pos_x = ((float)screen_res_x - ((float)pic_width * min_scale)) / 2;
    float mid_pos_y = ((float)screen_res_y - ((float)pic_height * min_scale)) / 2;
    this->speed_x = 0.0 + rdx/10;
    this->speed_y = 0.0 + rdy/10;
    this->scale_speed = 1.0001;
    this->setPosition(mid_pos_x,mid_pos_y);
    this->setScale(min_scale, min_scale);
}
