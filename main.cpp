#include <SFML/Graphics.hpp>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <slidepic.h>
#include <chrono>
#include <thread>



sf::Image new_image;
bool image_is_ready = false;
bool pic_load_thread_running = false;

void load_pic(std::string filename)
{
    pic_load_thread_running = true;
    if(new_image.loadFromFile(filename))
    {
        image_is_ready = true;
    }
    else
    {
        std::cout << "Could not load image " << filename << "!"  << std::endl;
    }
    pic_load_thread_running = false;
}


sf::Vector2u find_max_pic_dimension(std::vector<std::string>& pics_to_show)
{
    sf::Vector2u dim;
    sf::Image im;
    for (auto file : pics_to_show)
    {
        if(!im.loadFromFile(file))
        {
            std::cout << "Cannot load file " << file << std::endl;
            exit(1);
        }
        if (im.getSize().x > dim.x) dim.x = im.getSize().x;
        if (im.getSize().y > dim.y) dim.y = im.getSize().y;
    }
    return dim;
}

int main(int argc, char* argv[])
{
    int          opt;
    unsigned int screen_res_x     = sf::VideoMode::getDesktopMode().width;
    unsigned int screen_res_y     = sf::VideoMode::getDesktopMode().height;
    unsigned int bits_per_pixel   = sf::VideoMode::getDesktopMode().bitsPerPixel;
    unsigned int frames_per_pic   = 200;
    unsigned int frames_per_blend = 100;
    sf::Texture  text_A;
    sf::Texture  text_B;
    std::thread  pic_load_thread;
    bool         B_is_coming      = true;
    bool         blending         = false;
    bool         cycling          = false;
    bool         last_pic         = false;
    unsigned int pic_frame        = 0;
    unsigned int blend_frame      = 0;
    unsigned int current_pic      = 0;
    SlidePic     sprite_A;
    SlidePic     sprite_B;
    sf::Image    bg_image;
    sf::Vector2u text_size;
    std::string pic_dir;
    std::vector<std::string> pics_to_show;
/*
    std::vector<sf::VideoMode> possible_vid_modes =  sf::VideoMode::getFullscreenModes();
    for (auto possible_vid_mode : possible_vid_modes)
    {
        std::cout << possible_vid_mode.width << "x" << possible_vid_mode.height << ", " << possible_vid_mode.bitsPerPixel << "BitPerPixel" << std::endl;
    }
*/


    // Read command line args
    while ((opt = getopt(argc, argv, "cd:s:t:")) != -1)
    {
        switch (opt)
        {
        case 'c': cycling = true;
            break;
        case 's':
            frames_per_pic = atoi(optarg);
            break;
        case 't':
            frames_per_blend = atoi(optarg);
            break;
        case 'd':
            pic_dir = optarg;
            break;
        default: /* '?' */
            fprintf(stderr, "Usage: %s [-d <folder with pics>] pic_1 pic_2 ... pic_n\n", argv[0]);
            exit(EXIT_FAILURE);
        }
    }
    // All image-filenames to vector
    for (int i = optind; i<argc ; i++) pics_to_show.push_back(pic_dir + "/" + argv[i]);

    // Create Fullscreen-Window
    sf::RenderWindow window(sf::VideoMode(screen_res_x, screen_res_y, bits_per_pixel), "", sf::Style::Fullscreen);
    window.setVerticalSyncEnabled(true);

    // Check all given pictures for Maximum x- and y-dimensions and create two
    // textures big enough to hold every picture.
    text_size = find_max_pic_dimension(pics_to_show);
    std::cout << text_size.x << " " << text_size.y << std::endl;
    std::cout << "pics_to_show.size(): " << pics_to_show.size() << std::endl;

    text_A.create(text_size.x, text_size.y);
    text_B.create(text_size.x, text_size.y);

    bg_image.create(text_size.x, text_size.y, sf::Color(0,0,0));
    new_image.loadFromFile(pics_to_show[0]);

    text_A.update(bg_image);
    text_A.update(new_image);

    text_A.setSmooth(true);
    text_B.setSmooth(true);
    sprite_A.setTexture(text_A);
    sprite_B.setTexture(text_B);

    sprite_A.prepare_slide(screen_res_x, screen_res_y, new_image.getSize().x, new_image.getSize().y, frames_per_blend + frames_per_pic);


    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed) window.close();
        }

        window.clear();

        if(blending)
        {
            sprite_A.update();
            sprite_B.update();
            int alpha_vanish = 255 - blend_frame * (255.0 / (float)frames_per_blend);
            int alpha_appear = 0   + blend_frame * (255.0 / (float)frames_per_blend);
            if (alpha_vanish < 0   ) alpha_vanish = 0;
            if (alpha_appear > 255 ) alpha_appear = 255;
            if(B_is_coming)
            {
                //std::cout << "Blending, B is coming, col_A.a = " << new_alpha_a << " col_B.a = " << new_alpha_b << std::endl;
                sprite_A.setColor(sf::Color(255, 255, 255, alpha_vanish));
                sprite_B.setColor(sf::Color(255, 255, 255, alpha_appear));
                window.draw(sprite_B);
                window.draw(sprite_A);
            }
            else
            {
                //std::cout << "Blending, A is coming, col_A.a = " << new_alpha_a << " col_B.a = " << new_alpha_b << std::endl;
                sprite_B.setColor(sf::Color(255, 255, 255, alpha_vanish));
                sprite_A.setColor(sf::Color(255, 255, 255, alpha_appear));
                window.draw(sprite_A);
                window.draw(sprite_B);
            }
            if(blend_frame > frames_per_blend)
            {
                pic_frame = 0;
                blending = false;
                B_is_coming = !B_is_coming;
            }
            blend_frame++;
        }
        else // No blending, show only one pic.
        {
            if(B_is_coming) // show Pic A
            {
                //std::cout << "Showing, B is coming" << std::endl;
                sprite_A.update();
                window.draw(sprite_A);
            }
            else            // show Pic B
            {
                //std::cout << "Showing, A is coming" << std::endl;
                sprite_B.update();
                window.draw(sprite_B);
            }
            if(pic_frame > frames_per_pic)
            {
                blend_frame = 0;
                blending = true;
            }
            // Start Thread to load next pic in first pic_frame
            if(pic_frame == 0)
            {
                if(pic_load_thread.joinable()) pic_load_thread.join();
                image_is_ready = false;
                if(current_pic < pics_to_show.size()-1)
                {
                    //std::cout << "Starting thread to load pic " << pics_to_show[current_pic + 1] << std::endl;
                    pic_load_thread = std::thread(load_pic, pics_to_show[current_pic + 1]);
                    current_pic++;
                }
                else // Last pic is beeing displayed!
                {
                    if(cycling)
                    {
                        pic_load_thread = std::thread(load_pic, pics_to_show[0]);
                        current_pic = 1;
                    }
                    else
                    {
                        last_pic = true;
                    }
                }
            }
            // Update texture on gfx-card just before start of blending (pic should be loaded until then...)
            // First copy bg_image to texture in case new_images is smaller than current
            if(pic_frame == frames_per_pic-1)
            {
                if(B_is_coming)  text_B.update (bg_image);
                else             text_A.update (bg_image);
            }

            //then copy new_image to texture
            if(pic_frame == frames_per_pic)
            {
                if(last_pic)
                {
                    window.close();
                }
                else
                {
                    if(image_is_ready)
                    {
                        if(B_is_coming)
                        {
                            text_B.update (new_image);
                            sprite_B.prepare_slide(screen_res_x, screen_res_y, new_image.getSize().x, new_image.getSize().y, frames_per_blend + frames_per_pic);
                        }
                        else
                        {
                            text_A.update (new_image);
                            sprite_A.prepare_slide(screen_res_x, screen_res_y, new_image.getSize().x, new_image.getSize().y, frames_per_blend + frames_per_pic);
                        }
                    }
                    else
                    {
                        if(pic_load_thread_running) // We need more time for the image to load....
                        {
                            pic_frame--;
                            std::cout << "Waiting for next pic to be loaded..." << std::endl;
                        }
                        else // The image could not be loaded!
                        {
                            window.close();
                        }
                    }
                }
            }
            pic_frame++;
        }
        window.display();
    }
    return 0;
}
