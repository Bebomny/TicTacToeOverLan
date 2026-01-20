# TicTacToeOverLan
1. [Overview](#overview)
2. [Idea](#idea)
3. Implementation plans
4. Implementation analysis
5. User manual
6. Contribution guide

## Overview
TicTacToeOverLan allows for up to 6 simultaneous players connected over the network.
With port forwarding It should be possible to play over the internet.
Custom board settings allow for a board of size 32x32 with custom win condition length.
![Screenshot of the game running](.\resources\tictactieoverlan-img1.png)

## Idea
The general idea is inspired by how minecraft handles it's worlds.
We should have an option to launch an Internal Server by one of the Clients.
Others would connect to it via a specified IP and port, or a url.
The state of the game should be held and validated on the server.
Board updates are going to be sent packaged with the whole board, 
as it's pretty small and shouldn't pose any performance concerns.
The host should be the one managing the board settings, and requesting game starts and restarts.

### Networking
The connections are going to be handled by Windows WebSockets. 
The Windows api already contains methods for handling multiple connections at a time, making it quick to set up.
Sadly that means, this implementation won't work on linux systems.
A custom network protocol needs to be designed, so the server and client know what data is being transmitted and how to handle it.

### State Management
