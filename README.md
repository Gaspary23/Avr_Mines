# AVR Mines

## Introduction

AVR Mines is a Minesweeper implementation for AVR. Minesweeper

Minesweeper is a game in which the player is given a board with a certain number of spaces that contain hidden mines. To win, the player must select all empty spaces, avoiding the mines.

## Gameplay

The board is a 5 x 14 matrix which starts with all spaces hidden, as seen in **Figure 1**. Once a space is selected, it and its neighbors are revealed. A space can be empty or contain a mine. Empty spaces display the total value of adjacent mines, or nothing if they have no neighbouring mines.

<p align="center">
  <img src="https://lh6.googleusercontent.com/KbEe98pgpzxm7q4e9VcQOEofyWBAaHUlcj2RhR4-m04PyTyIWHOA9puv0zDMjeKwInRIX1IU-9gOdVK81d-xNBTXTny6y28bnryemjrImoKlRvcNOH4A_1uMyCLAtAFF3oH5MPz37HAhtLXGdg" />
</p>
<h4 align="center">
  Figure 1. Board with all spaces hidden
</h4>

A space reveals no neighbours if it has any adjacent mines. There are 14 mines on the board, and if any one of them are selected, the game is lost. The player can mark houses where they believe there are mines with flags to make the game easier, as seen in **Figure 2**. When all empty houses are selected, the player wins the game. Then, the player can restart the game with a new, randomly generated board.

<p align="center">
  <img src="https://lh5.googleusercontent.com/YYUNc7a3Zyjsp2PkiYwr9oKhANGXT3BjsAiiDPv0pUN3DOSiZzZJ6VNPtvtt2hBacH--T7cb5FGjXnm3s1agOqbaCZqIhgSWBmLeQoq_-xLLOs_DSN3hV7vZbPOwz7XXkyPe1HgCuDzYVRuYfg" />
</p>
<h4 align="center">
  Figure 2. Partially explored board
</h4>

## End of game

 - ### Success: every empty space was revealed;
 - ### Defeat: a mine was selected.

Every space is revealed and the appropriate message is shown, as seen in **Figures 2** and **3**. The player then may press the FLAG button to restart the game.

<p align="center">
  <img src="https://lh3.googleusercontent.com/hT1hObA7wl0n-DFspSWY9oqZcqxzFrFy-wbX45kgU8gtWPmFKnkYsOJvoYdukaPccwL5GdG_CSVp0S0cuDNf3TVIpi8lfbQEpzq89xvTZ4bcyC7DGBqev6Dap7am6csWeNbA_9UlmvSqTgGJ1A" />
</p>
<h4 align="center">
  Figure 3. Victory screen
</h4>

<p align="center">
  <img src="https://lh5.googleusercontent.com/RBUrfteJYom-YUTxq5mAH-nZEKsihsBRaMTrYpNQdtp2Hex8W2Zn6rRvUf-pYX44Q12I2JNTCZTVKLJSjKECg26K6JnEZkSPkYoJJCGJOV1lDq8-gAardsIcmct-zx-SXWq2xdHI2_gL3rYbeQ" />
</p>
<h4 align="center">
  Figura 4. Defeat screen
</h4>

## Resources used

To implement this game, the AVR atmega328 processor  and the pcd544 display were used. The project was simulated with simulIDE 0.4.15_SR9-1. Six buttons were used, four for movement and two others. Of the latter, the CHECK button is responsible for starting the game and for selecting a field, while FLAG is used for restarting the game and marking a square on the board with a flag.

## Building and running

The game may be built and ran by executing `$ make` in the project's root directory and then loading the generated .hex file within simulIDE after using it to open  simulide/mines.simu (right click the CPU and select "Load firmware").
