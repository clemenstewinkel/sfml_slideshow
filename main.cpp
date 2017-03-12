#include <SFML/Graphics.hpp>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <slidepic.h>
#include <chrono>
#include <thread>

const int FRAMES_PER_PIC = 200;
const int FRAMES_PER_BLEND = 254;

const int SCREEN_RES_X = 1920;
const int SCREEN_RES_Y = 1080;

sf::Texture text_A;
sf::Texture text_B;

sf::Image new_image;

std::thread my_thread;

bool B_is_coming    = true;
bool image_is_ready = false;

void timekeeper(std::string message)
{
    static std::chrono::system_clock::time_point tp;
    static bool first_call = true;
    if(first_call)
    {
        tp = std::chrono::system_clock::now();
        first_call = false;
    }
    else
    {
        std::chrono::nanoseconds ns = std::chrono::system_clock::now() - tp;
        if(ns.count() > 1) std::cout  << ns.count() / 1000 << " Microseconds." << message << std::endl;
        tp = std::chrono::system_clock::now();
    }
}

void load_pic(std::string filename)
{
    new_image.loadFromFile(filename);
    image_is_ready = true;
}






int main(int argc, char* argv[])
{
    int opt;

    timekeeper("hallo");
    SlidePic  sprite_A;
    SlidePic  sprite_B;




    std::string pic_dir;
    std::vector<std::string> pics_to_show;
    while ((opt = getopt(argc, argv, "d:")) != -1) {
        switch (opt) {
        case 'd':
            pic_dir = optarg;
            break;
        default: /* '?' */
            fprintf(stderr, "Usage: %s -d <folder with pics> pic_1 pic_2 ... pic_n\n", argv[0]);
            exit(EXIT_FAILURE);
        }
    }
    for (int i = optind; i<argc ; i++) pics_to_show.push_back(pic_dir + "/" + argv[i]);

    sf::RenderWindow window(sf::VideoMode(SCREEN_RES_X, SCREEN_RES_Y), "", sf::Style::Fullscreen);
    window.setVerticalSyncEnabled(true);

    text_A.loadFromFile(pics_to_show[0]);
    text_B.loadFromFile(pics_to_show[1]);

    new_image.loadFromFile(pics_to_show[2]);


    text_A.setSmooth(true);
    text_B.setSmooth(true);

    sprite_A.setTexture(text_A);
    sprite_B.setTexture(text_B);

    sprite_A.prepare_slide(SCREEN_RES_X, SCREEN_RES_Y, FRAMES_PER_BLEND + FRAMES_PER_PIC);
    sprite_B.prepare_slide(SCREEN_RES_X, SCREEN_RES_Y, FRAMES_PER_BLEND + FRAMES_PER_PIC);

    bool blending       = false;
    int pic_frame       = 0;
    int blend_frame     = 0;
    sf::Color col_A;
    sf::Color col_B;
    int current_pic     = 0;

    while (window.isOpen())
    {
        timekeeper("start loop");
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed) window.close();
        }
        timekeeper("after pollEvent");

        window.clear();
        timekeeper("after clear");

        if(blending)
        {
            sprite_A.update();
            sprite_B.update();
            col_A = sprite_A.getColor();
            col_B = sprite_B.getColor();
            if(B_is_coming)
            {
                sprite_A.setColor(sf::Color(col_A.r, col_A.g, col_A.b, col_A.a - (255/FRAMES_PER_BLEND)));
                sprite_B.setColor(sf::Color(col_B.r, col_B.g, col_B.b, col_B.a + (255/FRAMES_PER_BLEND)));
                window.draw(sprite_B);
                window.draw(sprite_A);
            }
            else
            {
                sprite_A.setColor(sf::Color(col_A.r, col_A.g, col_A.b, col_A.a + (255/FRAMES_PER_BLEND)));
                sprite_B.setColor(sf::Color(col_B.r, col_B.g, col_B.b, col_B.a - (255/FRAMES_PER_BLEND)));
                window.draw(sprite_A);
                window.draw(sprite_B);
            }
            blend_frame++;
            if(blend_frame > FRAMES_PER_BLEND)
            {
                blend_frame = 0;
                blending = false;
                B_is_coming = !B_is_coming;
                current_pic++;
            }
        }
        else // No blending, show only one pic.
        {
            if(B_is_coming) // show Pic A
            {
                sprite_A.setColor(sf::Color(255,255,255,255));
                sprite_A.update();
                window.draw(sprite_A);
            }
            else            // show Pic B
            {
                sprite_B.setColor(sf::Color(255,255,255,255));
                sprite_B.update();
                window.draw(sprite_B);
            }
            timekeeper("after draw");

            pic_frame++;
            if(pic_frame > FRAMES_PER_PIC)
            {
                pic_frame = 0;
                blending = true;
                if(B_is_coming)
                {
                    sprite_B.prepare_slide(SCREEN_RES_X, SCREEN_RES_Y, FRAMES_PER_BLEND + FRAMES_PER_PIC);
                    sprite_B.setColor(sf::Color(255,255,255,0));
                }
                else
                {
                    sprite_A.prepare_slide(SCREEN_RES_X, SCREEN_RES_Y, FRAMES_PER_BLEND + FRAMES_PER_PIC);
                    sprite_A.setColor(sf::Color(255,255,255,0));
                }
            }
            // Start Thread to load next pic in first pic_frame
            if(pic_frame == 0)
            {
                timekeeper("before thread_start");
                if(my_thread.joinable()) my_thread.join();
                my_thread = std::thread(load_pic, pics_to_show[current_pic + 1]);
                timekeeper("after thread_start");
            }
            // Update texture on gfx-card just before start of blending (pic should be loaded until then...)
            if(pic_frame == FRAMES_PER_PIC)
            {
                if(image_is_ready)
                {
                    if(B_is_coming)
                        text_B.update (new_image);
                    else
                        text_A.update (new_image);
                }
            }

            timekeeper("before display");
        }
        window.display();
        timekeeper("after display");
    }
    return 0;
}
