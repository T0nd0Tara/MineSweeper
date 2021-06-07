#define OLC_PGE_APPLICATION

#include <iostream>
#include <time.h>
#include "olcPixelGameEngine.h"

// adding the matrix class
#include <taraNS.h> 

class MineSweeper : public olc::PixelGameEngine
{
public:
	MineSweeper() {
		sAppName = "MineSweeper";
	}

private:

	olc::vi2d screenSize, boardSize, cellSize;

	// -1 if theres a bomb, 0 if no bombs in area, n if theres n bombs near
	tara::Matrix<int>* map = nullptr;

	// 0 if not revealed, 1 if revealed, 2 if flag
	tara::Matrix<uint8_t>* revealMap;

	// the end game count down (to show where the bombs are)
	float countDown;

	bool firstClick;

	olc::vi2d Board2Screen(olc::vi2d in) {
		return in * cellSize;
	}

	olc::vi2d Screen2Board(olc::vi2d in) {
		return  in / cellSize;
	}

	// when you want to reveal a cell
	void PopCell(olc::vi2d cords) {
		// checking the cords
		if (boardSize.x <= cords.x || cords.x < 0) return;
		if (boardSize.y <= cords.y || cords.y < 0) return;

		// returnning if to cell is already revealed
		if (revealMap->get_cell(cords.x, cords.y)) return;

		// revealing the cell
		revealMap->set_cell(cords.x, cords.y, true);

		if (map->get_cell(cords.x, cords.y) != 0) return;

		// if the cell is 0, we want to continue and revealing the neibors cells
		olc::vi2d TestCords = {0,0};
		for (TestCords.y = -1; TestCords.y <= 1; TestCords.y++)
			for (TestCords.x = -1; TestCords.x <= 1; TestCords.x++) {
				olc::vi2d checkCords = cords + TestCords;


				// go away if you are out of the board
				if (boardSize.x <= checkCords.x || checkCords.x < 0) continue;
				if (boardSize.y <= checkCords.y || checkCords.y < 0) continue;

				PopCell(checkCords);

			}
	}

	void restartMap() {
		srand(time(NULL));

		map = new tara::Matrix<int>(boardSize.x, boardSize.y, 0);

		// putting bombs
		for (uint32_t i = 0; i < boardSize.x * boardSize.y; i++) {
			// 15% chance of there beiing a bomb
			if (rand() % 100 < 15) {
				map->set_cell(i, -1);
			}
		}

		// setting the values
		for (uint32_t y = 0; y < boardSize.y; y++) {
			for (uint32_t x = 0; x < boardSize.y; x++) {
				if (map->get_cell(x, y) == -1) continue;

				uint16_t bombsCount = 0;
				for (int yTest = -1; yTest <= 1; yTest++)
					for (int xTest = -1; xTest <= 1; xTest++) {
						int checkX = x + xTest;
						int checkY = y + yTest;

						// go away if you are out of the board
						if (boardSize.x <= checkX || checkX < 0) continue;
						if (boardSize.y <= checkY || checkY < 0) continue;

						if (map->get_cell(checkX, checkY) == -1) bombsCount++;
					}

			
				map->set_cell(x,y, bombsCount);
			}
		}
	}

	void resetGame() {
		// restarts the map
		restartMap();

		// restarts the reveal map
		revealMap = new tara::Matrix<uint8_t>(screenSize.x, screenSize.y, false);

		firstClick = true;
	}

	void DrawX(olc::vi2d cell, olc::Pixel color = olc::WHITE) {
		cell = Board2Screen(cell);
		DrawLine(cell, cell + cellSize, color);
		DrawLine({ cell.x, cell.y + cellSize.y }, { cell.x + cellSize.x, cell.y }, color);
	}
protected:
	bool OnUserCreate() override {
		boardSize = {32,32};

		screenSize.x = ScreenWidth();
		screenSize.y = ScreenHeight();

		cellSize = screenSize / boardSize;

		resetGame();

		countDown = 0.0f;



		return true;
	}

	bool OnUserUpdate(float elapsedTime) override {
		if (GetKey(olc::ESCAPE).bHeld) return false;

		// clear Screen
		Clear(olc::BLACK);

		// get mouse cords in screen space
		olc::vi2d mouseCords = GetMousePos();

		// get mouse cords in board space
		olc::vi2d mouseCordsBS = Screen2Board(mouseCords);

		// viewed cell in board space
		olc::vi2d viewedCell = Board2Screen(mouseCordsBS);

		
		// print blocks
		for (int y = 0; y < boardSize.y; y++) {
			DrawLine(Board2Screen({ 0 ,y}),
					 Board2Screen({boardSize.x, y}));
		}
		for (int x = 0; x < boardSize.x; x++) {
			DrawLine(Board2Screen({x, 0 }),
					 Board2Screen({x, boardSize.y }));
		}


		// display the numbers
		for (int y = 0; y < boardSize.y; y++) {
			for (int x = 0; x < boardSize.x; x++) {
				// we are skipping the ones who arent revealed yet
				if (revealMap->get_cell(x,y) == 0) continue;

				// there's a flag
				if (revealMap->get_cell(x, y) == 2) {
					FillRect(Board2Screen(olc::vi2d{x,y}) + olc::vi2d{ 1,1 }, cellSize - olc::vi2d{ 1,1 }, olc::RED);
					continue;
				}

				FillRect(Board2Screen(olc::vi2d{ x,y }) + olc::vi2d{ 1,1 }, cellSize - olc::vi2d{ 1,1 }, olc::DARK_GREY);
				// Dont want to draw the zeros
				if (map->get_cell(x,y) > 0)
					DrawString(Board2Screen(olc::vi2d{ x, y }) + cellSize / 5, std::to_string(map->get_cell(x, y)), olc::WHITE, cellSize.mag() / 12);
			}
		}

		// if there's a count down
		if (countDown > 0.0f) {
			countDown -= 1.2f * elapsedTime;
			countDown = std::max(countDown, 0.0f);

			// making it to flash the red Xs
			bool showX = ((int)countDown % 4) % 2 == 0;

			for (int y = 0; y < boardSize.y; y++) {
				for (int x = 0; x < boardSize.x; x++) {
					if (map->get_cell(x, y) == -1 && showX)
						DrawX({ x,y }, olc::YELLOW);
				}
			}

			if (countDown == 0) { // we have finished the count down part
				resetGame();
			}

			return true;
		}


		// skip if the mouse is off board
		if (mouseCordsBS.x >= boardSize.x) return true;
		if (mouseCordsBS.y >= boardSize.y) return true;

		// Draw blue Block on selected tile
		SetPixelMode(olc::Pixel::ALPHA);
		FillRect(viewedCell + olc::vi2d{ 1,1 }, cellSize - olc::vi2d{ 1,1 }, olc::Pixel(0, 0, 255, 100));
		SetPixelMode(olc::Pixel::NORMAL);

		// The left key is pressed --- want to check the place
		if (GetMouse(0).bPressed) {
			// making sure that the first click dont lands on a bomb
			if (firstClick) {
				firstClick = false;

				while (map->get_cell(mouseCordsBS.x, mouseCordsBS.y) == -1) {
					delete map;
					restartMap();
				}

				PopCell(mouseCordsBS);
				return true;
			}


			if (map->get_cell(mouseCordsBS.x, mouseCordsBS.y) == -1) {
				// you lost the game

				// constant = 1.2f * the time you want it to be
				countDown = 1.2f * 3.5f;

				std::cout << "you lost!\n";
				return true;
			}


			// revealing only if it isnt a flag or already revealeds
			if (revealMap->get_cell(mouseCordsBS.x, mouseCordsBS.y) == 0)
				PopCell(mouseCordsBS);
		}

		// The right key is pressed --- want to put flag
		else if (GetMouse(1).bPressed) {
			// inverting the flag state
			if (revealMap->get_cell(mouseCordsBS.x, mouseCordsBS.y) == 0)
				revealMap->set_cell(mouseCordsBS.x, mouseCordsBS.y, 2);

			else if (revealMap->get_cell(mouseCordsBS.x, mouseCordsBS.y) == 2)
				revealMap->set_cell(mouseCordsBS.x, mouseCordsBS.y, 0);

		}

		return true;
	}
};

int main() {
	MineSweeper game;
	if (game.Construct(350,350, 3, 3))
		game.Start();
	return 0;
}