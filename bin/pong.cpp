//Patrick Collins (patricol)

#include <SFML/Graphics.hpp>
#include <SFML/System/Time.hpp>
#include <algorithm>
#include <math.h>
#include <stdlib.h>
#include "pong.h"
#include "player.h"
#include "ai.h"

float pi = acos(-1);

int winning_score = 11;

float base_paddle_speed = 0.8;
float base_ball_speed = 1.2;
float speed_scales[] = {0.3, 0.6, 1, 1.5, 5};
float framerate_scales[] = {0.0, 0.1, 0.3, 0.7, 2.0};

float paddle_thickness = 0.015;
float paddle_wall_gap = paddle_thickness*1.5;//Distance between wall and center of paddle that moves along that wall.
float paddle_length = 0.08;
float powerup_paddle_length = 0.16;
float ball_diameter = 0.02;
float net_thickness = 0.02;
float net_length = 0.2;
float net_x = 0.5;
float net_0_y = 0.3;
float net_1_y = 0.7;
float powerup_width = 0.05;
float powerup_height = 0.05;
float paddle_edge_buffer = 0.04;
float powerup_location_side_size = 0.6;

float max_momentum = 1.0;

int last_hitter = -1;

float time_since_last_frame = 0.0;


float ForceBetween(float value, float min, float max) {
    if (value > max) {return max;}
    if (value < min) {return min;}
    return value;
}

float RandomFloatBetween(float minimum, float maximum) {
    //from https://stackoverflow.com/a/686373
    float random_float = minimum + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(maximum - minimum)));
    return random_float;
}

bool PercentChance(int chance) {
    return (bool) (rand() % 100 < chance);
}

bool CoinToss(void) {
    return PercentChance(50);
}

float NewBallAngle(void) {
    return RandomFloatBetween(0.0, pi*2);
}

float TweakAngle(float angle) {
    return angle + RandomFloatBetween(-pi/20, pi/20);
}


struct GameState ResetBall(struct GameState gs) {
    gs.ball.x = 0.5;
    gs.ball.y = 0.5;
    gs.ball.w = ball_diameter;
    gs.ball.h = ball_diameter;
    gs.ball_angle = NewBallAngle();
    return gs;
}

struct GameState ResetGame(struct GameState gs) {
    gs = ResetBall(gs);
    gs.powerup.exists = false;
    gs.powerup.x = 0.0;
    gs.powerup.y = 0.0;
    gs.powerup.w = powerup_width;
    gs.powerup.h = powerup_height;
    float paddle_offset, offset;
    for (int i=0; i<4; i++) {
        gs.p[i].powerup_state = 0;
        gs.p[i].score = 0;
        paddle_offset = i%2 ? 1-paddle_wall_gap : paddle_wall_gap;
        offset = i%2 ? 1.5 : -0.5;
        if (i<2) {
            gs.p[i].paddle.x = paddle_offset;
            gs.p[i].paddle.y = 0.5;
            gs.p[i].paddle.w = paddle_thickness;
            gs.p[i].paddle.h = paddle_length;
            gs.p[i].goal.w = 1 - ball_diameter*2;
            gs.p[i].goal.h = 2;
            gs.p[i].goal.x = offset;
            gs.p[i].goal.y = 0.5;
            gs.walls[i].w = 1;
            gs.walls[i].h = 2;
            gs.walls[i].x = offset;
            gs.walls[i].y = 0.5;
        } else {
            gs.p[i].paddle.x = 0.5;
            gs.p[i].paddle.y = paddle_offset;
            gs.p[i].paddle.w = paddle_length;
            gs.p[i].paddle.h = paddle_thickness;
            gs.p[i].goal.w = 2;
            gs.p[i].goal.h = 1 - ball_diameter*2;
            gs.p[i].goal.x = 0.5;
            gs.p[i].goal.y = offset;
            gs.walls[i].w = 2;
            gs.walls[i].h = 1;
            gs.walls[i].x = 0.5;
            gs.walls[i].y = offset;
        }
    }
    for (int i=0; i<2; i++) {
            gs.nets[i].x = net_x;
            gs.nets[i].w = net_thickness;
            gs.nets[i].h = net_length;
    }
    gs.nets[0].y = net_0_y;
    gs.nets[1].y = net_1_y;
    return gs;
}


struct GameState HandlePowerupMode(struct GameState gs) {
    if (!gs.powerup.exists && PercentChance(10)) {
        float min, max;
        min = 0.5 - powerup_location_side_size/2;
        max = 1-min;
        gs.powerup.x = RandomFloatBetween(min, max);
        gs.powerup.y = RandomFloatBetween(min, max);
        gs.powerup.exists = true;
    }
    return gs;
}

sf::RectangleShape MakeCollisionRect(struct Object rect_obj) {
    float x, y;
    sf::RectangleShape rect(sf::Vector2f(rect_obj.w, rect_obj.h));
    x = rect_obj.x - rect_obj.w/2;
    y = rect_obj.y - rect_obj.h/2;
    rect.setPosition(x, y);
    return rect;
}

bool RectsCollide(struct Object rect_obj_1, struct Object rect_obj_2) {
    sf::RectangleShape rect_1, rect_2;
    rect_1 = MakeCollisionRect(rect_obj_1);
    rect_2 = MakeCollisionRect(rect_obj_2);
    return rect_1.getGlobalBounds().intersects(rect_2.getGlobalBounds());
}

struct GameState Bounce(struct GameState gs, struct Object obj_clipping_with_ball) {
    //passing gs not ball so ball doesn't push objects around if args are reversed.
    //Unclip and change direction.
    float horizontal_displacement, vertical_displacement;
    if (gs.ball.x > obj_clipping_with_ball.x) {
        //Ball is to the right of the object.
        horizontal_displacement = abs((gs.ball.x-(gs.ball.w/2))-(obj_clipping_with_ball.x+(obj_clipping_with_ball.w/2)));
    } else {
        horizontal_displacement = -abs((gs.ball.x+(gs.ball.w/2))-(obj_clipping_with_ball.x-(obj_clipping_with_ball.w/2)));
    }
    if (gs.ball.y > obj_clipping_with_ball.y) {
        //Ball is below the object.
        vertical_displacement = abs((gs.ball.y-(gs.ball.h/2))-(obj_clipping_with_ball.y+(obj_clipping_with_ball.h/2)));
    } else {
        vertical_displacement = -abs((gs.ball.y+(gs.ball.h/2))-(obj_clipping_with_ball.y-(obj_clipping_with_ball.h/2)));
    }
    
    if (abs(horizontal_displacement)<abs(vertical_displacement)) {
        //move horizontally
        gs.ball.x += horizontal_displacement;
        gs.ball_angle = TweakAngle(gs.ball_angle * -1);
    } else {
        //move vertically
        gs.ball.y += vertical_displacement;
        gs.ball_angle = TweakAngle(pi - gs.ball_angle);
    }
    return gs;
}

struct GameState MovingBounce(struct GameState gs, struct Object obj_clipping_with_ball, struct Movement movement) {
    //bounces against a moving object; object's motion/momentum affects bounce direction.
    return Bounce(gs, obj_clipping_with_ball);
}

struct GameState PaddleBounce(struct GameState gs, int paddle_hit) {
    //Alternatively, have the part of the paddle that was hit fully determine the angle of the ball. Some pong clones do that; adds a lot of strategy.
    if (gs.p[paddle_hit].momentum) {
        struct Movement movement;
        if (paddle_hit>=2) {movement.angle += pi/2;}
        if (gs.p[paddle_hit].momentum<0) {movement.angle += pi;}
        movement.energy = ForceBetween(abs(gs.p[paddle_hit].momentum), 0.0, max_momentum);
        return MovingBounce(gs, gs.p[paddle_hit].paddle, movement);
    } else {return Bounce(gs, gs.p[paddle_hit].paddle);}
}


struct GameState PowerUpPaddle(struct GameState gs, int player) {
    if (player<2) {gs.p[player].paddle.h = powerup_paddle_length;}
    else {gs.p[player].paddle.w = powerup_paddle_length;}
    return gs;
}

struct GameState PowerDownPaddle(struct GameState gs, int player) {
    if (player<2) {gs.p[player].paddle.h = paddle_length;}
    else {gs.p[player].paddle.w = paddle_length;}
    return gs;
}


struct GameState CheckCollision(struct GameState gs) {
    
    for (int i=0; i<4; i++) {
        if (gs.p[i].exists) {
            if (RectsCollide(gs.ball, gs.p[i].goal)) {
                gs.play_sound = 3;
                gs = ResetBall(gs);
                gs.p[i%2].score += 1;
            } else if (RectsCollide(gs.ball, gs.p[i].paddle)) {
                last_hitter = i;
                gs.play_sound = 2;
                if (gs.options[6]==3) {
                    //doing this here ensures that last_hitter is set.
                    gs = HandlePowerupMode(gs);
                }
                gs = PaddleBounce(gs, i);
                if (gs.p[i].powerup_state) {
                    gs.p[i].powerup_state--;
                    if (!gs.p[i].powerup_state) {gs = PowerDownPaddle(gs, i);}
                }
            }
        } else {
            if (RectsCollide(gs.ball, gs.walls[i])) {
                gs.play_sound = 1;
                gs = Bounce(gs, gs.walls[i]);
            }
        }
    }

    
    if (gs.options[6]==2) {
        for (int i=0; i<2; i++) {
            if (RectsCollide(gs.ball, gs.nets[i])) {
                gs.play_sound = 1;
                gs = Bounce(gs, gs.nets[i]);
            }
        }
    }

    if (gs.powerup.exists) {
        if (RectsCollide(gs.ball, gs.powerup)) {
            gs.p[last_hitter].powerup_state = 10;
            gs.powerup.exists = false;
            gs = PowerUpPaddle(gs, last_hitter);
        }
    }
    
    return gs;
}

struct GameState MovePaddles(struct Inputs new_inputs, struct GameState gs, float paddle_move_distance) {
    for (int i=0; i<4; i++) {
        float buffer;
        float final_move_distance = paddle_move_distance * (1+ForceBetween(abs(gs.p[i].momentum), 0.0, max_momentum));
        //Could simplify this with pointers.
        if (i<2) {
            if (new_inputs.p[i].ul) {gs.p[i].paddle.y -= final_move_distance;}
            if (new_inputs.p[i].dr) {gs.p[i].paddle.y += final_move_distance;}
            buffer = paddle_edge_buffer + gs.p[i].paddle.h/2;
            gs.p[i].paddle.y = ForceBetween(gs.p[i].paddle.y, buffer, 1-buffer);
        } else {
            if (new_inputs.p[i].ul) {gs.p[i].paddle.x -= final_move_distance;}
            if (new_inputs.p[i].dr) {gs.p[i].paddle.x += final_move_distance;}
            buffer = paddle_edge_buffer + gs.p[i].paddle.w/2;
            gs.p[i].paddle.x = ForceBetween(gs.p[i].paddle.x, buffer, 1-buffer);
        }
    }
    return gs;
}

struct GameState MoveBall(struct GameState gs, float ball_move_distance) {
    gs.ball.x += sin(gs.ball_angle) * ball_move_distance;
    gs.ball.y += cos(gs.ball_angle) * ball_move_distance;
    return gs;
}

struct Inputs GetNewInputs(struct GameState gs) {
    struct Inputs new_inputs = GetInputs();
    if (gs.mode==1) {//Get AI Input
        for (int i=0; i<4; i++) {
            if (gs.p[i].is_ai && gs.p[i].exists) {
                new_inputs.p[i] = GetAiInputs(gs, i);
            }
        }
    }
    return new_inputs;
}

struct GameState CheckForAWinner(struct GameState gs) {
    for (int i=0; i<2; i++) {
        if (gs.p[i].score == winning_score) {
            gs.mode = 2;
        }
    }
    return gs;
}

struct GameState HandleMainMenuInput(struct Inputs new_inputs, struct Inputs last_inputs, struct GameState gs) {
    int numoptions[] = {5,5,3,3,3,3,4,2};
    if (new_inputs.p[0].ul && !last_inputs.p[0].ul) {
        gs.selected = (gs.selected+8) % 9;
    }
    if (new_inputs.p[0].dr && !last_inputs.p[0].dr) {
        gs.selected = (gs.selected+1) % 9;
    }
    if (new_inputs.spacebar && !last_inputs.spacebar) {
        if (gs.selected==0) {
            gs.mode = 1;
            for (int i=0; i<4; i++) {
                gs.p[i].exists = gs.options[2+i] < 2;
                gs.p[i].is_ai = gs.p[i].exists && gs.options[2+i]==1;
            }
            winning_score = gs.options[6] ? -1 : 11;
        } else {
            gs.options[gs.selected-1] = (gs.options[gs.selected-1] + 1) % numoptions[gs.selected-1];
        }
    }
    return gs;
}

void UpdateMomentum(struct GameState gs, struct Inputs new_inputs, struct Inputs last_inputs, float passed_time) {
    for (int i=0; i<4; i++) {
        if (new_inputs.p[i].ul != new_inputs.p[i].dr) {
            if (new_inputs.p[i].ul) {
                if (last_inputs.p[i].ul) {gs.p[i].momentum -= passed_time;}
                else {gs.p[i].momentum = -passed_time;}
            } else {
                if (last_inputs.p[i].dr) {gs.p[i].momentum += passed_time;}
                else {gs.p[i].momentum = passed_time;}
            }
        } else {gs.p[i].momentum = 0;}
    }
}


struct GameState HandleGameCycle(struct GameState gs, struct Inputs new_inputs, struct Inputs last_inputs, float passed_time) {
    float paddle_move_distance, ball_move_distance;
    UpdateMomentum(gs, new_inputs, last_inputs, passed_time);
    paddle_move_distance = base_paddle_speed * passed_time * speed_scales[gs.options[0]];
    ball_move_distance = base_ball_speed * passed_time * speed_scales[gs.options[0]];
    gs = MovePaddles(new_inputs, gs, paddle_move_distance);
    gs = CheckCollision(gs);
    gs = MoveBall(gs, ball_move_distance);
    return gs;
}


int main(int argc, char** argv) {
    float passed_time;
    struct GameState gs;
    gs = ResetGame(gs);
    struct Inputs new_inputs, last_inputs;
    sf::Clock clock;
    last_inputs = GetNewInputs(gs);
    while(WindowOpen()) {
        passed_time = clock.restart().asSeconds();
        time_since_last_frame += passed_time;
        gs.update_this_frame = time_since_last_frame > framerate_scales[gs.options[1]];
        if (gs.update_this_frame) {time_since_last_frame = 0;}
        UpdatePlayerView(gs);
        gs.play_sound = 0;
        new_inputs = GetNewInputs(gs);
        switch(gs.mode) {
            case 0://main menu
                gs = HandleMainMenuInput(new_inputs, last_inputs, gs);
                break;
            case 1://game
                gs = HandleGameCycle(gs, new_inputs, last_inputs, passed_time);
                gs = CheckForAWinner(gs);
                if (!InFocus() || (new_inputs.spacebar && !last_inputs.spacebar)) {gs.mode = 3;}
                break;
            case 2://winner & ask to restart or quit.
                if (new_inputs.p[0].dr && !last_inputs.p[0].dr) {
                    gs.mode = 0;
                    gs = ResetGame(gs);
                } else if (new_inputs.p[0].ul && !last_inputs.p[0].ul) {
                    gs.mode = 1;
                    gs = ResetGame(gs);
                }
                break;
            case 3://pause
                if (new_inputs.p[0].dr && !last_inputs.p[0].dr) {
                    gs.mode = 0;
                    gs = ResetGame(gs);
                } else if (new_inputs.spacebar && !last_inputs.spacebar) {gs.mode = 1;}
                break;
        }
        last_inputs = new_inputs;
    }
    return 0;
}

/*
Add options to choose the paddle size and speed.
*/
