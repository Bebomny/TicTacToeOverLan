#include "client/GameClient.h"

/**
 * @brief Application Entry Point.
 * <br> Initializes the primary GameClient instance and enters the main execution loop.
 *
 * @return 0 upon successful termination.
 */
int main() {
    GameClient client;
    client.run();

    return 0;
}
