
#include<SDL2/SDL.h>
#include<SDL2/SDL_image.h>
#include<iostream>
#include<map>
#include<vector>
#include <stdlib.h>
#include<SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>
#include <sstream>

#include "TinyXml/tinyxml.h"

using namespace std;

SDL_Window* window;
SDL_Renderer* renderer;
SDL_Event Event;
SDL_Texture *background, *background2, *texture_tile,*character,*character2, *star, *Message, *blocker;
SDL_Rect rect_background, rect_background2, rect_character, rect_tile, rect_tileset, rect_character2, Message_rect, rect_star, rect_blocker;
SDL_Surface* surfaceMessage;
TTF_Font *Sans;
SDL_Color Color;


class Warp
{
public:
    int x;
    int y;
    string mapa;
    SDL_Rect rect;
    Warp(int x,int y,string mapa,SDL_Rect rect)
    {
        this->x=x;
        this->y=y;
        this->mapa=mapa;
        this->rect = rect;
    }
};

void showTTF(string str, int espacio)
{
    SDL_Surface* surfaceMessage = TTF_RenderText_Solid(Sans, str.c_str(), Color); // as TTF_RenderText_Solid could only be used on SDL_Surface then you have to create the surface first

    Message = SDL_CreateTextureFromSurface(renderer, surfaceMessage); //now you can convert it into a texture

    Message_rect.x = espacio;  //controls the rect's x coordinate
    Message_rect.y = 0; // controls the rect's y coordinte
    SDL_QueryTexture(Message, NULL, NULL, &Message_rect.w, &Message_rect.h);

}

bool collision(SDL_Rect r1, SDL_Rect r2)
{
    if(r1.x+r1.w <= r2.x)
        return false;
    if(r1.x >= r2.x+r2.w)
        return false;
    if(r1.y+r1.h<=r2.y)
        return false;
    if(r1.y >= r2.y+r2.h)
        return false;

    return true;
}

vector<Warp*> getWarps(string archivo)//Esto lee tiled
{
    TiXmlDocument doc(archivo.c_str());
    bool loadOkay = doc.LoadFile();
    TiXmlElement *map_node = doc.FirstChild("map")->ToElement();
    TiXmlNode*objectgroup_node = map_node->FirstChild("objectgroup");

    vector<Warp*> respuesta;

    if(objectgroup_node==NULL)
        return respuesta;

    for(TiXmlNode*object_node = objectgroup_node->FirstChild("object");
        object_node!=NULL;
        object_node=object_node->NextSibling("object"))
    {
        int x,y;
        string mapa;
        SDL_Rect rect;
        rect.x = atoi(object_node->ToElement()->Attribute("x"));
        rect.y = atoi(object_node->ToElement()->Attribute("y"));
        rect.w = atoi(object_node->ToElement()->Attribute("width"));
        rect.h = atoi(object_node->ToElement()->Attribute("height"));

        TiXmlNode*properties = object_node->FirstChild("properties");
        for(TiXmlNode*property = properties->FirstChild("property");
            property!=NULL;
            property=property->NextSibling("property"))
        {
            if(strcmp(property->ToElement()->Attribute("name"),"x")==0)
            {
                cout<<"X:"<<property->ToElement()->Attribute("value")<<endl;
                x=atoi(property->ToElement()->Attribute("value"));
            }
            if(strcmp(property->ToElement()->Attribute("name"),"y")==0)
            {
                y=atoi(property->ToElement()->Attribute("value"));
            }
            if(strcmp(property->ToElement()->Attribute("name"),"mapa")==0)
            {
                mapa=property->ToElement()->Attribute("value");
            }
        }
        Warp*warp = new Warp(x,y,mapa,rect);
        cout<<warp->x<<endl;
        cout<<warp->y<<endl;
        cout<<warp->mapa<<endl;
        cout<<warp->rect.x<<endl;
        cout<<warp->rect.y<<endl;
        cout<<warp->rect.w<<endl;
        cout<<warp->rect.h<<endl;
        respuesta.push_back(warp);
    }

    return respuesta;
}

vector<int> getMapa(string archivo,int layer)//esto divide cuadro por cuadro
{
    vector<int> mapa;
    TiXmlDocument doc(archivo.c_str());
    bool loadOkay = doc.LoadFile();
    TiXmlElement *map_node = doc.FirstChild("map")->ToElement();
    TiXmlNode*layer_node_temp = map_node->FirstChild("layer");
    for(int i=1;i<layer;i++)
        layer_node_temp=layer_node_temp->NextSibling("layer");

    TiXmlElement *layer_node = layer_node_temp->ToElement();

    for(TiXmlNode *tile_node = layer_node->FirstChild("data")->FirstChild("tile");
        tile_node!=NULL;
        tile_node=tile_node->NextSibling("tile"))
    {
        mapa.push_back(atoi(tile_node->ToElement()->Attribute("gid")));
    }
    return mapa;
}

void dibujarLayer(SDL_Renderer* renderer,vector<int>mapa)//dibuja cuadro por cuadro
{
    int x_pantalla = 0;
    int y_pantalla = 0;
    for(int i=0;i<mapa.size();i++)
    {
        int x = 0;
        int y = 0;
        for(int acum = 1;acum<mapa[i];acum++)
        {
            x+=32;
            if(acum%16==0)
            {
                y+=32;
                x=0;
            }
        }

    //            rect_tile.x = 32*(mapa[i]%16-1);
    //            rect_tile.y = 32*(mapa[i]/16);
        rect_tile.x = x;
        rect_tile.y = y;
        rect_tile.w = 32;
        rect_tile.h = 32;

        //cout<<rect_tile.x<<","<<rect_tile.y<<endl;

        rect_tileset.x = x_pantalla;
        rect_tileset.y = y_pantalla;

        if(mapa[i]!=0)
            SDL_RenderCopy(renderer, texture_tile, &rect_tile, &rect_tileset);

        x_pantalla+=32;
        if(x_pantalla>=320)
        {
            x_pantalla=0;
            y_pantalla+=32;//Cuando
        }
    }
}

bool collisionLayer(vector<int>collision_map,SDL_Rect rect_personaje)//que cambio de aqui?
{
//    rect_personaje.x+=1;
//    rect_personaje.y+=1;
//    rect_personaje.w-=2;
//    rect_personaje.h-=2;

    int x_pantalla = 0;
    int y_pantalla = 0;
    for(int i=0;i<collision_map.size();i++)
    {
        int x = 0;
        int y = 0;
        for(int acum = 1;acum<collision_map[i];acum++)
        {
            x+=32;
            if(acum%16==0)
            {
                y+=32;
                x=0;
            }
        }

    //            rect_tile.x = 32*(mapa[i]%16-1);
    //            rect_tile.y = 32*(mapa[i]/16);
        rect_tile.x = x;
        rect_tile.y = y;
        rect_tile.w = 32;
        rect_tile.h = 32;

        //cout<<rect_tile.x<<","<<rect_tile.y<<endl;

        rect_tileset.x = x_pantalla;
        rect_tileset.y = y_pantalla;
        rect_tileset.w = 32;
        rect_tileset.h = 32;

        if(collision_map[i]!=0)
        {
            if(collision(rect_personaje,rect_tileset))
            {
                return true;
            }
        }

        x_pantalla+=32;
        if(x_pantalla>=320)
        {
            x_pantalla=0;
            y_pantalla+=32;
        }
    }
    return false;
}

int main( int argc, char* args[] )
{
    //Init SDL
    if(SDL_Init(SDL_INIT_EVERYTHING) < 0)
    {
        return 10;
    }
    //Creates a SDL Window
    if((window = SDL_CreateWindow("Image Loading", 100, 100, 320/*WIDTH*/, 320/*HEIGHT*/, SDL_WINDOW_RESIZABLE | SDL_RENDERER_PRESENTVSYNC)) == NULL)
    {
        return 20;
    }
    //SDL Renderer
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED );
    if (renderer == NULL)
    {
        std::cout << SDL_GetError() << std::endl;
        return 30;
    }

    //TTF init
    TTF_Init();
    Sans = TTF_OpenFont("match.ttf", 17); //this opens a font style and sets a size
    Color = {255, 255, 255};

    //------------------------------------------------
    //|>>>>>>>>>>>>>>>>>Init textures<<<<<<<<<<<<<<<<|
    //------------------------------------------------

    //Background
    int w=0,h=0;
    background = IMG_LoadTexture(renderer,"fondo.png");
    SDL_QueryTexture(background, NULL, NULL, &w, &h);
    rect_background.x = 0;
    rect_background.y = 0;
    rect_background.w = w;
    rect_background.h = h;

    //Background start
    int w2=0,h2=0;
    background2 = IMG_LoadTexture(renderer,"start.png");
    SDL_QueryTexture(background2, NULL, NULL, &w, &h);
    bool start = false;
    rect_background2.x = 0;
    rect_background2.y = 0;
    rect_background2.w = w;
    rect_background2.h = h;

    //Personaje 1
    character = IMG_LoadTexture(renderer, "personaje/down1.png");
    SDL_QueryTexture(character, NULL, NULL, &w, &h);
    rect_character.x = 100;
    rect_character.y = 100;
    rect_character.w = w-4;
    rect_character.h = h-4;

    //Personaje 2
    character2 = IMG_LoadTexture(renderer, "personaje/down12.png");
    SDL_QueryTexture(character2, NULL, NULL, &w, &h);
    rect_character2.x = 200;
    rect_character2.y = 200;
    rect_character2.w = w-4;
    rect_character2.h = h-4;

    //star
    star = IMG_LoadTexture(renderer, "personaje/star.png");
    SDL_QueryTexture(star, NULL, NULL, &w, &h);
    rect_star.x = rand() % 289;
    rect_star.y = rand() % 287;
    rect_star.w = w-4;
    rect_star.h = h-4;

    //Blocker
    blocker = IMG_LoadTexture(renderer, "patito.png");
    SDL_QueryTexture(blocker, NULL, NULL, &w, &h);
    rect_blocker.x = 0;
    rect_blocker.y = 0;
    rect_blocker.w = 32;
    rect_blocker.h = 32;

    //------------------------------------------------
    //|>>>>>>>>>>>>>>>>>Init Variables<<<<<<<<<<<<<<<<|
    //------------------------------------------------

    //Personaje 1 y 2
    int animation_velocity = 15;
    int velocity = 3;
    int frame = 0;
    ostringstream varTTF;
    varTTF << "0 - 0";
    bool reload = true;
    int ganador = 0;

    //Personaje 1
    int starCont = 0;
    char orientation = 'd';// d u l r
    int current_sprite = 0;
    map<char,vector<SDL_Texture*> >sprites;
    sprites['u'].push_back(IMG_LoadTexture(renderer, "personaje/up1.png"));
    sprites['u'].push_back(IMG_LoadTexture(renderer, "personaje/up2.png"));
    sprites['d'].push_back(IMG_LoadTexture(renderer, "personaje/down1.png"));
    sprites['d'].push_back(IMG_LoadTexture(renderer, "personaje/down2.png"));
    sprites['l'].push_back(IMG_LoadTexture(renderer, "personaje/left1.png"));
    sprites['l'].push_back(IMG_LoadTexture(renderer, "personaje/left2.png"));
    sprites['r'].push_back(IMG_LoadTexture(renderer, "personaje/right1.png"));
    sprites['r'].push_back(IMG_LoadTexture(renderer, "personaje/right2.png"));

    //Personaje 2
    int starCont2 = 0;
    char orientation2 = 'd';// d u l r
    int current_sprite2 = 0;
    map<char,vector<SDL_Texture*> >sprites2;
    sprites2['u'].push_back(IMG_LoadTexture(renderer, "personaje/up12.png"));
    sprites2['u'].push_back(IMG_LoadTexture(renderer, "personaje/up22.png"));
    sprites2['d'].push_back(IMG_LoadTexture(renderer, "personaje/down12.png"));
    sprites2['d'].push_back(IMG_LoadTexture(renderer, "personaje/down22.png"));
    sprites2['l'].push_back(IMG_LoadTexture(renderer, "personaje/left12.png"));
    sprites2['l'].push_back(IMG_LoadTexture(renderer, "personaje/left22.png"));
    sprites2['r'].push_back(IMG_LoadTexture(renderer, "personaje/right12.png"));
    sprites2['r'].push_back(IMG_LoadTexture(renderer, "personaje/right22.png"));


//    map<char,vector<SDL_Texture*> >sprites;
//    sprites['u'].push_back(IMG_LoadTexture(renderer, "personaje/up1.png"));
//    sprites['u'].push_back(IMG_LoadTexture(renderer, "personaje/up2.png"));
//    sprites['d'].push_back(IMG_LoadTexture(renderer, "personaje/down1.png"));
//    sprites['d'].push_back(IMG_LoadTexture(renderer, "personaje/down2.png"));
//    sprites['l'].push_back(IMG_LoadTexture(renderer, "personaje/left1.png"));
//    sprites['l'].push_back(IMG_LoadTexture(renderer, "personaje/left2.png"));
//    sprites['r'].push_back(IMG_LoadTexture(renderer, "personaje/right1.png"));
//    sprites['r'].push_back(IMG_LoadTexture(renderer, "personaje/right2.png"));

//    SDL_QueryTexture(sprites['u'][0], NULL, NULL, &w, &h);//que es?
//    rect_character.x = 100;
//    rect_character.y = 100;
//    rect_character.w = w;
//    rect_character.h = h;

//    texture_npc = IMG_LoadTexture(renderer,"npc.png");
//    SDL_QueryTexture(texture_npc, NULL, NULL, &w, &h);
//    rect_npc.x = 300;
//    rect_npc.y = 100;
//    rect_npc.w = w;
//    rect_npc.h = h;

    texture_tile = IMG_LoadTexture(renderer,"tile/crypt.png");//que va de parametro
    rect_tile.x = 32*4;
    rect_tile.y = 32*5;
    rect_tile.w = 32;
    rect_tile.h = 32;

//      Esto borra el mapa
//    SDL_QueryTexture(texture_npc, NULL, NULL, &w, &h);//que es?
//    rect_tileset.x = 0;
//    rect_tileset.y = 0;
//    rect_tileset.w = w;
//    rect_tileset.h = h;

    map<string,vector<int> >mapas_down;
    map<string,vector<int> >mapas_over;
    map<string,vector<int> >mapas_collision;
    map<string,vector<Warp*> >warps;

    mapas_down["mapa1"]=getMapa("tile/mapa1.tmx",1);
    mapas_over["mapa1"]=getMapa("tile/mapa1.tmx",2);
    mapas_collision["mapa1"]=getMapa("tile/mapa1.tmx",3);
    warps["mapa1"] = getWarps("tile/mapa1.tmx");

    mapas_down["mapa1"]=getMapa("tile/mapa1.tmx",1);
    mapas_over["mapa1"]=getMapa("tile/mapa1.tmx",2);
    mapas_collision["mapa1"]=getMapa("tile/mapa1.tmx",3);
    warps["mapa1"] = getWarps("tile/mapa1.tmx");

    mapas_down["mapa2"]=getMapa("tile/mapa2.tmx",1);
    mapas_over["mapa2"]=getMapa("tile/mapa2.tmx",2);
    mapas_collision["mapa2"]=getMapa("tile/mapa2.tmx",3);
    warps["mapa2"] = getWarps("tile/mapa2.tmx");

    mapas_down["mapa3"]=getMapa("tile/mapa3.tmx",1);
    mapas_over["mapa3"]=getMapa("tile/mapa3.tmx",2);
    mapas_collision["mapa3"]=getMapa("tile/mapa3.tmx",3);
    warps["mapa3"] = getWarps("tile/mapa3.tmx");

    mapas_down["mapa4"]=getMapa("tile/mapa4.tmx",1);
    mapas_over["mapa4"]=getMapa("tile/mapa4.tmx",2);
    mapas_collision["mapa4"]=getMapa("tile/mapa4.tmx",3);
    warps["mapa4"] = getWarps("tile/mapa4.tmx");

    mapas_down["mapa5"]=getMapa("tile/mapa5.tmx",1);
    mapas_over["mapa5"]=getMapa("tile/mapa5.tmx",2);
    mapas_collision["mapa5"]=getMapa("tile/mapa5.tmx",3);
    warps["mapa5"] = getWarps("tile/mapa5.tmx");

    mapas_down["mapa6"]=getMapa("tile/mapa6.tmx",1);
    mapas_over["mapa6"]=getMapa("tile/mapa6.tmx",2);
    mapas_collision["mapa6"]=getMapa("tile/mapa6.tmx",3);
    warps["mapa6"] = getWarps("tile/mapa6.tmx");

    string mapa_actual="mapa1";

    //------------------------------------------------
    //|>>>>>>>>>>>>>>>>>Init Sonido<<<<<<<<<<<<<<<<<<|
    //------------------------------------------------
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2,2048);

    Mix_Music *musica= Mix_LoadMUS("Kalimba.mp3");
    Mix_Chunk *sonido= Mix_LoadWAV("sound.wav");

    //Main Loop
    while(true)
    {
        while(SDL_PollEvent(&Event))
        {
            if(Event.type == SDL_QUIT)
            {
                return 0;
            }
        }

        //Init current key states
        const Uint8* currentKeyStates = SDL_GetKeyboardState( NULL );

        //Aqui la musica se repite sin fin -*<x>*-
        if(!Mix_PlayingMusic())
            Mix_PlayMusic(musica,1);

        //Pantalla de start
        if(currentKeyStates[ SDL_SCANCODE_SPACE ])
            start = true;

        //Personaje 1
        while(collisionLayer(mapas_collision[mapa_actual],rect_character))
        {
            rect_character.x = rand() % 289;
            rect_character.y = rand() % 287;
        }
        if(currentKeyStates[ SDL_SCANCODE_D ] && rect_character.x < 289)
        {
            rect_character.x+=velocity;
            while(collision(rect_character,rect_character2))
                rect_character.x-=velocity;
            while(collisionLayer(mapas_collision[mapa_actual],rect_character))
                rect_character.x-=1;
            orientation='r';
        }
        if(currentKeyStates[ SDL_SCANCODE_A ] && rect_character.x >= -2)
        {
            rect_character.x-=velocity;
            while(collision(rect_character,rect_character2))
                rect_character.x+=velocity;
            while(collisionLayer(mapas_collision[mapa_actual],rect_character))
                rect_character.x+=1;
            if(collision(rect_character,rect_blocker))
            {
                if(ganador != 1)
                    rect_character.x+=velocity;
                else
                {
                    starCont = 0;
                    starCont2 = 0;
                    varTTF.str("");
                    varTTF.clear();
                    varTTF << "0 - 0";
                    ganador = 0;
                    reload = true;
                }
            }
            orientation='l';
        }
        if(currentKeyStates[ SDL_SCANCODE_S ] && rect_character.y < 288)
        {
            rect_character.y+=velocity;
            while(collision(rect_character,rect_character2))
                rect_character.y-=velocity;
            while(collisionLayer(mapas_collision[mapa_actual],rect_character))
                rect_character.y-=1;
            orientation='d';
        }
        if(currentKeyStates[ SDL_SCANCODE_W ] && rect_character.y > 0)
        {
            rect_character.y-=velocity;
            while(collision(rect_character,rect_character2))
                rect_character.y+=velocity;
            while(collisionLayer(mapas_collision[mapa_actual],rect_character))
                rect_character.y+=1;
            if(collision(rect_character,rect_blocker))
            {
                if(ganador != 1)
                    rect_character.y+=velocity;
                else
                {
                    starCont = 0;
                    starCont2 = 0;
                    varTTF.str("");
                    varTTF.clear();
                    varTTF << "0 - 0";
                    ganador = 0;
                    reload = true;
                }
            }
            orientation='u';
        }
        //animacion del sprite
        if(frame%animation_velocity==0)
        {
            current_sprite++;
            if(current_sprite>1)
                current_sprite=0;
        }

        //Personaje 2
        while(collisionLayer(mapas_collision[mapa_actual],rect_character2))
        {
            rect_character2.x = rand() % 289;
            rect_character2.y = rand() % 287;
        }
        if(currentKeyStates[ SDL_SCANCODE_RIGHT ] && rect_character2.x < 289)
        {
            rect_character2.x+=velocity;
            while(collision(rect_character2,rect_character))
                rect_character2.x-=velocity;
            while(collisionLayer(mapas_collision[mapa_actual],rect_character2))
                rect_character2.x-=1;
            orientation2='r';
        }
        if(currentKeyStates[ SDL_SCANCODE_LEFT ] && rect_character2.x >= -2)
        {
            rect_character2.x-=velocity;
            while(collision(rect_character2,rect_character))
                rect_character2.x+=velocity;
            while(collisionLayer(mapas_collision[mapa_actual],rect_character2))
                rect_character2.x+=1;
            if(collision(rect_character2,rect_blocker))
            {
                if(ganador != 2)
                    rect_character2.x+=velocity;
                else
                {
                    starCont = 0;
                    starCont2 = 0;
                    varTTF.str("");
                    varTTF.clear();
                    varTTF << "0 - 0";
                    ganador = 0;
                    reload = true;
                }
            }
            orientation2='l';
        }
        if(currentKeyStates[ SDL_SCANCODE_DOWN ] && rect_character2.y < 287)
        {
            rect_character2.y+=velocity;
            while(collision(rect_character2,rect_character))
                rect_character2.y-=velocity;
            while(collisionLayer(mapas_collision[mapa_actual],rect_character2))
                rect_character2.y-=1;
            orientation2='d';
        }
        if(currentKeyStates[ SDL_SCANCODE_UP ] && rect_character2.y > 0)
        {
            rect_character2.y-=velocity;
            while(collision(rect_character2,rect_character))
                rect_character2.y+=velocity;
            while(collisionLayer(mapas_collision[mapa_actual],rect_character2))
                rect_character2.y+=1;
            if(collision(rect_character2,rect_blocker))
            {
                if(ganador != 2)
                    rect_character2.y+=velocity;
                else
                {
                    starCont = 0;
                    starCont2 = 0;
                    varTTF.str("");
                    varTTF.clear();
                    varTTF << "0 - 0";
                    ganador = 0;
                    reload = true;
                }
            }
            orientation2='u';
        }
        //animacion del sprite
        if(frame%animation_velocity==0)
        {
            current_sprite2++;
            if(current_sprite2>1)
                current_sprite2=0;
        }


//        >>>>>>>>>>Velocity control<<<<<<<<<<<<<<<<<
//        if(currentKeyStates[ SDL_SCANCODE_LSHIFT ])
//        {
//            velocity=6;
//            animation_velocity=10;
//        }else
//        {
//            velocity=3;
//            animation_velocity=20;
//        }
//

//------------------------------------------------
//|>>>>>>>>>>>>>>>>>>>>Estrella<<<<<<<<<<<<<<<<<<|
//------------------------------------------------
    while(collisionLayer(mapas_collision[mapa_actual],rect_star) || collision(rect_blocker,rect_star))
    {
        rect_star.x = rand() % 289;
        rect_star.y = rand() % 287;
    }
    if(collision(rect_character,rect_star))
    {
        Mix_PlayChannel(-1,sonido,0);
        do
        {
            rect_star.x = rand() % 289;
            rect_star.y = rand() % 287;
        }while(collisionLayer(mapas_collision[mapa_actual],rect_star) || collision(rect_blocker,rect_star));
        starCont++;
        reload = true;
        varTTF.str("");
        varTTF.clear();
        varTTF << starCont << " - " << starCont2;
        cout<<"Player 1"<<endl;
    }
    else if(collision(rect_character2,rect_star))
    {
        Mix_PlayChannel(-1,sonido,0);
        do
        {
            rect_star.x = rand() % 289;
            rect_star.y = rand() % 287;
        }while(collisionLayer(mapas_collision[mapa_actual],rect_star) || collision(rect_blocker,rect_star));
        starCont2++;
        reload = true;
        varTTF.str("");
        varTTF.clear();
        varTTF << starCont << " - " << starCont2;
        cout<<"Player 2"<<endl;
    }

    if(starCont == 10 && ganador == 0)
    {
        varTTF.str("");
        varTTF.clear();
        varTTF << "Jugador 1 ha Ganado!";
    }
    else if(starCont2 == 10 && ganador == 0)
    {
        varTTF.str("");
        varTTF.clear();
        varTTF << "Jugador 2 ha Ganado!";
    }

        for(int i=0;i<warps[mapa_actual].size();i++)
        {
            if(collision(warps[mapa_actual][i]->rect,rect_character))
            {
                rect_character.x=warps[mapa_actual][i]->x;
                rect_character.y=warps[mapa_actual][i]->y;
                mapa_actual=warps[mapa_actual][i]->mapa;
            }
            if(collision(warps[mapa_actual][i]->rect,rect_character2))
            {
                rect_character2.x=warps[mapa_actual][i]->x;
                rect_character2.y=warps[mapa_actual][i]->y;
                mapa_actual=warps[mapa_actual][i]->mapa;
            }
        }

        SDL_Delay(17);

        SDL_RenderCopy(renderer, background, NULL, &rect_background);

        dibujarLayer(renderer,mapas_down[mapa_actual]);

        SDL_RenderCopy(renderer, sprites[orientation][current_sprite], NULL, &rect_character);
        SDL_RenderCopy(renderer, sprites2[orientation2][current_sprite2], NULL, &rect_character2);
        SDL_RenderCopy(renderer, star, NULL, &rect_star);

        dibujarLayer(renderer,mapas_over[mapa_actual]);

        //dibujarLayer(renderer,collision_map);

        //SDL_RenderCopy(renderer, texture_npc, NULL, &rect_npc);
        if((starCont == 10 || starCont2 == 10) && ganador == 0)
        {
            showTTF(varTTF.str(), 50);
            SDL_RenderCopy(renderer, Message, NULL, &Message_rect);
            SDL_RenderPresent(renderer);
            SDL_Delay(2500);
            if(starCont == 10)
                ganador = 1;
            if(starCont2 == 10)
                ganador = 2;
            varTTF.str("");
            varTTF.clear();
            varTTF << starCont << " - " << starCont2;
        }
        else if(reload)
        {
            showTTF(varTTF.str(), 135);
            reload = false;
        }
        SDL_RenderCopy(renderer, Message, NULL, &Message_rect);
        if(!start)
            SDL_RenderCopy(renderer, background2, NULL, &rect_background2);
        SDL_RenderPresent(renderer);
        frame++;
    }

	return 0;
}
