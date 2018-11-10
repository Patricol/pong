//Patrick Collins (patricol)

#include <SFML/Graphics.hpp>
#include "player.h"
#include "pong.h"
#include "ai.h"

struct PlayerInputs GetAiInputs(struct GameState gs, int which_player) {
    struct PlayerInputs new_inputs;
    if (which_player < 2) {
        if (gs.ball.y > gs.p[which_player].paddle.y) {new_inputs.dr = true;}
        else if (gs.ball.y < gs.p[which_player].paddle.y) {new_inputs.ul = true;}
    } else {
        if (gs.ball.x > gs.p[which_player].paddle.x) {new_inputs.dr = true;}
        else if (gs.ball.x < gs.p[which_player].paddle.x) {new_inputs.ul = true;}
    }
    return new_inputs;
}

