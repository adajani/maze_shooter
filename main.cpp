/**
 * @file main.cpp
 * @brief Main entry point for the Maze Shooter game
 * @author Ahmed Dajani <adajani@iastate.edu>
 * @date 2025
 * @version 1.0
 * 
 * This is the main entry point for the Maze Shooter game. It initializes the
 * SDL library, creates a game window, and starts the main game loop.
 * to compile: g++ -o maze_shooter main.cpp `pkg-config --cflags --libs sdl2 SDL2_image SDL2_mixer SDL2_ttf` -lm 
 */

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>
#include <cmath>
#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <cstring>


const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;
const int MAP_WIDTH = 24;
const int MAP_HEIGHT = 24;
const double FOV = M_PI / 3;  // 60 degrees field of view
const double MOVE_SPEED = 0.05;
const double ROT_SPEED = 0.03;
const double JUMP_SPEED = 0.15;
const double GRAVITY = 0.01;
const double GROUND_HEIGHT = 0.0;

// Texture dimensions
const int TEXTURE_WIDTH = 64;
const int TEXTURE_HEIGHT = 64;
const int NUM_TEXTURES = 8;

// Gun animation constants
const int GUN_FRAMES = 4;  // idle, fire1, fire2, fire3
const int ANIMATION_SPEED = 100; // milliseconds per frame

// Game states
enum GameState {
    STATE_MENU,
    STATE_PLAYING,
    STATE_EXIT
};

// Menu items
enum MenuItem {
    MENU_NEW_GAME,
    MENU_EXIT,
    MENU_ITEM_COUNT
};

// Simple map layout (wall >=1, empty space = 0)
int worldMap[MAP_WIDTH][MAP_HEIGHT] = {
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,2,2,2,2,2,0,0,0,0,3,0,3,0,3,0,0,0,1},
    {1,0,0,0,0,0,2,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,2,0,0,0,2,0,0,0,0,3,0,0,0,3,0,0,0,1},
    {1,0,0,0,0,0,2,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,2,2,0,2,2,0,0,0,0,3,0,3,0,3,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,4,4,4,4,4,4,4,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,4,0,4,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,4,0,0,0,0,5,0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,4,0,4,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,4,4,4,4,4,4,4,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
};

class MazeShooter {
private:
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Texture* screenTexture;
    Uint32* screenBuffer;
    bool running;
    
    // Game state
    GameState currentState;
    int selectedMenuItem;
    
    // Fonts
    TTF_Font* titleFont;
    TTF_Font* menuFont;
    TTF_Font* copyrightFont;
    
    // Menu background
    SDL_Texture* menuBackground;
    
    // Audio
    Mix_Music* menuMusic;
    Mix_Music* gameMusic;
    Mix_Chunk* shootSound;
    bool musicEnabled;
    
    // Gun system
    SDL_Texture* gunSprites[GUN_FRAMES];
    int currentGunFrame;
    bool isShooting;
    Uint32 animationTimer;
    Uint32 lastAnimationTime;
    
    // Player position and direction
    double posX, posY;  // Player position
    double dirX, dirY;  // Direction vector
    double planeX, planeY;  // Camera plane
    
    // Jumping mechanics
    double cameraHeight;  // Camera height offset
    double verticalVelocity;  // Current vertical velocity
    bool isJumping;
    
    // FPS tracking
    Uint32 frameCount;
    Uint32 lastTime;
    double fps;
    
    // Input states
    bool keys[SDL_NUM_SCANCODES];
    
    // Optimized texture data
    Uint32 texture[NUM_TEXTURES][TEXTURE_WIDTH * TEXTURE_HEIGHT];
    
public:
    MazeShooter() : window(nullptr), renderer(nullptr), screenTexture(nullptr), screenBuffer(nullptr), 
                    titleFont(nullptr), menuFont(nullptr), copyrightFont(nullptr), menuBackground(nullptr),
                    menuMusic(nullptr), gameMusic(nullptr), shootSound(nullptr), musicEnabled(true), 
                    currentGunFrame(0), isShooting(false), animationTimer(0), 
                    currentState(STATE_MENU), selectedMenuItem(MENU_NEW_GAME), running(true) {
        // Initialize player position and direction
        posX = 22.0; posY = 12.0;  // Starting position
        dirX = -1.0; dirY = 0.0;   // Initial direction (facing left)
        planeX = 0.0; planeY = 0.66; // Camera plane (perpendicular to direction)
        
        // Initialize jumping mechanics
        cameraHeight = GROUND_HEIGHT;
        verticalVelocity = 0.0;
        isJumping = false;
        
        // Initialize FPS tracking
        frameCount = 0;
        lastTime = SDL_GetTicks();
        lastAnimationTime = SDL_GetTicks();
        fps = 0.0;
        
        // Initialize gun sprites to nullptr
        for (int i = 0; i < GUN_FRAMES; i++) {
            gunSprites[i] = nullptr;
        }
        
        // Initialize input states
        for (int i = 0; i < SDL_NUM_SCANCODES; i++) {
            keys[i] = false;
        }
        
        // Load textures from PNG files only
        loadTextures();
    }
    
    bool loadTextureFromPNG(int textureNum, const std::string& filename) {
        SDL_Surface* surface = IMG_Load(filename.c_str());
        if (!surface) {
            std::cerr << "Failed to load texture " << filename << ": " << IMG_GetError() << std::endl;
            return false;
        }
        
        // Convert surface to our desired format if needed
        SDL_Surface* convertedSurface = nullptr;
        if (surface->format->format != SDL_PIXELFORMAT_ARGB8888) {
            convertedSurface = SDL_ConvertSurfaceFormat(surface, SDL_PIXELFORMAT_ARGB8888, 0);
            SDL_FreeSurface(surface);
            surface = convertedSurface;
        }
        
        if (!surface) {
            std::cerr << "Failed to convert surface format for " << filename << std::endl;
            return false;
        }
        
        // Lock surface for pixel access
        SDL_LockSurface(surface);
        
        Uint32* pixels = (Uint32*)surface->pixels;
        int width = surface->w;
        int height = surface->h;
        
        // Copy and scale texture to our fixed size
        for (int y = 0; y < TEXTURE_HEIGHT; y++) {
            for (int x = 0; x < TEXTURE_WIDTH; x++) {
                // Scale coordinates to source image
                int srcX = (x * width) / TEXTURE_WIDTH;
                int srcY = (y * height) / TEXTURE_HEIGHT;
                
                // Ensure we don't go out of bounds
                if (srcX >= width) srcX = width - 1;
                if (srcY >= height) srcY = height - 1;
                
                texture[textureNum][TEXTURE_WIDTH * y + x] = pixels[srcY * width + srcX];
            }
        }
        
        SDL_UnlockSurface(surface);
        SDL_FreeSurface(surface);
        
        std::cout << "Successfully loaded texture " << textureNum << " from " << filename << std::endl;
        return true;
    }
    
    void createErrorTexture(int textureNum) {
        // Create a simple error texture (magenta and black checkerboard)
        for (int y = 0; y < TEXTURE_HEIGHT; y++) {
            for (int x = 0; x < TEXTURE_WIDTH; x++) {
                bool checker = ((x / 8) + (y / 8)) % 2;
                texture[textureNum][TEXTURE_WIDTH * y + x] = checker ? 0xFFFF00FF : 0xFF000000; // Magenta/Black
            }
        }
        std::cout << "Created error texture for slot " << textureNum << std::endl;
    }
    
    void loadTextures() {
        std::cout << "Loading textures from PNG files only..." << std::endl;
        
        // List of required texture filenames
        std::vector<std::string> textureFiles = {
            "", // Index 0 unused
            "textures/wall1.png",    // Brick wall
            "textures/wall2.png",    // Stone wall
            "textures/wall3.png",    // Blue wall
            "textures/wall4.png",    // White wall
            "textures/wall5.png",    // Wood wall
            "textures/wall6.png",    // Green wall
            "textures/wall7.png"     // Purple wall
        };
        
        // Try to load each texture from PNG file only
        bool allTexturesLoaded = true;
        for (int i = 1; i < NUM_TEXTURES; i++) {
            if (i < textureFiles.size() && !textureFiles[i].empty()) {
                if (!loadTextureFromPNG(i, textureFiles[i])) {
                    createErrorTexture(i);
                    allTexturesLoaded = false;
                }
            } else {
                createErrorTexture(i);
                allTexturesLoaded = false;
            }
        }
        
        if (allTexturesLoaded) {
            std::cout << "All textures loaded successfully!" << std::endl;
        } else {
            std::cout << "Some textures missing - using error textures" << std::endl;
        }
    }
    
    void loadFonts() {
        std::cout << "Loading fonts..." << std::endl;
        const std::string fontPath = "fonts/font.ttf"; // Use a default path for simplicity

        titleFont = TTF_OpenFont(fontPath.c_str(), 48);
        menuFont = TTF_OpenFont(fontPath.c_str(), 24);
        copyrightFont = TTF_OpenFont(fontPath.c_str(), 16);
        
        if (titleFont && menuFont && copyrightFont) {
            std::cout << "Fonts loaded successfully from: " << fontPath << std::endl;
        } else {
            std::cout << "Font loading failed: " << TTF_GetError() << std::endl;
        }
    }
    
    void loadMenuBackground() {
        std::cout << "Loading menu background..." << std::endl;
        const std::string path = "menu.jpg"; // Default fallback
        SDL_Surface* surface = IMG_Load(path.c_str());
        if (surface) {
            menuBackground = SDL_CreateTextureFromSurface(renderer, surface);
            SDL_FreeSurface(surface);
            
            if (menuBackground) {
                std::cout << "Menu background loaded from: " << path << std::endl;
                return;
            }
        }
    }
    
    void loadMusic() {
        if (!musicEnabled) {
            std::cout << "Audio disabled - skipping music loading" << std::endl;
            return;
        }
        
        std::cout << "Loading music..." << std::endl;
        
        // Load menu music
        menuMusic = Mix_LoadMUS("music/menu.mp3");
        if (!menuMusic) {
            std::cout << "Could not load music/menu.mp3: " << Mix_GetError() << std::endl;
        } else {
            std::cout << "Menu music loaded!" << std::endl;
        }
        
        // Load game music
        gameMusic = Mix_LoadMUS("music/background.mp3");
        if (!gameMusic) {
            std::cout << "Could not load music/background.mp3: " << Mix_GetError() << std::endl;
        } else {
            std::cout << "Game music loaded!" << std::endl;
        }
    }
    
    void loadGunAssets() {
        std::cout << "Loading gun sprites and sounds..." << std::endl;
        
        // Gun sprite filenames
        std::vector<std::string> gunFiles = {
            "gun/gun_idle.png",   // Frame 0 - Idle
            "gun/gun_fire1.png",  // Frame 1 - Fire frame 1
            "gun/gun_fire2.png",  // Frame 2 - Fire frame 2
            "gun/gun_fire3.png"   // Frame 3 - Fire frame 3
        };
        
        // Load gun sprites
        for (int i = 0; i < GUN_FRAMES; i++) {
            SDL_Surface* surface = IMG_Load(gunFiles[i].c_str());
            if (surface) {
                gunSprites[i] = SDL_CreateTextureFromSurface(renderer, surface);
                SDL_FreeSurface(surface);
                if (gunSprites[i]) {
                    std::cout << "Loaded gun sprite: " << gunFiles[i] << std::endl;
                }
            }
        }
        
        // Load shoot sound
        if (musicEnabled) {
            shootSound = Mix_LoadWAV("sounds/shoot.wav");
            if (shootSound) {
                std::cout << "Gun sound loaded!" << std::endl;
                Mix_VolumeChunk(shootSound, 64);
            }
        }
    }
    
    void playMusic(Mix_Music* music) {
        if (musicEnabled && music) {
            Mix_HaltMusic();
            if (Mix_PlayMusic(music, -1) == -1) {
                std::cout << "Could not play music: " << Mix_GetError() << std::endl;
            } else {
                Mix_VolumeMusic(64);
            }
        }
    }
    
    void startNewGame() {
        // Reset player position
        posX = 22.0; posY = 12.0;
        dirX = -1.0; dirY = 0.0;
        planeX = 0.0; planeY = 0.66;
        cameraHeight = GROUND_HEIGHT;
        verticalVelocity = 0.0;
        isJumping = false;
        
        // Switch to game state and music
        currentState = STATE_PLAYING;
        playMusic(gameMusic);
        std::cout << "Starting new game!" << std::endl;
    }
    
    void returnToMenu() {
        currentState = STATE_MENU;
        selectedMenuItem = MENU_NEW_GAME;
        playMusic(menuMusic);
        std::cout << "Returned to main menu" << std::endl;
    }
    
    inline Uint32 getTexturePixel(int textureNum, int texX, int texY) {
        texX &= (TEXTURE_WIDTH - 1);
        texY &= (TEXTURE_HEIGHT - 1);
        
        if (textureNum < 1 || textureNum >= NUM_TEXTURES) {
            return 0xFFFF00FF; // Magenta
        }
        
        return texture[textureNum][TEXTURE_WIDTH * texY + texX];
    }
    
    bool init() {
        if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
            std::cerr << "SDL initialization failed: " << SDL_GetError() << std::endl;
            return false;
        }
        
        // Initialize SDL_ttf
        if (TTF_Init() == -1) {
            std::cerr << "SDL_ttf initialization failed: " << TTF_GetError() << std::endl;
            return false;
        }
        
        // Initialize SDL_image (including JPG support)
        int imgFlags = IMG_INIT_PNG | IMG_INIT_JPG;
        if (!(IMG_Init(imgFlags) & imgFlags)) {
            std::cerr << "SDL_image initialization failed: " << IMG_GetError() << std::endl;
            return false;
        }
        
        // Initialize SDL_mixer
        if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
            std::cerr << "SDL_mixer initialization failed: " << Mix_GetError() << std::endl;
            musicEnabled = false;
        } else {
            std::cout << "Audio system initialized!" << std::endl;
            musicEnabled = true;
        }
        
        window = SDL_CreateWindow("Maze Shooter - Developed by Ahmed Dajani (c) 2025",
            SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
            SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
        
        if (!window) {
            std::cerr << "Window creation failed: " << SDL_GetError() << std::endl;
            return false;
        }
        
        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
        if (!renderer) {
            std::cerr << "Renderer creation failed: " << SDL_GetError() << std::endl;
            return false;
        }
        
        // Create screen texture for fast pixel buffer rendering
        screenTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, 
                                        SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);
        if (!screenTexture) {
            std::cerr << "Screen texture creation failed: " << SDL_GetError() << std::endl;
            return false;
        }
        
        // Allocate screen buffer
        screenBuffer = new Uint32[SCREEN_WIDTH * SCREEN_HEIGHT];
        
        // Load all assets
        loadFonts();
        loadMusic();
        loadGunAssets();
        loadMenuBackground();
        
        // Start with menu music
        playMusic(menuMusic);
        
        return true;
    }
    
    void handleMenuEvents(SDL_Event& e) {
        if (e.type == SDL_KEYDOWN) {
            switch (e.key.keysym.scancode) {
                case SDL_SCANCODE_UP:
                    selectedMenuItem = (selectedMenuItem - 1 + MENU_ITEM_COUNT) % MENU_ITEM_COUNT;
                    break;
                case SDL_SCANCODE_DOWN:
                    selectedMenuItem = (selectedMenuItem + 1) % MENU_ITEM_COUNT;
                    break;
                case SDL_SCANCODE_SPACE:
                case SDL_SCANCODE_RETURN:
                    if (selectedMenuItem == MENU_NEW_GAME) {
                        startNewGame();
                    } else if (selectedMenuItem == MENU_EXIT) {
                        running = false;
                    }
                    break;
                case SDL_SCANCODE_ESCAPE:
                    running = false;
                    break;
            }
        }
    }
    
    void shootGun() {
        if (isShooting) return;
        
        isShooting = true;
        currentGunFrame = 1;
        animationTimer = 0;
        lastAnimationTime = SDL_GetTicks();
        
        if (shootSound && musicEnabled) {
            Mix_PlayChannel(-1, shootSound, 0);
        }
        
        std::cout << "BANG!" << std::endl;
    }
    
    void updateGunAnimation() {
        if (!isShooting) {
            currentGunFrame = 0;
            return;
        }
        
        Uint32 currentTime = SDL_GetTicks();
        
        if (currentTime - lastAnimationTime >= ANIMATION_SPEED) {
            currentGunFrame++;
            lastAnimationTime = currentTime;
            
            if (currentGunFrame >= GUN_FRAMES) {
                currentGunFrame = 0;
                isShooting = false;
            }
        }
    }
    
    void handleGameEvents(SDL_Event& e) {
        if (e.type == SDL_KEYDOWN) {
            keys[e.key.keysym.scancode] = true;
            
            // Return to menu
            if (e.key.keysym.scancode == SDL_SCANCODE_ESCAPE) {
                returnToMenu();
                return;
            }
            
            // Handle jump
            if (e.key.keysym.scancode == SDL_SCANCODE_SPACE && !isJumping) {
                verticalVelocity = JUMP_SPEED;
                isJumping = true;
            }
            
            // Handle gun shooting
            if ((e.key.keysym.scancode == SDL_SCANCODE_LSHIFT || 
                 e.key.keysym.scancode == SDL_SCANCODE_RSHIFT) && !isShooting) {
                shootGun();
            }
            
            if (e.key.keysym.scancode == SDL_SCANCODE_X && !isShooting) {
                shootGun();
            }
        }
        else if (e.type == SDL_KEYUP) {
            keys[e.key.keysym.scancode] = false;
        }
    }
    
    void handleEvents() {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                running = false;
            }
            
            if (currentState == STATE_MENU) {
                handleMenuEvents(e);
            } else if (currentState == STATE_PLAYING) {
                handleGameEvents(e);
            }
        }
        
        // Handle continuous input (only in game)
        if (currentState == STATE_PLAYING) {
            // Movement
            if (keys[SDL_SCANCODE_W]) {
                if (worldMap[int(posX + dirX * MOVE_SPEED)][int(posY)] == 0) posX += dirX * MOVE_SPEED;
                if (worldMap[int(posX)][int(posY + dirY * MOVE_SPEED)] == 0) posY += dirY * MOVE_SPEED;
            }
            if (keys[SDL_SCANCODE_S]) {
                if (worldMap[int(posX - dirX * MOVE_SPEED)][int(posY)] == 0) posX -= dirX * MOVE_SPEED;
                if (worldMap[int(posX)][int(posY - dirY * MOVE_SPEED)] == 0) posY -= dirY * MOVE_SPEED;
            }
            
            // Strafing
            if (keys[SDL_SCANCODE_A]) {
                if (worldMap[int(posX - planeX * MOVE_SPEED)][int(posY)] == 0) posX -= planeX * MOVE_SPEED;
                if (worldMap[int(posX)][int(posY - planeY * MOVE_SPEED)] == 0) posY -= planeY * MOVE_SPEED;
            }
            if (keys[SDL_SCANCODE_D]) {
                if (worldMap[int(posX + planeX * MOVE_SPEED)][int(posY)] == 0) posX += planeX * MOVE_SPEED;
                if (worldMap[int(posX)][int(posY + planeY * MOVE_SPEED)] == 0) posY += planeY * MOVE_SPEED;
            }
            
            // Rotation
            if (keys[SDL_SCANCODE_LEFT]) {
                double oldDirX = dirX;
                dirX = dirX * cos(ROT_SPEED) - dirY * sin(ROT_SPEED);
                dirY = oldDirX * sin(ROT_SPEED) + dirY * cos(ROT_SPEED);
                double oldPlaneX = planeX;
                planeX = planeX * cos(ROT_SPEED) - planeY * sin(ROT_SPEED);
                planeY = oldPlaneX * sin(ROT_SPEED) + planeY * cos(ROT_SPEED);
            }
            if (keys[SDL_SCANCODE_RIGHT]) {
                double oldDirX = dirX;
                dirX = dirX * cos(-ROT_SPEED) - dirY * sin(-ROT_SPEED);
                dirY = oldDirX * sin(-ROT_SPEED) + dirY * cos(-ROT_SPEED);
                double oldPlaneX = planeX;
                planeX = planeX * cos(-ROT_SPEED) - planeY * sin(-ROT_SPEED);
                planeY = oldPlaneX * sin(-ROT_SPEED) + planeY * cos(-ROT_SPEED);
            }
            
            // Update jumping physics
            if (isJumping) {
                cameraHeight += verticalVelocity;
                verticalVelocity -= GRAVITY;
                
                if (cameraHeight <= GROUND_HEIGHT) {
                    cameraHeight = GROUND_HEIGHT;
                    verticalVelocity = 0.0;
                    isJumping = false;
                }
            }
            
            // Update gun animation
            updateGunAnimation();
        }
    }
    
    void renderText(TTF_Font* font, const std::string& text, int x, int y, SDL_Color color, bool centered = false) {
        if (!font) return;
        
        SDL_Surface* textSurface = TTF_RenderText_Solid(font, text.c_str(), color);
        if (textSurface) {
            SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
            if (textTexture) {
                SDL_Rect textRect;
                textRect.w = textSurface->w;
                textRect.h = textSurface->h;
                
                if (centered) {
                    textRect.x = x - textRect.w / 2;
                    textRect.y = y - textRect.h / 2;
                } else {
                    textRect.x = x;
                    textRect.y = y;
                }
                
                SDL_RenderCopy(renderer, textTexture, nullptr, &textRect);
                SDL_DestroyTexture(textTexture);
            }
            SDL_FreeSurface(textSurface);
        }
    }
    
    void renderMenu() {
        // Clear screen first
        SDL_SetRenderDrawColor(renderer, 20, 30, 50, 255);
        SDL_RenderClear(renderer);
        
        // Render background image if available
        if (menuBackground) {
            // Scale background to fit screen
            SDL_Rect backgroundRect = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
            SDL_RenderCopy(renderer, menuBackground, NULL, &backgroundRect);
        }
        
        // Colors (made more vibrant to stand out over background)
        SDL_Color titleColor = {255, 0, 0, 255};  // Bright Yellow
        SDL_Color normalColor = {255, 255, 255, 255}; // White for better visibility
        SDL_Color selectedColor = {255, 50, 50, 255};  // Bright Red
        SDL_Color copyrightColor = {0, 0, 255, 255}; // Light gray
        
        // Add text shadows/outlines for better readability over background
        SDL_Color shadowColor = {0, 0, 0, 255}; // Black shadow
        
        // Render title with shadow
        renderText(titleFont, "Maze Shooter", SCREEN_WIDTH / 2 + 2, 152, shadowColor, true); // Shadow
        renderText(titleFont, "Maze Shooter", SCREEN_WIDTH / 2, 150, titleColor, true);     // Main text
        
        // Render copyright with shadow
        renderText(copyrightFont, "Developed by Ahmed Dajani (c) 2025", SCREEN_WIDTH / 2 + 1, 201, shadowColor, true); // Shadow
        renderText(copyrightFont, "Developed by Ahmed Dajani (c) 2025", SCREEN_WIDTH / 2, 200, copyrightColor, true);   // Main text
        
        // Render menu items with color-based selection and shadows
        std::vector<std::string> menuItems = {"New Game", "Exit"};
        
        for (int i = 0; i < menuItems.size(); i++) {
            SDL_Color color = (i == selectedMenuItem) ? selectedColor : normalColor;
            int y = 300 + i * 60;
            
            // Render shadow first, then main text
            renderText(menuFont, menuItems[i], SCREEN_WIDTH / 2 + 2, y + 2, shadowColor, true); // Shadow
            renderText(menuFont, menuItems[i], SCREEN_WIDTH / 2, y, color, true);               // Main text
        }
        
        // Instructions with shadow
        renderText(copyrightFont, "Use Arrow Keys to navigate, Space to select", SCREEN_WIDTH / 2 + 1, 501, shadowColor, true); // Shadow
        renderText(copyrightFont, "Use Arrow Keys to navigate, Space to select", SCREEN_WIDTH / 2, 500, normalColor, true);     // Main text
        
        SDL_RenderPresent(renderer);
    }
    
    void updateFPS() {
        frameCount++;
        Uint32 currentTime = SDL_GetTicks();
        
        if (currentTime - lastTime >= 1000) {
            fps = frameCount * 1000.0 / (currentTime - lastTime);
            frameCount = 0;
            lastTime = currentTime;
        }
    }
    
    void drawFPS() {
        std::stringstream ss;
        ss << "FPS: " << (int)fps;
        SDL_Color fpsColor = {255, 255, 255, 255};
        renderText(copyrightFont, ss.str(), 10, 10, fpsColor);
    }
    
    void drawGun() {
        if (!gunSprites[currentGunFrame]) {
            return;
        }
        
        int gunWidth, gunHeight;
        SDL_QueryTexture(gunSprites[currentGunFrame], NULL, NULL, &gunWidth, &gunHeight);
        
        int scaledWidth = gunWidth * 2;
        int scaledHeight = gunHeight * 2;
        
        int gunX = (SCREEN_WIDTH - scaledWidth) / 2;
        int gunY = SCREEN_HEIGHT - scaledHeight;
        
        SDL_Rect gunRect = {gunX, gunY, scaledWidth, scaledHeight};
        SDL_RenderCopy(renderer, gunSprites[currentGunFrame], NULL, &gunRect);
    }
    
    void renderGame() {
        updateFPS();
        
        int horizon = SCREEN_HEIGHT / 2 + (int)(cameraHeight * 100);
        
        for (int y = 0; y < SCREEN_HEIGHT; y++) {
            for (int x = 0; x < SCREEN_WIDTH; x++) {
                if (y < horizon) {
                    screenBuffer[y * SCREEN_WIDTH + x] = 0xFF87CEEB;
                } else {
                    screenBuffer[y * SCREEN_WIDTH + x] = 0xFF555555;
                }
            }
        }
        
        // Raycasting for walls
        for (int x = 0; x < SCREEN_WIDTH; x++) {
            double cameraX = 2 * x / double(SCREEN_WIDTH) - 1;
            double rayDirX = dirX + planeX * cameraX;
            double rayDirY = dirY + planeY * cameraX;
            
            int mapX = int(posX);
            int mapY = int(posY);
            
            double deltaDistX = std::abs(1 / rayDirX);
            double deltaDistY = std::abs(1 / rayDirY);
            double perpWallDist;
            
            int stepX, stepY;
            double sideDistX, sideDistY;
            
            if (rayDirX < 0) {
                stepX = -1;
                sideDistX = (posX - mapX) * deltaDistX;
            } else {
                stepX = 1;
                sideDistX = (mapX + 1.0 - posX) * deltaDistX;
            }
            
            if (rayDirY < 0) {
                stepY = -1;
                sideDistY = (posY - mapY) * deltaDistY;
            } else {
                stepY = 1;
                sideDistY = (mapY + 1.0 - posY) * deltaDistY;
            }
            
            int hit = 0;
            int side;
            
            while (hit == 0) {
                if (sideDistX < sideDistY) {
                    sideDistX += deltaDistX;
                    mapX += stepX;
                    side = 0;
                } else {
                    sideDistY += deltaDistY;
                    mapY += stepY;
                    side = 1;
                }
                
                if (worldMap[mapX][mapY] > 0) hit = 1;
            }
            
            if (side == 0) {
                perpWallDist = (mapX - posX + (1 - stepX) / 2) / rayDirX;
            } else {
                perpWallDist = (mapY - posY + (1 - stepY) / 2) / rayDirY;
            }
            
            int lineHeight = (int)(SCREEN_HEIGHT / perpWallDist);
            
            int drawStart = -lineHeight / 2 + horizon;
            if (drawStart < 0) drawStart = 0;
            
            int drawEnd = lineHeight / 2 + horizon;
            if (drawEnd >= SCREEN_HEIGHT) drawEnd = SCREEN_HEIGHT - 1;
            
            int texNum = worldMap[mapX][mapY];
            
            double wallX;
            if (side == 0) {
                wallX = posY + perpWallDist * rayDirY;
            } else {
                wallX = posX + perpWallDist * rayDirX;
            }
            wallX -= floor(wallX);
            
            int texX = int(wallX * double(TEXTURE_WIDTH));
            if (side == 0 && rayDirX > 0) texX = TEXTURE_WIDTH - texX - 1;
            if (side == 1 && rayDirY < 0) texX = TEXTURE_WIDTH - texX - 1;
            
            double step = 1.0 * TEXTURE_HEIGHT / lineHeight;
            double texPos = (drawStart - horizon + lineHeight / 2) * step;
            
            for (int y = drawStart; y < drawEnd; y++) {
                int texY = (int)texPos & (TEXTURE_HEIGHT - 1);
                texPos += step;
                
                Uint32 color = getTexturePixel(texNum, texX, texY);
                
                if (side == 1) {
                    color = ((color >> 1) & 0x7F7F7F7F) | 0xFF000000;
                }
                
                screenBuffer[y * SCREEN_WIDTH + x] = color;
            }
        }
        
        SDL_UpdateTexture(screenTexture, NULL, screenBuffer, SCREEN_WIDTH * sizeof(Uint32));
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, screenTexture, NULL, NULL);
        
        drawFPS();
        drawGun();
        
        // Show game instructions
        SDL_Color instructColor = {255, 255, 255, 255};
        renderText(copyrightFont, "ESC - Return to Menu | WASD - Move | Arrows - Turn | SPACE - Jump | SHIFT - Shoot", 10, SCREEN_HEIGHT - 30, instructColor);
        
        SDL_RenderPresent(renderer);
    }
    
    void render() {
        if (currentState == STATE_MENU) {
            renderMenu();
        } else if (currentState == STATE_PLAYING) {
            renderGame();
        }
    }
    
    void run() {
        std::cout << "Maze Shooter Started!" << std::endl;
        std::cout << "Currently in main menu" << std::endl;
        
        while (running) {
            handleEvents();
            render();
            SDL_Delay(16);
        }
    }
    
    void cleanup() {
        if (menuMusic) {
            Mix_FreeMusic(menuMusic);
            menuMusic = nullptr;
        }
        
        if (gameMusic) {
            Mix_FreeMusic(gameMusic);
            gameMusic = nullptr;
        }
        
        if (shootSound) {
            Mix_FreeChunk(shootSound);
            shootSound = nullptr;
        }
        
        for (int i = 0; i < GUN_FRAMES; i++) {
            if (gunSprites[i]) {
                SDL_DestroyTexture(gunSprites[i]);
                gunSprites[i] = nullptr;
            }
        }
        
        if (titleFont) TTF_CloseFont(titleFont);
        if (menuFont) TTF_CloseFont(menuFont);
        if (copyrightFont) TTF_CloseFont(copyrightFont);
        
        if (menuBackground) {
            SDL_DestroyTexture(menuBackground);
            menuBackground = nullptr;
        }
        
        if (screenBuffer) {
            delete[] screenBuffer;
        }
        if (screenTexture) {
            SDL_DestroyTexture(screenTexture);
        }
        if (renderer) {
            SDL_DestroyRenderer(renderer);
        }
        if (window) {
            SDL_DestroyWindow(window);
        }
        
        if (musicEnabled) {
            Mix_CloseAudio();
        }
        TTF_Quit();
        IMG_Quit();
        SDL_Quit();
        
        std::cout << "Maze Shooter cleaned up. Thanks for playing!" << std::endl;
    }
    
    ~MazeShooter() {
        cleanup();
    }
};

int main(int argc, char* argv[]) {
    MazeShooter game;
    
    if (!game.init()) {
        std::cerr << "Failed to initialize Maze Shooter!" << std::endl;
        return -1;
    }
    
    std::cout << "================================" << std::endl;
    std::cout << "MAZE SHOOTER" << std::endl;
    std::cout << "Developed by Ahmed Dajani (c) 2025" << std::endl;
    std::cout << "================================" << std::endl;
    std::cout << std::endl;
    std::cout << "Controls:" << std::endl;
    std::cout << "WASD - Move, Arrows - Turn, Space - Jump, Shift - Shoot" << std::endl;
    std::cout << "Press Space to start a new game or Exit to quit." << std::endl;
    std::cout << "Press ESC to return to the main menu." << std::endl;
    std::cout << "================================" << std::endl;
    
    game.run();
    
    return 0;
}