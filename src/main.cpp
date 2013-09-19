#ifdef __cplusplus
    #include <cstdlib>
#else
    #include <stdlib.h>
#endif
#ifdef __APPLE__
#include <SDL/SDL.h>
#else
#include <SDL.h>
#endif

#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <GL/gl.h>
#include <GL/glu.h>
#include <cmath>
#include <string>
#include <windows.h>
#include <time.h>

#include "SDL_image.h"
//My knowledge of sound/music isn't great, so the code could be sloppy.
#include "SDL_mixer.h"
#include "SDL_audio.h"

#define SBPP 16//32

#include <time.h>
#include <list>

#define GL_BGR 0x80e0
#define PI 3.14159265

#include <boost/signals2.hpp>
#include <boost/utility/singleton.hpp>
/*
enum names_for_my_conditions {CONDITION_ONE = 0001, CONDITION_TWO = 0010, CONDITION_THREE = 0100};

void add_another_condition_and_check_if_all_three_conditions_are_true(names_for_my_conditions bit_condition)
{
    //Ek weet static is bad, maar dis net 'n quick example! xD
    static unsigned int all_bit_conditions_that_are_true_currently = 0000;

    all_bit_conditions_that_are_true_currently = all_bit_conditions_that_are_true_currently | (unsigned int) bit_condition;

    unsigned int all_three_conditions_are_true = 0111;

    if(all_three_conditions_are_true & all_bit_conditions_that_are_true_currently)
        cout << "All three the conditions are currently true!";

    //In this case, this if will be true!

    //Implementation: add_another_condition(CONDITION_ONE);
    //                add_another_condition(CONDITION_TWO | CONDITION_THREE);
}
*/
//#include "GameSignals.h"

///DO_NOT_READ_THIS!
///THIS IS AN OPENGL/SDL TEMPLATE FOR BASIC OPPERATIONS.
///THIS IS AN PERSPECTIVE 3D TEMPLATE. USE APPROPRIATE VERTEX CORDINATES.

///Important functions:
///    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
///    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
///    glBlendFunc(GL_DST_COLOR,GL_ZERO);
///    glBlendFunc(GL_ONE, GL_ONE);
///    glDisable(GL_DEPTH_TEST);
///    glLineWidth(2.0);
///    glColor4f(1.0, 1.0, 1.0, 1.0);
///    SDL_GetTicks();
///

using namespace std;

const int TextNum = 5;

int SW = 850; ///VERY IMPORTANT, THEY ARE NOT CONSTANT, AS THE SCREEN IS RESIZABLE.
int SH = 560;

int beforeResizeSW = SW;
int beforeResizeSH = SH;

GLuint texture[TextNum];
SDL_Surface *surface;

enum _gamemode_enum {GM_EASY, GM_HARD};
enum player_movement_direction {P_MOVE_UP, P_MOVE_DOWN, P_MOVE_LEFT, P_MOVE_RIGHT, NO_MOVEMENT};

const float start_movement_speed_for_player = 0.1;
const int PLAYER_INITIAL_SIZE = 30;
const int NPC_INITIAL_SIZE = 40;
const float how_fast_projectiles_travel_initially = 0.1;
const int player_base_initial_damage = 5;
const float player_start_position_x = SW/2;
const float player_start_position_y = SH-50;
const float initial_projectile_size = 10;
const float correction_player_projectile_launch_y_start_cords = 15;
const float correction_player_projectile_launch_x_start_cords = 2;

//I've designed a "more advanced" texture loader in the other game, but I thought for now this would be enough.
bool loadTextures()
{
    SDL_Surface *textureImages[TextNum];

    if(!(textureImages[0] = IMG_Load("Textures/player_ship2_alpha.png"))) return false;
    if(!(textureImages[1] = IMG_Load("Textures/enemy_ship_2_alpha.png"))) return false;
    if(!(textureImages[2] = IMG_Load("Textures/enemy_ship_3_alpha.png"))) return false;
    if(!(textureImages[3] = IMG_Load("Textures/player_projectile_alpha.png"))) return false;
    if(!(textureImages[4] = IMG_Load("Textures/enemy_projectile_alpha.png"))) return false;

    glGenTextures(TextNum, texture);

    SDL_WM_SetIcon(textureImages[0], NULL);

    for(int i = 0; i < TextNum; i++){
        glBindTexture(GL_TEXTURE_2D, texture[i]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
        gluBuild2DMipmaps(GL_TEXTURE_2D, 4, textureImages[i]->w, textureImages[i]->h, GL_RGBA, GL_UNSIGNED_BYTE, textureImages[i]->pixels);
    }

    for(int i = 0; i < TextNum; i++){
        if(textureImages[i])
            free(textureImages[i]);
    }

    return true;
}
//A class created in the other game... Ported over.
//Many functions are not neccesary in this case... And VERY ARGUABLY CODED xD.
class delaySome {
public:
    delaySome(bool isThisAnEventDelay);
    void setDelay(float deL) { delayTime = deL*1000;}
    void setDelayMili(float deL) { delayTime = deL; }
    void startClock() { if(referenceTime == 0) referenceTime = SDL_GetTicks()+1; } //An inherit check.
    void stopClock() { referenceTime = 0; }
    bool isDelayOver();
    float getReferenceTime() { return referenceTime; }
    void startEvent(bool SE = true) { eventStarted = SE; }
    void restartEvent() { eventStarted = false; }
    bool isEventStarted() { return eventStarted; }
    void setLoopEvent(bool setLoop = true) { if(thisIsAnEvent) loopEvent = setLoop; } //NB CHECK HERE.
    void setDelayAsEventBased(bool eventYes = true) { thisIsAnEvent = eventYes; }
    bool clockNotStartedYet() { if(referenceTime == 0) return true; else return false; }

    float getDeltaTime(); ///WORKS WITH STARTCLOCK AS THE INITIAL TIME.
private:
    float referenceTime;
    float delayTime;
    float currentTime;
    bool thisIsAnEvent;
    bool eventStarted;
    bool loopEvent;

    float firstTime;
    float nextTime;
};

//Untested.
float delaySome::getDeltaTime()
{
    nextTime = SDL_GetTicks();

    float holdFirstTime = firstTime;

    firstTime = nextTime;

    return (nextTime - holdFirstTime);
}

delaySome::delaySome(bool isThisAnEventDelay = false)
{
    referenceTime = 0;
    delayTime = 0;
    currentTime = 0;
    thisIsAnEvent = isThisAnEventDelay;
    eventStarted = false;
    loopEvent = false;
}

bool delaySome::isDelayOver()
{
    if(!eventStarted && referenceTime != 0) //!!/REFERENCETIME CONDITION ADDED.
    {
        currentTime = SDL_GetTicks();

        if((currentTime - referenceTime) > delayTime)
        {
            referenceTime = SDL_GetTicks();

            if(thisIsAnEvent)
                eventStarted = true;

            if(eventStarted)
                return !loopEvent;

            else return true; //Basicly just invert every return if the event should loop.
        }
    }
    if(eventStarted && referenceTime != 0)
        return loopEvent;

    else return false;
}

///An example...
///delaySome ourDelay;
///ourDelay.setDelayMili(1500);
///ourDelay.startClock();
///...
///if(ourDelay.isDelayOver())
    ///1.5 seconds has passed since the clock started. This if will be true 1.5 seconds are this run, ect ect.
///That's it! All the other functions are unneccsary and a little hacked xD.

typedef struct {
    float xComp;
    float yComp;
} vectorFormat; //Only vectors in 2D.

class Projectile
{
    public:
        //x and y is the starting coordinates of the projectile.
        Projectile(float x, float y, float whereToGoX, float whereToGoY, int damage, float size) : x(x), y(y), whereToGoX(whereToGoX), whereToGoY(whereToGoY), damage(damage), size(size)
        {
            unitVect = getUnitVector(whereToGoX, x, whereToGoY, y);
            unitVect.yComp = -unitVect.yComp;
            how_fast_projectiles_travel = how_fast_projectiles_travel_initially;
        }

        void update(float delta_time)
        {
            x += unitVect.xComp*delta_time*how_fast_projectiles_travel;
            y += unitVect.yComp*delta_time*how_fast_projectiles_travel;

            //cout << "X: " << x << endl;
            //cout << "Y: " << y << endl;
        }

        vectorFormat getUnitVector(float,float,float,float);

    protected:
        float x, y;  // cur position.
        float whereToGoX, whereToGoY; // direction of travelling
        int damage;
        float size;
        float how_fast_projectiles_travel;
        vectorFormat unitVect;

};

class enemyProjectile : public Projectile {
public:
    enemyProjectile(float x, float y, float whereToGoX, float whereToGoY, int damage, float size) : Projectile(x,y,whereToGoX,whereToGoY,damage,size) {}
    void draw();
protected:
};

//Could have also written the draw function in the base class... Just need to make it so that we can set the texture then. Not that hard though.

void enemyProjectile::draw()
{
    glLoadIdentity();
    glColor4f(1.0,1.0,1.0,1.0);
    glEnable(GL_BLEND);

    glTranslatef(x,y,0);

    glBindTexture(GL_TEXTURE_2D, texture[4]);
    glBegin(GL_QUADS);
    glTexCoord2f(1.0,1.0); glVertex2f(size,size);
    glTexCoord2f(0.0,1.0); glVertex2f(-size,size);
    glTexCoord2f(0.0,0.0); glVertex2f(-size,-size);
    glTexCoord2f(1.0,0.0); glVertex2f(size,-size);

    glEnd();
    glDisable(GL_BLEND);
}

class playerProjectile : public Projectile {
public:
    playerProjectile(float x, float y, float whereToGoX, float whereToGoY, int damage, float size) : Projectile(x,y,whereToGoX,whereToGoY,damage,size) {}
    void draw();
//protected:
};

void playerProjectile::draw()
{
    glLoadIdentity();
    glColor4f(1.0,1.0,1.0,1.0);
    glEnable(GL_BLEND);

    glTranslatef(x,y,0);

    glBindTexture(GL_TEXTURE_2D, texture[3]);
    glBegin(GL_QUADS);
    glTexCoord2f(1.0,1.0); glVertex2f(size,size);
    glTexCoord2f(0.0,1.0); glVertex2f(-size,size);
    glTexCoord2f(0.0,0.0); glVertex2f(-size,-size);
    glTexCoord2f(1.0,0.0); glVertex2f(size,-size);
    glEnd();
    glDisable(GL_BLEND);
}

vectorFormat Projectile::getUnitVector(float positionBX, float positionAX, float positionBY, float positionAY)
{
    vectorFormat unitVector;

    float positionVectorXComp = positionBX - positionAX;
    float positionVectorYComp = positionBY - positionAY;

    //The function to calculate the radius can actually just be called, but for now we keep it like this.
    float positionVectorSize = sqrt(pow((positionBX - positionAX), 2) + pow((positionBY - positionAY), 2));

    unitVector.xComp = positionVectorXComp/positionVectorSize;
    unitVector.yComp = positionVectorYComp/positionVectorSize;

    return unitVector;
}

class gameEntity {
public:
    gameEntity(float positionX,float positionY, float size) : positionX(positionX), positionY(positionY), size(size) {damage = 0;}
    void set_position(float x, float y) { positionX = x; positionY = y; }
    void set_health(int hp) { health = hp; }
    float get_x() { return positionX; }
    float get_y() { return positionY; }
    float get_size() { return size; }
protected:
    float positionX;
    float positionY;
    int health; //Int?
    int damage;
    float size;
    delaySome game_entity_timer;
};

class Player : public gameEntity {
public:
    Player(float positionX, float positionY, float size) : gameEntity(positionX,positionY,size), how_fast_player_moves_coefficient(start_movement_speed_for_player) {player_move_dir = NO_MOVEMENT; damage = player_base_initial_damage; player_projectile_size = initial_projectile_size;}
    void update(float delta_time);
    void handle_movement(player_movement_direction);
    void draw();
    int get_damage_done_with_projectile();
    float get_projectile_size() { return player_projectile_size; }
protected:
    float how_fast_player_moves_coefficient;
    float player_projectile_size;
    player_movement_direction player_move_dir;
};

int Player::get_damage_done_with_projectile()
{
    //Fancy manipulations could be done here.
    return damage;
}

void Player::update(float delta_time)
{
    delta_time = game_entity_timer.getDeltaTime();

    switch(player_move_dir)
    {
        case P_MOVE_UP:
            positionY -= delta_time*how_fast_player_moves_coefficient;
            break;

        case P_MOVE_DOWN:
            positionY += delta_time*how_fast_player_moves_coefficient;
            break;

        case P_MOVE_LEFT:
            positionX -= delta_time*how_fast_player_moves_coefficient;
            break;

        case P_MOVE_RIGHT:
            positionX += delta_time*how_fast_player_moves_coefficient;
            break;

        default:
            break;
    }
}

void Player::draw()
{
    glLoadIdentity();
    glColor4f(1.0,1.0,1.0,1.0);
    glEnable(GL_BLEND);

    glTranslatef(positionX,positionY,0);

    glBindTexture(GL_TEXTURE_2D, texture[0]);
    glBegin(GL_QUADS);
    glTexCoord2f(1.0,1.0); glVertex2f(size,size);
    glTexCoord2f(0.0,1.0); glVertex2f(-size,size);
    glTexCoord2f(0.0,0.0); glVertex2f(-size,-size);
    glTexCoord2f(1.0,0.0); glVertex2f(size,-size);

    glEnd();
    glDisable(GL_BLEND);
}

void Player::handle_movement(player_movement_direction p_move_dir)
{
    player_move_dir = p_move_dir;
    /*
    switch(p_move_dir)
    {
        case P_MOVE_UP:
            positionY -= delta_time*how_fast_player_moves_coefficient;
            break;

        case P_MOVE_DOWN:
            positionY += delta_time*how_fast_player_moves_coefficient;
            break;

        case P_MOVE_LEFT:
            positionX -= delta_time*how_fast_player_moves_coefficient;
            break;

        case P_MOVE_RIGHT:
            positionX += delta_time*how_fast_player_moves_coefficient;
            break;

        default:
            break;
    }
    */
}

class NPC : public gameEntity {
public:
    NPC(float positionX, float positionY, float size) : gameEntity(positionX,positionY,size) {}
    void update(float delta_time);
    void draw();
protected:

};

void NPC::update(float delta_time)
{
    //UPDATES NPC.
}

void NPC::draw()
{
    glLoadIdentity();
    glColor4f(1.0,1.0,1.0,1.0);
    glEnable(GL_BLEND);

    glTranslatef(positionX,positionY,0);

    glBindTexture(GL_TEXTURE_2D, texture[1]);
    glBegin(GL_QUADS);
    glTexCoord2f(1.0,1.0); glVertex2f(size,size);
    glTexCoord2f(0.0,1.0); glVertex2f(-size,size);
    glTexCoord2f(0.0,0.0); glVertex2f(-size,-size);
    glTexCoord2f(1.0,0.0); glVertex2f(size,-size);

    glEnd();
    glDisable(GL_BLEND);
}

class GameController : public boost::singleton<GameController>
{
	public:
		GameController(const boost::restricted) {timer.startClock();}

		void init(float,float,_gamemode_enum );
		void spawn_enemy(float,float);
		void update();
		void collision_detection(float);
		//void add_projectile(float x, float y, float dx, float dy, int damage);
		void add_enemy_projectile(float x, float y, float dx, float dy, int damage, float);
		void add_player_projectile(float x, float y, float dx, float dy, int damage, float);
		void update_projectiles(float);
		void assign_slots_to_events();

		typedef boost::signals2::signal< void (player_movement_direction) > player_movement_signal;
		typedef boost::signals2::signal< void (float,float,float,float,int,float) > player_particle_launch_signal;

		void add_player_movement_event_function_slot( const player_movement_signal::slot_type& slot );
		void add_player_particle_launch_event_function_slot( const player_particle_launch_signal::slot_type& slot );

		void announce_player_move(player_movement_direction p_move_direction);
		void announce_player_particle_launch();

	private:
		player_movement_signal _player_movement_signal;
		player_particle_launch_signal _player_particle_launch_signal;

        std::list<NPC*> _npcs;
        delaySome timer;
        std::list<enemyProjectile> enemy_projectiles;
        std::list<playerProjectile> player_projectiles;
        Player* _player;
};

void GameController::add_player_movement_event_function_slot( const player_movement_signal::slot_type& slot )
{
    _player_movement_signal.connect(slot);
}

void GameController::add_player_particle_launch_event_function_slot( const player_particle_launch_signal::slot_type& slot )
{
    _player_particle_launch_signal.connect(slot);
}

void GameController::assign_slots_to_events()
{
    //add_player_movement_event_function_slot(_player->handle_movement);
    add_player_movement_event_function_slot( boost::bind(&Player::handle_movement, _player, _1) );

    add_player_particle_launch_event_function_slot( boost::bind(&GameController::add_player_projectile, this, _1, _2, _3, _4, _5, _6) );

    //_player_movement_signal.connect( boost::bind(&Player::handle_movement, _player, _1, _2) );

    //_player_movement_signal.connect( boost::bind(&Player::handle_movement, Player, _1)  );
}

void GameController::announce_player_move(player_movement_direction p_move_direction)
{
    _player_movement_signal(p_move_direction);
}

void GameController::announce_player_particle_launch()
{
    int screen_height = SH; ///ASSIGNED FROM THE GLOBAL VARIABLE SCREEN HEIGHT.

    //So that the projectile is launched in a verticle line. Dynamic, so that it could be changed later, if need be.
    _player_particle_launch_signal(_player->get_x()-correction_player_projectile_launch_x_start_cords, _player->get_y()-correction_player_projectile_launch_y_start_cords, _player->get_x()-correction_player_projectile_launch_x_start_cords, (float)screen_height, _player->get_damage_done_with_projectile(), _player->get_projectile_size());

}

void GameController::init(float playerX, float playerY, _gamemode_enum _gamemode)
{
  _player = new Player(playerX, playerY, PLAYER_INITIAL_SIZE);
  //_player->set_position(playerX,playerY);

    switch (_gamemode)
    {
        case GM_EASY:
            _player->set_health(50);
            break;
        case GM_HARD:
            _player->set_health(15);
            break;
    }
}

void GameController::spawn_enemy(float where_to_spawn_x, float where_to_spawn_y)
{
    NPC* npc = new NPC(200, 200, NPC_INITIAL_SIZE);
    npc->set_position(where_to_spawn_x,where_to_spawn_y);  // set a random position
    _npcs.push_back(npc);  // add the NPC to the GameController’s npc list.
}

void GameController::update()
{
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    glLoadIdentity( );

    float delta_time = timer.getDeltaTime();

    collision_detection(delta_time);

    _player->update(delta_time);  // run the player update function, to peform any internal logic
    _player->draw(); // render the player

    //iterate over all the NPCs and call their update functions.
    std::list<NPC*>::iterator it;

    for (it = _npcs.begin(); it != _npcs.end(); ++it)
    {
        (*it)->update(delta_time); // logic updates, like movement and whatnot.
        (*it)->draw(); // draw the latest NPC state.
    }

    update_projectiles(delta_time);

    SDL_GL_SwapBuffers( );
}

void GameController::collision_detection(float delta_time)
{
    // Check whether the player collided with any of the enemies.
            // If player collided with enemy, player->lose_health( enemy->get_dmg() );

     // Check whether the player collided with any of the powerups.
    // If player collided with powerups, player->add_health( powerup->get_boost() );
}

void GameController::add_enemy_projectile(float x, float y, float where_to_to_x, float where_to_to_y, int damage, float size)
{
    enemy_projectiles.push_back( enemyProjectile(x,y,where_to_to_x,where_to_to_y,damage, size) );
}

void GameController::add_player_projectile(float x, float y, float where_to_to_x, float where_to_to_y, int damage, float size)
{
    player_projectiles.push_back( playerProjectile(x,y,where_to_to_x,where_to_to_y,damage, size) );
}

void GameController::update_projectiles(float delta_time)
{
    //float delta_time = timer.getDeltaTime();

    std::list<enemyProjectile>::iterator e_it;
    std::list<playerProjectile>::iterator p_it;

    for (e_it = enemy_projectiles.begin(); e_it != enemy_projectiles.end(); ++e_it)
    {
        e_it->update(delta_time); // run the projectile’s update function.
        e_it->draw();
        // Each projectile can potentially have different ways to update itself. Some can move in a straight line, others can zig zag, move in an arch, or circles, or even home in on the player.
    }

    for (p_it = player_projectiles.begin(); p_it != player_projectiles.end(); ++p_it)
    {
        p_it->update(delta_time); // run the projectile’s update function.
        p_it->draw();
        // Each projectile can potentially have different ways to update itself. Some can move in a straight line, others can zig zag, move in an arch, or circles, or even home in on the player.
    }
}


/* function to release/destroy our resources and restoring the old desktop */
void Quit( int returnCode )
{
    /* clean up the window */
    SDL_Quit( );

    /* and exit appropriately */
    exit( returnCode );
}

bool resizeWindow( int w, int h )
{
    if(h==0)h=1; //Sigh.. please check your if's eh?
    glViewport(0,0,w,h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    //gluPerspective(45.0, (GLfloat)w/(GLfloat)h, 0.1, 100.0);
    glOrtho(0, w, h, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    return(true);
}

int initGL( )
{

    glShadeModel( GL_SMOOTH );

    //glLineWidth(LINE_WIDTH_);

    srand(time(NULL));

    //SDL_WM_GrabInput(SDL_GRAB_ON);

    //SDL_ShowCursor(0);

    /* Set the background black */
    glClearColor( 0.0f, 0.0f, 0.0f, 0.0f );
    //BuildFont();
    //glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
   // glBlendFunc( GL_SRC_ALPHA, GL_ONE );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    /* Depth buffer setup */
    glClearDepth( 1.0f );

    /* Enables Depth Testing */
    glEnable( GL_DEPTH_TEST );

    /* The Type Of Depth Test To Do */
    glDepthFunc( GL_LEQUAL );

    /* Really Nice Perspective Calculations */
    //glHint( GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST );

    if(!loadTextures()){
    cout << "FAILURE LOADING TEXTURES.";
    exit(1);
    }

    glEnable(GL_TEXTURE_2D);
    return(true);
}


void handleKeyPress( SDL_keysym *keysym)
{
    switch ( keysym->sym )
    {
            case SDLK_LEFT: case SDLK_a:
                    //game_controller->announce_player_move(P_MOVE_LEFT);
                break;
            case SDLK_RIGHT: case SDLK_d:
                    //game_controller->announce_player_move(P_MOVE_RIGHT);
                break;
            case SDLK_UP: case SDLK_w:
                    //game_controller->announce_player_move(P_MOVE_UP);
                break;
            case SDLK_DOWN: case SDLK_s:
                    //game_controller->announce_player_move(P_MOVE_DOWN);
                break;
            default:
            break;
	}

    switch ( keysym->sym )
	{
	case SDLK_ESCAPE:
	    /* ESC key was pressed */
	    Quit( 0 );
	    break;
	case SDLK_F1:
	    /* F1 key was pressed
	     * this toggles fullscreen mode
	     */
	    SDL_WM_ToggleFullScreen( surface );
	    break;
    case SDLK_w :
        break;
	default:
	    break;
	}

    return;
}

void KeyUpEvents( SDL_keysym *keysym) //Is this how it should be done?
{
    switch( keysym->sym ){
            case SDLK_LEFT: case SDLK_a:
                   // game_controller->announce_player_move(P_MOVE_LEFT);
                break;
            case SDLK_RIGHT: case SDLK_d:
                   // game_controller->announce_player_move(P_MOVE_RIGHT);
                break;
            case SDLK_UP: case SDLK_w:
                   // game_controller->announce_player_move(P_MOVE_UP);
                break;
            case SDLK_DOWN: case SDLK_s:
                   // game_controller->announce_player_move(P_MOVE_DOWN);
                break;
            default:
            break;
    }
}

void mouseEvents(SDL_Event *eventx)
{
    int mx;
    int my;

    SDL_GetMouseState(&mx, &my);

    if(eventx->button.button == SDL_BUTTON_LEFT){
    //  xCan->Walls.addLink(mx,my);
       // cout << "\nMX: " << mx << "\nMY: " << my;
      //  xCan->CannonBall.setCourse(mx,my);
    }

}

void initCoutRedirecting() { ///CAUSE OF PROBLAMATIC EXIT. REMOVE WHEN DEBUGGING IS FINISHED.
//SDL_Init(/*whatever*/);
//AllocConsole();
freopen("CONOUT$", "wb", stdout);
freopen("CONOUT$", "wb", stderr);
freopen("CONIN$", "rb", stdin);
/*...*/
}

int main ( int argc, char** argv )
{
    // initialize SDL video

   // float MXFLOATCONVERSION = 0;

    //initCoutRedirecting();

    GameController::lease game_controller;
    game_controller->init(player_start_position_x, player_start_position_y, GM_HARD);
    game_controller->assign_slots_to_events();

    int videoFlags;
    /* main loop variable */
    int done = false;
    /* used to collect events */
    SDL_Event event;
    /* this holds some info about our display */
    const SDL_VideoInfo *videoInfo;
    /* whether or not the window is active */
    int isActive = true;

    /* initialize SDL */
    if ( SDL_Init( SDL_INIT_VIDEO ) < 0 )
	{
	    fprintf( stderr, "Video initialization failed: %s\n",
		     SDL_GetError( ) );
	    Quit( 1 );
	}

    /* Fetch the video info */
    videoInfo = SDL_GetVideoInfo( );

    if ( !videoInfo )
	{
	    fprintf( stderr, "Video query failed: %s\n",
		     SDL_GetError( ) );
	    Quit( 1 );
	}

    /* the flags to pass to SDL_SetVideoMode */
    videoFlags  = SDL_OPENGL;          /* Enable OpenGL in SDL */
    videoFlags |= SDL_GL_DOUBLEBUFFER; /* Enable double buffering */
    videoFlags |= SDL_HWPALETTE;       /* Store the palette in hardware */
    videoFlags |= SDL_RESIZABLE;       /* Enable window resizing */
    videoFlags != SDL_FULLSCREEN;

    /* This checks to see if surfaces can be stored in memory */
    if ( videoInfo->hw_available )
	videoFlags |= SDL_HWSURFACE;
    else
	videoFlags |= SDL_SWSURFACE;

    /* This checks if hardware blits can be done */
    if ( videoInfo->blit_hw )
	videoFlags |= SDL_HWACCEL;

    /* Sets up OpenGL double buffering */
    SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );

    /* get a SDL surface */
    surface = SDL_SetVideoMode( SW, SH, SBPP,
				videoFlags );

    /* Verify there is a surface */
    if ( !surface )
	{
	    fprintf( stderr,  "Video mode set failed: %s\n", SDL_GetError( ) );
	    Quit( 1 );
	}

    /* initialize OpenGL */
    if ( initGL( ) == false )
	{
	    fprintf( stderr, "Could not initialize OpenGL.\n" );
	    Quit( 1 );
	}

    /* Resize the initial window */
    resizeWindow( SW, SH );

    /* wait for events */
    while ( !done )
	{
	    /* handle the events in the queue */

	    while ( SDL_PollEvent( &event ) )
		{
		    switch( event.type )
			{
			    if(beforeResizeSW != SW || beforeResizeSH != SH)
                    surface = SDL_SetVideoMode( SW, SH, SBPP, videoFlags ); //SDL_NOFRAME | SDL_FULLSCREEN

			case SDL_ACTIVEEVENT:
			    /* Something's happend with our focus
			     * If we lost focus or we are iconified, we
			     * shouldn't draw the screen
			     */
			    if ( event.active.gain == 0 )
				isActive = false;
			    else
				isActive = true;
			    break;
			case SDL_VIDEORESIZE:
			    /* handle resize event */
			    beforeResizeSW = SW;
                beforeResizeSH = SH;

			    SW = event.resize.w;
                SH = event.resize.h;

			    if ( !surface )
				{
				    fprintf( stderr, "Could not get a surface after resize: %s\n", SDL_GetError( ) );
				    Quit( 1 );
				}
                resizeWindow( SW, SH );
			    break;
			case SDL_KEYDOWN:
			    /* handle key presses */
			    //handleKeyPress( &event.key.keysym, game_controller );
			    //ENTIRE handleKeyPress function here.
			    switch ( event.key.keysym.sym )
                {
                    case SDLK_LEFT: case SDLK_a:
                            game_controller->announce_player_move(P_MOVE_LEFT);
                        break;
                    case SDLK_RIGHT: case SDLK_d:
                            game_controller->announce_player_move(P_MOVE_RIGHT);
                        break;
                    case SDLK_UP: case SDLK_w:
                            game_controller->announce_player_move(P_MOVE_UP);
                        break;
                    case SDLK_DOWN: case SDLK_s:
                            game_controller->announce_player_move(P_MOVE_DOWN);
                        break;
                    case SDLK_LCTRL:
                            game_controller->announce_player_particle_launch();
                        break;
                    default:
                        break;
                }

                switch ( event.key.keysym.sym )
                {
                    case SDLK_ESCAPE:
                        /* ESC key was pressed */
                        Quit( 0 );
                        break;
                    case SDLK_F1:
                        /* F1 key was pressed
                        * this toggles fullscreen mode
                        */
                        SDL_WM_ToggleFullScreen( surface );
                        break;
                    case SDLK_w :
                        break;
                    default:
                        break;
                }
			    break;
            case SDL_KEYUP:
                //KeyUpEvents( &event.key.keysym, game_controller );
                switch( event.key.keysym.sym )
                {
                    case SDLK_LEFT: case SDLK_a:
                         game_controller->announce_player_move(NO_MOVEMENT);
                        break;
                    case SDLK_RIGHT: case SDLK_d:
                         game_controller->announce_player_move(NO_MOVEMENT);
                        break;
                    case SDLK_UP: case SDLK_w:
                         game_controller->announce_player_move(NO_MOVEMENT);
                        break;
                    case SDLK_DOWN: case SDLK_s:
                         game_controller->announce_player_move(NO_MOVEMENT);
                        break;
                    default:
                        break;
                }
                break;
            case SDL_MOUSEBUTTONDOWN:
                //mouseEvents(&event);
                break;
			case SDL_QUIT:
			    /* handle quit requests */
			    done = true;
			    break;
			default:
			    break;
			}
		}
    /* Draw it to the screen */

	    /* draw the scene */
	    if(!done)
	    {
	        //glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
            //glLoadIdentity( );

            //game_controller->update_projectiles();
            game_controller->update();
            //game_controller->update_projectiles();
            //game_controller->spawn_enemy();

            //SDL_GL_SwapBuffers( );

        }
		//drawGLScene( );
		//firstModel->draw(SDL_GetTicks());
	}

    /* clean ourselves up and exit */
    Quit( 0 );
    /* Should never get here */
    return( 0 );
}
