//Patrick Collins (patricol)

#ifndef PONG_H
#define PONG_H


struct Movement {
    float angle;//0.0 through pi*2
    //up = pi
    //down = 0
    //left = 3*pi/2
    //right = pi/2
    float energy;
    
    Movement():angle(0.0),
               energy(0.0){}
};

struct Object {
    float x;
    float y;
    float w;
    float h;

    Object():x(0),
             y(0),
             w(0),
             h(0){}
};


struct PlayerState {
    bool exists;
    bool is_ai;
    struct Object paddle;
    float momentum;//positive is down or right, negative is up or left.
    struct Object goal;
    int powerup_state;
    int score;

    PlayerState():exists(false),
                  is_ai(false),
                  momentum(0.0),
                  powerup_state(0),
                  score(0){}
};

struct PowerUp : Object {
    bool exists;

    PowerUp():exists(false){}
};


struct GameState {
    int mode;
    int selected;
    int options[8];
    int play_sound;
    bool update_this_frame;//Used for framerate setting.
    struct Object ball;
    float ball_angle;
    struct PowerUp powerup;
    struct PlayerState p[4];
    struct Object walls[4];
    struct Object nets[2];

    GameState():mode(0),
                selected(1),//Making the default selected option not be start, so the user discovers the AZ keys before starting a game for the first time.
                options{2,0,0,1,2,2,0,0},
                play_sound(0),
                update_this_frame(true),
                ball_angle(0.0){}
};

#endif
