//Patrick Collins (patricol)

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <SFML/Window/VideoMode.hpp>
#include <string.h>
#include "player.h"
#include "pong.h"

float view_width = 800.0;
float view_height = 600.0;


bool in_focus = true;


sf::SoundBuffer sound_buffer;
sf::Sound sound;


//sf::RenderWindow App(sf::VideoMode((int) view_width, (int) view_height,32), "Pong - Patrick Collins", sf::Style::Titlebar | sf::Style::Close);
sf::RenderWindow App(sf::VideoMode((int) view_width, (int) view_height,32), "Pong - Patrick Collins");

bool WindowOpen(void) {
    return App.isOpen();
}

bool InFocus(void) {
    return in_focus;
}

struct Inputs GetInputs(void) {
    //App.setSize(sf::Vector2u(1000, 1000));
    struct Inputs new_inputs;
    sf::Event Event;
    while(App.pollEvent(Event)) {
        if(Event.type == sf::Event::Closed) {
            App.close();
        }
        if(Event.type == sf::Event::Resized) {
            view_width = Event.size.width;
            view_height = Event.size.height;
            sf::FloatRect visibleArea(0, 0, Event.size.width, Event.size.height);
            App.setView(sf::View(visibleArea));
        }
        if(Event.type == sf::Event::LostFocus) {in_focus = false;}
        if(Event.type == sf::Event::GainedFocus) {in_focus = true;}
    }
    new_inputs.p[0].ul = sf::Keyboard::isKeyPressed(sf::Keyboard::A) && in_focus;
    new_inputs.p[0].dr = sf::Keyboard::isKeyPressed(sf::Keyboard::Z) && in_focus;
    new_inputs.p[1].ul = sf::Keyboard::isKeyPressed(sf::Keyboard::P) && in_focus;
    new_inputs.p[1].dr = sf::Keyboard::isKeyPressed(sf::Keyboard::L) && in_focus;
    new_inputs.p[2].ul = sf::Keyboard::isKeyPressed(sf::Keyboard::C) && in_focus;
    new_inputs.p[2].dr = sf::Keyboard::isKeyPressed(sf::Keyboard::V) && in_focus;
    new_inputs.p[3].ul = sf::Keyboard::isKeyPressed(sf::Keyboard::N) && in_focus;
    new_inputs.p[3].dr = sf::Keyboard::isKeyPressed(sf::Keyboard::M) && in_focus;
    new_inputs.spacebar = sf::Keyboard::isKeyPressed(sf::Keyboard::Space) && in_focus;
    return new_inputs;
}





sf::Font ImportFont(void) {
    sf::Font bitfont;
    bitfont.loadFromFile("./src/bit5x3.ttf");
    return bitfont;
}
sf::Font bitfont = ImportFont();

sf::Text GetBasicText(int character_size, std::basic_string<char> text) {
    sf::Text basic_text;
    basic_text.setFont(bitfont);
    basic_text.setString(text);
    basic_text.setCharacterSize(character_size);
    sf::FloatRect text_bounds = basic_text.getLocalBounds();
    basic_text.setOrigin(text_bounds.left + text_bounds.width/2, text_bounds.top + text_bounds.height/2);
    return basic_text;
}

void DrawColorRect(struct Object rect_obj, sf::Color color) {
    float x, y;
    sf::RectangleShape rect(sf::Vector2f(rect_obj.w*view_width, rect_obj.h*view_height));
    x = rect_obj.x - rect_obj.w/2;
    y = rect_obj.y - rect_obj.h/2;
    rect.setPosition(x*view_width, y*view_height);
    rect.setFillColor(color);
    App.draw(rect);
}

void DrawRect(struct Object rect_obj) {
    DrawColorRect(rect_obj, sf::Color::White);
}

void DrawPaddles(struct GameState gs) {
    for (int i=0; i<4; i++) {
        if (gs.p[i].exists) {
            DrawRect(gs.p[i].paddle);
        }
    }
}


void DrawScore(int score, int player) {
    //center the text?
    sf::Text text = GetBasicText(50, std::to_string(score));
    text.setFillColor(sf::Color::Red);
    text.setPosition((0.25 + 0.5*player)*view_width, (0.25 + 0.5*player)*view_height);
    App.draw(text);
    
}

void DrawScores(struct GameState gs) {
    //what if there are only players from one side? current behavior shows no score but ends the game with win for nonexistent team eventually
    if ((gs.p[0].exists || gs.p[2].exists) && (gs.p[1].exists || gs.p[3].exists)) {
        for (int i=0; i<2; i++) {
            DrawScore(gs.p[i].score, i);
        }
    }
}


void DrawWin(int team) {
    sf::Text text = GetBasicText(70, "Team " + std::to_string(team+1) + " Wins!");
    text.setFillColor(sf::Color::Red);
    text.setPosition(0.5*view_width, 0.5*view_height);
    sf::Text play_again = GetBasicText(40, "Press A to Play Again");
    sf::Text quit = GetBasicText(40, "Press Z to Quit");
    play_again.setFillColor(sf::Color::Blue);
    quit.setFillColor(sf::Color::Blue);
    play_again.setPosition(0.5*view_width, 0.8*view_height);
    quit.setPosition(0.5*view_width, 0.9*view_height);
    App.draw(text);
    App.draw(play_again);
    App.draw(quit);
}

void DrawLines(void) {
    sf::RectangleShape line(sf::Vector2f(view_width, 1));
    line.setPosition(0, 0);
    line.setFillColor(sf::Color::Black);
    for (int i=0; i<view_height; i++) {
        App.draw(line);
        line.move(0, 2);
    }
}

void DrawBall(struct Object ball) {
    DrawRect(ball);
}

void DrawMainMenu(int selected, int options[8]) {
    App.clear(sf::Color::Black);
    sf::Text hint = GetBasicText(15, "A Z Space");
    sf::Text menu_items[9];
    std::string speed_options[5] = {"Rock","Sloth","Normal","Cheetah","SANIC"};
    std::string framerate_options[5] = {"PC","Console","Cinematic","Flipbook","PowerPoint"};
    std::string player_options[3] = {"Human","AI","Off"};
    std::string mode_options[4] = {"First to 11","Infinite","Obstacles","Power-Ups"};
    menu_items[0] = GetBasicText(70, "Start!");
    menu_items[1] = GetBasicText(50, "Speed: " + speed_options[options[0]]);
    menu_items[2] = GetBasicText(50, "Framerate: " + framerate_options[options[1]]);
    menu_items[3] = GetBasicText(50, "P1: " + player_options[options[2]]);
    menu_items[4] = GetBasicText(50, "P2: " + player_options[options[3]]);
    menu_items[5] = GetBasicText(50, "P3: " + player_options[options[4]]);
    menu_items[6] = GetBasicText(50, "P4: " + player_options[options[5]]);
    menu_items[7] = GetBasicText(50, mode_options[options[6]]);
    menu_items[8] = GetBasicText(50, "Help");
    for (int i=0; i<9; i++) {
        menu_items[i].setPosition(0.5*view_width, (0.1+(0.1*i))*view_height);
        menu_items[i].setFillColor(sf::Color::Blue);
    }
    menu_items[selected].setFillColor(sf::Color::Red);
    for (int i=0; i<9; i++) {App.draw(menu_items[i]);}
    hint.setPosition(0.5*view_width, 0.97*view_height);
    App.draw(hint);
    if (options[7]) {
        sf::Text hints[4];
        hints[0] = GetBasicText(30, "UP=A DOWN=Z");
        hints[1] = GetBasicText(30, "UP=P DOWN=L");
        hints[2] = GetBasicText(30, "LEFT=C RIGHT=V");
        hints[3] = GetBasicText(30, "LEFT=N RIGHT=M");
        sf::Text teams[4];
        teams[0] = GetBasicText(30, "TEAM 1");
        teams[1] = GetBasicText(30, "TEAM 2");
        teams[2] = GetBasicText(30, "TEAM 1");
        teams[3] = GetBasicText(30, "TEAM 2");
        for (int i=0; i<4; i++) {
            hints[i].setPosition(0.2*view_width, (0.4+(0.1*i))*view_height);
            teams[i].setPosition(0.8*view_width, (0.4+(0.1*i))*view_height);
            App.draw(hints[i]);
            App.draw(teams[i]);
        }
    sf::Text credit = GetBasicText(15, "Created by Patrick Collins (Patricol)");
    credit.setPosition(0.5*view_width, 0.95*view_height);
    App.draw(credit);
    }
}

void DrawPause(void) {
    sf::Text text = GetBasicText(70, "PAUSED");
    sf::Text cont = GetBasicText(40, "Press Space to Continue");
    sf::Text quit = GetBasicText(40, "Press Z to Quit");
    text.setFillColor(sf::Color::Blue);
    cont.setFillColor(sf::Color::Red);
    quit.setFillColor(sf::Color::Red);
    text.setPosition(0.5*view_width, 0.5*view_height);
    cont.setPosition(0.5*view_width, 0.8*view_height);
    quit.setPosition(0.5*view_width, 0.9*view_height);
    App.draw(text);
    App.draw(cont);
    App.draw(quit);
}


void PlaySound(int which_sound) {
    if (which_sound) {
        switch(which_sound) {
            case 1:
                if(!sound_buffer.loadFromFile("./src/wall.wav")) {}
                break;
            case 2:
                if(!sound_buffer.loadFromFile("./src/paddle.wav")) {}
                break;
            case 3:
                if(!sound_buffer.loadFromFile("./src/score.wav")) {}
                break;
        }
        sound.setBuffer(sound_buffer);
        sound.play();
    }
}

void DrawSpecial(struct GameState gs) {
    if (gs.options[6]==2) {//Obstacles
        DrawRect(gs.nets[0]);
        DrawRect(gs.nets[1]);
    }
    if (gs.powerup.exists) {//Power-Ups
        DrawColorRect(gs.powerup, sf::Color::Blue);
    }
}

void UpdatePlayerView(struct GameState gs) {
    if (gs.update_this_frame) {
        switch(gs.mode) {
            case 0:
                DrawMainMenu(gs.selected, gs.options);
                break;
            case 1:
                App.clear(sf::Color::Black);
                DrawPaddles(gs);
                DrawSpecial(gs);
                DrawScores(gs);
                DrawBall(gs.ball);
                PlaySound(gs.play_sound);
                break;
            case 2:
                App.clear(sf::Color::Black);
                DrawPaddles(gs);
                DrawScores(gs);
                if (gs.p[0].score > gs.p[1].score) {DrawWin(0);}
                else {DrawWin(1);}
                PlaySound(gs.play_sound);
                break;
            case 3:
                DrawPause();
                break;
        }
        DrawLines();
        App.display();
    } else {
        PlaySound(gs.play_sound);
    }
}

/*
owner of the powerup gets a double sized pad for the next 10 bounces.
Also a FFA mode.
*/
