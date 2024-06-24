#include <SDL.h>
#include <sdl_ttf.h>
#include <string>
#include <cstdlib>
#include <ctime>

int randomNum(int min, int max) {

	// seed the random number generator with the current time
	std::srand(static_cast<unsigned int>(std::time(0)));

	// generate a random number between min and max
	int randomNum = min + (std::rand() % (max - min + 1));

	return randomNum;
}

int main(int argc, char* argv[]) {
	const int screenWidth = 1280;
	const int screenHeight = 720;

	// Initialise the SDL Video Subsystem. (Returns 0 on success)

	// Check for video initialising error
	if (SDL_Init(SDL_INIT_VIDEO)) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Could not initialise SDL: %s", SDL_GetError());
		
		return 3; //error code 3
	}

	SDL_Window* window = SDL_CreateWindow("Pong", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, screenWidth, screenHeight, 0);

	if (window == nullptr) { // If the Window could not be created, log the error and exit with error code 3
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create window: %s", SDL_GetError());
		return 3;
	}

	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

	
	if (renderer == nullptr) { // If the renderer could not be created, log the error and exit.
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create renderer: %s", SDL_GetError());
		return 3;
	}
	
	// Initialise SDL_ttf

	// Check for TTF initialising error
	if (TTF_Init() == -1) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Could not intialise SDL_ttf : % s", TTF_GetError());
		return 3;
	}
	
	// Load font
	TTF_Font* pongFont = TTF_OpenFont("SquareFontNums-Regular.ttf", 20);

	if (pongFont == nullptr) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to create font: %s", TTF_GetError());
		return 1;
	}

	// Render text surface for P1 and P2
	SDL_Color white = { 0xFF, 0xFF, 0xFF, 0xFF };

	SDL_Surface* textSurfaceP1Score = TTF_RenderText_Solid(pongFont, "0", white);
	if (textSurfaceP1Score == nullptr) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Unable to render textSurfaceP1Score: %s", TTF_GetError());
		return 1;
	}
	SDL_Surface* textSurfaceP2Score = TTF_RenderText_Solid(pongFont, "0", white);
	if (textSurfaceP2Score == nullptr) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Unable to render textSurfaceP2Score: %s", TTF_GetError());
		return 1;
	}

	// Create texture from surface for P1 and P2
	SDL_Texture* textTextureP1Score = SDL_CreateTextureFromSurface(renderer, textSurfaceP1Score);
	if (textTextureP1Score == nullptr) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Unable to create textTextureP1Score from rendered text: %s", SDL_GetError());
	}
	SDL_Texture* textTextureP2Score = SDL_CreateTextureFromSurface(renderer, textSurfaceP2Score);
	if (textTextureP2Score == nullptr) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Unable to create textTextureP2Score from rendered text: %s", SDL_GetError());
	}

	// Get the dimensions of the text for P1 and P2
	int textWidthP1Score = textSurfaceP1Score->w;
	int textHeightP1Score = textSurfaceP1Score->h;

	int textWidthP2Score = textSurfaceP2Score->w;
	int textHeightP2Score = textSurfaceP2Score->h;

	// Free the text surface as we no longer need it
	SDL_FreeSurface(textSurfaceP1Score);
	SDL_FreeSurface(textSurfaceP2Score);

	bool running{ true };

	// Key press states
	bool wPressed{ false };
	bool sPressed{ false };
	bool upPressed{ false };
	bool downPressed{ false };

	// Game States
	bool playerTurn{ true }; //true for player 1, false for player 2
	bool gameOngoing{ true }; //check if game is currently ongoing

	// Player Score
	int player1Score = 0;
	int player2Score = 0;

	// Objects
	SDL_Rect paddle1{ 
		screenWidth, // x
		screenHeight, // y
		10, 120 }; // w, h

	SDL_Rect paddle2{
		screenWidth, // x
		screenHeight, // y
		10, 120 }; // w, h

	SDL_Rect middleLine{
		(screenWidth - 5) / 2 , // x
		0, // y
		5, screenHeight }; // w, h

	SDL_Rect ball{
		screenWidth / 2, // x
		screenHeight / 2, // y
		20, 20 }; // w, h

	// for drawing the text
	SDL_Rect renderQuadTextP1{
		((screenWidth - textWidthP1Score) / 2) - 20, // x (centre then offset)
		textHeightP1Score, // y (centre then offset)
		textWidthP1Score, textHeightP1Score // w, h
	};
	SDL_Rect renderQuadTextP2{
		((screenWidth - textWidthP2Score) / 2) + 20, // x (centre then offset)
		textHeightP2Score, 
		textWidthP2Score, textHeightP2Score // w, h
	};

	// Player coordinates
	int p1YCoord = (screenHeight / 2) - (paddle1.h / 2);
	int p2YCoord = (screenHeight / 2) - (paddle2.h / 2);

	// Ball coordinates
	int ballXStart = (screenWidth / 2) - (ball.w / 2);
	int ballYStart = (screenHeight / 2) - (ball.h / 2);


	// Ball speed
	int ballXVelocity = 3;
	int ballYVelocity = 1;
	int differenceInY = 0;

	// Game Loop
	while (running) {

		// Player 1 position
		paddle1.x = (screenWidth / 2) / (paddle1.w / 2);
		paddle1.y = p1YCoord;

		// Player 2 position
		paddle2.x = screenWidth - (screenWidth / 2) / (paddle2.w / 2);
		paddle2.y = p2YCoord;

		// Middle line position
		middleLine.x = (screenWidth / 2) - (middleLine.w / 2);

		// Polling for SDL Events
		SDL_Event event;

		// Loop until there are no more pending events to process.
		while (SDL_PollEvent(&event) != 0) {
			if (event.type == SDL_QUIT) {
				running = false; // Stop running if the window is closed
			}

			else if (event.type == SDL_KEYDOWN) {
				
				// Select actions based on key press
				switch (event.key.keysym.sym) {
				case SDLK_w: // w key
					wPressed = true;
					break;
				case SDLK_s:
					sPressed = true;
					break;
				case SDLK_UP: // up arrow
					upPressed = true;
					break;
				case SDLK_DOWN: // down arrow
					downPressed = true;
					break;
				}
			}
			else if (event.type == SDL_KEYUP || event.type == SDL_MOUSEBUTTONUP) {

				// Select actions based on key press
				switch (event.key.keysym.sym) {
				case SDLK_w: // w key
					wPressed = false;
					break;
				case SDLK_s:
					sPressed = false;
					break;
				case SDLK_UP: // up arrow
					upPressed = false;
					break;
				case SDLK_DOWN: // down arrow
					downPressed = false;
					break;
				}
			}
		}
		// Drawing to the renderer

		// Set render colour to black
		SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0x00);

		// Clear the renderer with the current render color. (the background)
		SDL_RenderClear(renderer);

		// Set the render colour white.
		SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0x00);

		// Draw the player1paddle
		SDL_RenderFillRect(renderer, &paddle1);

		// Draw the player2paddle
		SDL_RenderFillRect(renderer, &paddle2);

		// Draw the ball
		SDL_RenderFillRect(renderer, &ball);

		// Draw the middle line
		SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0x00);
		SDL_RenderFillRect(renderer, &middleLine);

		// Draw the score
		SDL_RenderCopy(renderer, textTextureP1Score, NULL, &renderQuadTextP1);
		SDL_RenderCopy(renderer, textTextureP2Score, NULL, &renderQuadTextP2);
		
		// Render everything to the screen.
		SDL_RenderPresent(renderer);

		// Paddle movement
		if (wPressed) {
			if (p1YCoord < 0) {
			}
			else {
				p1YCoord += -8;
			}
		}
		if (sPressed) {

			if (p1YCoord > 720 - paddle1.h) {
			}
			else {
				p1YCoord += 8;
			}
		}
		if (upPressed) {
			if (p2YCoord < 0) {
			}
			else {
				p2YCoord += -8;
			}
		}
		if (downPressed) {
			if (p2YCoord > 720 - paddle2.h) {
			}
			else {
				p2YCoord += 8;
			}
		}

		// Ball movement
		if (gameOngoing) {

			if (playerTurn) { // heading towards player 1
				ball.x += - ballXVelocity;
				ball.y += ballYVelocity;
				
				if (((ball.x >= paddle1.x && ball.x <= (paddle1.x + paddle1.w)) && (ball.y >= paddle1.y && ball.y <= (paddle1.y + paddle1.h)))) { // checks if the ball hits the paddles x and y overlap
					playerTurn = false;
					
					differenceInY = ball.y - (paddle1.y + (paddle1.h / 2));
					ballYVelocity = ballYVelocity + (differenceInY / (paddle1.h / 2));

					ballXVelocity = 6;
				}
				if (ball.x <= 0) { // check if the ball hits left wall then update score
					playerTurn = true;
					player2Score++;
					gameOngoing = false;
				}
				if (ball.y >= screenHeight - ball.h) { // check if the ball hits the bottom wall
					ballYVelocity = -ballYVelocity;
				}
				if (ball.y <= 0) { // check if the ball hits the top wall
					ballYVelocity = -ballYVelocity;
				}
			}
			else { // heading towards player 2
				ball.x += ballXVelocity;
				ball.y += ballYVelocity;

				if (((ball.x >= paddle2.x - paddle2.w && ball.x <= (paddle2.x + paddle2.w)) && (ball.y >= paddle2.y && ball.y <= (paddle2.y + paddle2.h)))) { // checks if the ball hits the paddles x and y overlap
					playerTurn = true;

					differenceInY = ball.y - (paddle2.y + (paddle2.h / 2));
					ballYVelocity = ballYVelocity + (differenceInY / (paddle2.h / 2));

					ballXVelocity = 6;
				}
				if (ball.x >= screenWidth) { // check if the ball hits the right wall then update score
					playerTurn = false;
					player1Score++;
					gameOngoing = false;
				}
				if (ball.y >= screenHeight - ball.h) { // check if the ball hits the bottom wall
					ballYVelocity = -ballYVelocity;
				}
				if (ball.y <= 0) { // check if the ball hits the top wall
					ballYVelocity = -ballYVelocity;
				}
			}
		}


		else { 
			// Reset the ball position when the game restarts
			ball.x = ballXStart;
			ball.y = ballYStart;

			// Reset the ball X velocity and randomise Y velocity
			ballXVelocity = 3;
			ballYVelocity = randomNum(1, 4);

			// Convert player score to string
			std::string strPlayer1Score = std::to_string(player1Score);
			std::string strPlayer2Score = std::to_string(player2Score);

			// Render score to screen
			textSurfaceP1Score = TTF_RenderText_Solid(pongFont, strPlayer1Score.c_str(), white);
			textTextureP1Score = SDL_CreateTextureFromSurface(renderer, textSurfaceP1Score);

			textSurfaceP2Score = TTF_RenderText_Solid(pongFont, strPlayer2Score.c_str(), white);
			textTextureP2Score = SDL_CreateTextureFromSurface(renderer, textSurfaceP2Score);

			// Start the game again
			gameOngoing = true;
		}
	}

	// Cleanup SDL components and quit
	SDL_DestroyTexture(textTextureP1Score);
	SDL_DestroyTexture(textTextureP2Score);
	TTF_CloseFont(pongFont);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}

