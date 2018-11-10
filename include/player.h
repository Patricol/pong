//Patrick Collins (patricol)

#ifndef PLAYER_H
#define PLAYER_H


struct PlayerInputs {
    bool ul;
    bool dr;

    PlayerInputs():ul(false),
                   dr(false){}
};

struct Inputs {
    struct PlayerInputs p[4];
    bool spacebar;
    
    Inputs():spacebar(false){}
};

bool WindowOpen(void);

bool InFocus(void);

struct Inputs GetInputs(void);

void UpdatePlayerView(struct GameState);

#endif
