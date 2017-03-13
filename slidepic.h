#ifndef SLIDEPIC_H
#define SLIDEPIC_H

#include <SFML/Graphics.hpp>


class SlidePic : public sf::Sprite
{
public:
    SlidePic();
    void update();
    void prepare_slide(int screen_res_x, int screen_res_y, int pic_width, int pic_height, int frames_to_show);
private:
    float speed_x;
    float speed_y;
    float scale_speed;
    float init_pos_x;
    float init_pos_y;
    float init_scale;
    std::string filename;
};

#endif // SLIDEPIC_H
