#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_pixels.h>
#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_stdinc.h>
#include <SDL2/SDL_surface.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_clipboard.h>
#include <iostream>
#include <iterator>
#include <sstream>
#include <string>
#include <vector>
#include <fstream>
#include <map>

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) > (b) ? (b) : (a))

SDL_Window *window = nullptr;
SDL_Renderer *renderer = nullptr;
TTF_Font *font = nullptr;
SDL_Texture *textTexture = nullptr;

std::string fontPath = "/usr/share/fonts/TTF/";
std::string pth;

struct line
{
    std::string textBuffer = ""; 
};

struct cursor
{
    int x;
    int y;
};

struct rgba
{
    Uint8 r;
    Uint8 g;
    Uint8 b;
    Uint8 a;
};

rgba fontColor = { .r = 0, .g = 0, .b = 0, .a = 255};
rgba bgColor = { .r = 255, .g = 255, .b = 255, .a = 255};
cursor cursorPos = { .x = 0, .y = 0 };
int currLine = 0;
std::vector<line> lines;
line nullLine = {.textBuffer = ""};
std::string file = "";
std::map<std::string, std::string> shiftKeybindings;
int tabWidth = 4;
bool quit = false;
int fontSize = 20;

bool ShiftKeyPressed(SDL_Event e) { return e.key.keysym.mod & KMOD_SHIFT; }
bool CtrlKeyPressed(SDL_Event e) { return e.key.keysym.mod & KMOD_CTRL; }
bool BackspaceKeyPressed(SDL_Event e) { return e.key.keysym.sym == SDLK_BACKSPACE; }
bool EnterKeyPressed(SDL_Event e) { return e.key.keysym.sym == SDLK_RETURN; }
bool TabKeyPressed(SDL_Event e) { return e.key.keysym.sym == SDLK_TAB; }

void InvertColors()
{
    fontColor.r = 255 - fontColor.r;
    fontColor.g = 255 - fontColor.g;
    fontColor.b = 255 - fontColor.b;
    bgColor.r = 255 - bgColor.r;
    bgColor.g = 255 - bgColor.g;
    bgColor.b = 255 - bgColor.b;
}

bool IsOtherModKey(char c)
{
    return (c == '`' || c == '-' || c == '=' || c == '[' || c == ']' || c == '\\' || c == ';' || c == '\'' || c == ',' || c == '.' || c == '/');
}
void InitializeShiftKeybindings()
{
    shiftKeybindings["`"] = "~";
    shiftKeybindings["1"] = "!";
    shiftKeybindings["2"] = "@";
    shiftKeybindings["3"] = "#";
    shiftKeybindings["4"] = "$";
    shiftKeybindings["5"] = "%";
    shiftKeybindings["6"] = "^";
    shiftKeybindings["7"] = "&";
    shiftKeybindings["8"] = "*";
    shiftKeybindings["9"] = "(";
    shiftKeybindings["0"] = ")";
    shiftKeybindings["-"] = "_";
    shiftKeybindings["="] = "+";
    shiftKeybindings["["] = "{";
    shiftKeybindings["]"] = "}";
    shiftKeybindings["\\"] = "|";
    shiftKeybindings[";"] = ":";
    shiftKeybindings["'"] = "\"";
    shiftKeybindings[","] = "<";
    shiftKeybindings["."] = ">";
    shiftKeybindings["/"] = "?";
}

void InitializeSDL() 
{
    SDL_Init(SDL_INIT_VIDEO);
    TTF_Init();
    window = SDL_CreateWindow("Fluffy Text Editor :3", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
}

void LoadFont(const char* fontPath, int fontSize) 
{
    font = TTF_OpenFont(fontPath, fontSize);
    if (!font) 
    {
        std::cerr << "Failed to load font: " << TTF_GetError() << std::endl;
    }
}

int OpenFile(std::string file)
{
    std::ifstream input_stream(file);
    if (!input_stream) 
    {
        std::cerr << "Can't open input file!";
        return -1;
    }
    std::string fileLine;
    while (getline(input_stream, fileLine)) 
    {
        lines[currLine].textBuffer = fileLine;
        lines.push_back(nullLine);
        currLine++;
    }
    return 0;
}

int SaveFile()
{
    std::ofstream saveFile(file);
    if (!saveFile.is_open())
    {
        std::cerr << "Error saving to file" << std::endl;
        return -1;
    }

    for (auto line : lines)
    {
        saveFile << line.textBuffer << "\n";
    }
    saveFile.close();
    std::cout << "successfully wrote to file" << std::endl;

    return 0;
}

void InsertAtCursorPos(char letter)
{
    std::string l = std::string(1, letter);
    lines[cursorPos.y].textBuffer.insert(cursorPos.x, l);
}

void InsertAtCursorPos(std::string letter)
{
    lines[cursorPos.y].textBuffer.insert(cursorPos.x, letter);
}


void DeleteAtCursorPos()
{
    lines[cursorPos.y].textBuffer.erase(cursorPos.x - 1, 1);
}

void RenderText() {
    SDL_SetRenderDrawColor(renderer, bgColor.r, bgColor.g, bgColor.b, bgColor.a);
    SDL_RenderClear(renderer);

    SDL_Color textColor = {fontColor.r, fontColor.g, fontColor.b, fontColor.a};
    int y = -fontSize; // Starting y position for the first line
    for (int i = 0; i < lines.size(); i++) 
    {
        y += fontSize;
         std::string lineText;
        if (i == cursorPos.y)
        {
            std::string a = lines[i].textBuffer;
            a.insert(cursorPos.x, "|");
            lineText = std::to_string(i + 1) + " " + a;
        }
        else { lineText = std::to_string(i + 1) + "  " + lines[i].textBuffer; }
        SDL_Surface* textSurface = TTF_RenderText_Solid(font, lineText.c_str(), textColor);
        SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
        SDL_Rect textRect = {20, y, textSurface->w, textSurface->h};

        SDL_RenderCopy(renderer, textTexture, nullptr, &textRect);

        SDL_FreeSurface(textSurface);
        SDL_DestroyTexture(textTexture);
    }

    SDL_RenderPresent(renderer);
}

void CleanupSDL() 
{
    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();
}

void Paste()
{
    char* clipBoardText = SDL_GetClipboardText();
    if (clipBoardText)
    {
        std::stringstream ss(clipBoardText);
        std::string l;
        while (std::getline(ss, l))
        {
            InsertAtCursorPos(l);
            cursorPos.y++;
            cursorPos.x = 0;
            lines.insert(lines.begin() + cursorPos.y, nullLine);
        }
        SDL_free(clipBoardText);
    }

}

void HandleInput(SDL_Event event)
{
    if (event.type == SDL_QUIT) { quit = true; }
    else if (event.type == SDL_KEYDOWN) 
    {
        if (ShiftKeyPressed(event)) 
        {
            char pressedChar = event.key.keysym.sym;
            if (pressedChar >= 'a' && pressedChar <= 'z') 
            {
                pressedChar -= ('a' - 'A');
                InsertAtCursorPos(pressedChar);
                cursorPos.x++;
            }
            else if (pressedChar >= '0' && pressedChar <= '9') 
            {
                std::string s(1, pressedChar);
                std::string modified = shiftKeybindings[s];
                InsertAtCursorPos(modified);
                cursorPos.x++;
            }
            else if (IsOtherModKey(pressedChar))
            {
                std::string s(1, pressedChar);
                std::string modified = shiftKeybindings[s];
                InsertAtCursorPos(modified);
                cursorPos.x++;
            }
                        
        }
        else if (CtrlKeyPressed(event)) 
        { 
            if (event.key.keysym.sym == SDLK_s) { SaveFile(); } 
            else if (event.key.keysym.sym == SDLK_v) { Paste(); }
            else if (event.key.keysym.sym == SDLK_i) { InvertColors(); }
            else if (event.key.keysym.sym == SDLK_EQUALS)
            {
                fontSize++;
                LoadFont(pth.c_str(), fontSize);
            }
            else if (event.key.keysym.sym == SDLK_MINUS)
            {
                fontSize = MAX(1, fontSize - 1);
                LoadFont(pth.c_str(), fontSize);
            }
        }
        else if (BackspaceKeyPressed(event))  
        {
            if (!lines[cursorPos.y].textBuffer.empty() && cursorPos.x != 0) 
            { 
                DeleteAtCursorPos(); 
                cursorPos.x--;
            } 
            else 
            {
                if (cursorPos.y > 0)
                {
                    std::string move = lines[cursorPos.y].textBuffer;
                    lines.erase(lines.begin() + cursorPos.y);
                    cursorPos.y--;
                    lines[cursorPos.y].textBuffer += move;
                    cursorPos.x = lines[cursorPos.y].textBuffer.size();
                }
            }
        }
        else if (EnterKeyPressed(event)) 
        {
            std::string keep = lines[cursorPos.y].textBuffer.substr(0, cursorPos.x);
            std::string move = lines[cursorPos.y].textBuffer.substr(cursorPos.x);
            lines[cursorPos.y].textBuffer = keep;
            cursorPos.y++;
            cursorPos.x = 0;
            lines.insert(lines.begin() + cursorPos.y, line { .textBuffer = move }); 
        }
    
        else if (TabKeyPressed(event))
        {
            std::string spaces = "";
            for (int i = 0; i < tabWidth; i++) { spaces += " "; }
            InsertAtCursorPos(spaces);
            cursorPos.x += tabWidth;
        }
        else if (event.key.keysym.sym >= SDLK_SPACE && event.key.keysym.sym <= SDLK_z) 
        {
            InsertAtCursorPos(static_cast<char>(event.key.keysym.sym));
            cursorPos.x++;
        }
        else if (event.key.keysym.sym == SDLK_LEFT) { cursorPos.x = MAX(0, cursorPos.x - 1); }
        else if (event.key.keysym.sym == SDLK_RIGHT) { cursorPos.x = MIN(cursorPos.x + 1, lines[cursorPos.y].textBuffer.size()); }
        else if (event.key.keysym.sym == SDLK_DOWN) 
        { 
            if (cursorPos.y != lines.size() - 1)
            {
                cursorPos.x = MIN(cursorPos.x, lines[cursorPos.y + 1].textBuffer.size());
                cursorPos.y++;
            }

        }
        else if (event.key.keysym.sym == SDLK_UP) 
        {
            if (cursorPos.y != 0)
            {
                cursorPos.x = MIN(cursorPos.x, lines[cursorPos.y - 1].textBuffer.size());
                cursorPos.y--;
            }
        }

    }

}

int main(int argc, char* argv[]) 
{
    InitializeSDL();
    pth = fontPath + "CascadiaCode.ttf";
    LoadFont(pth.c_str(), fontSize); 
    lines.push_back(nullLine);
    if (argc > 1)
    {
        file = argv[1];
        OpenFile(file);
    }
    InitializeShiftKeybindings();

    while (!quit) 
    {
        SDL_Event event;
        while (SDL_PollEvent(&event)) { HandleInput(event); }
        RenderText();
    }
    CleanupSDL();
    return 0;
}

