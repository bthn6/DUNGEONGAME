#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_ITEMS 10
#define MAX_INVENTORY 5
#define MAX_ROOMS 5
#define MAX_NAME_LEN 50
#define MAX_DESC_LEN 256

typedef struct {
    char name[MAX_NAME_LEN];
} Item;

typedef struct {
    char name[MAX_NAME_LEN];
    int health;
    int strength;
} Creature;

typedef struct Room {
    char description[MAX_DESC_LEN];
    struct Room* north;
    struct Room* south;
    struct Room* east;
    struct Room* west;
    Item* items[MAX_ITEMS];
    int item_count;
    Creature* creature;
} Room;

typedef struct {
    int health;
    int strength;
    int inventory_capacity;
    Item* inventory[MAX_INVENTORY];
} Player;

// Functions
void initialize_game(Player* player, Room** rooms, Room** current_room);
void free_game(Room** rooms, Player* player);
void game_loop(Player* player, Room** rooms, Room** current_room);
void handle_command(char* input, Player* player, Room** rooms, Room** current_room);
void move_player(char* direction, Room** current_room);
void look(Room* current_room);
void show_inventory(Player* player);
void pickup_item(char* item_name, Player* player, Room* current_room);
void attack_creature(Player* player, Room* current_room);
void save_game(Player* player, Room* rooms[], Room* current_room, char* filepath);
void load_game(Player* player, Room* rooms[], Room** current_room, char* filepath);

int main() {
    Player player;
    Room* rooms[MAX_ROOMS] = {NULL};
    Room* current_room = NULL;

    initialize_game(&player, rooms, &current_room);
    game_loop(&player, rooms, &current_room);
    free_game(rooms, &player);

    return 0;
}

// Initialize Game State
void initialize_game(Player* player, Room** rooms, Room** current_room) {
    // Initialize player
    player->health = 100;
    player->strength = 10;
    player->inventory_capacity = MAX_INVENTORY;
    for(int i = 0; i < MAX_INVENTORY; i++) {
        player->inventory[i] = NULL;
    }

    for(int i = 0; i < MAX_ROOMS; i++) {
        rooms[i] = (Room*)malloc(sizeof(Room));
        if(!rooms[i]) {
            printf("Memory allocation failed for rooms.\n");
            exit(1);
        }
        memset(rooms[i], 0, sizeof(Room));
    }

    strcpy(rooms[0]->description, "You are in the entrance of the dungeon. Passages lead east and south.");
    strcpy(rooms[1]->description, "You are in a dimly lit hallway. Passages lead west and south.");
    strcpy(rooms[2]->description, "You are in a small chamber with a wooden chest. Passages lead north.");
    strcpy(rooms[3]->description, "You are in a grand hall with high ceilings. Passages lead north and east.");
    strcpy(rooms[4]->description, "You are in the treasure room! Glittering items are scattered around.");

    // Setup room connections
    rooms[0]->east = rooms[1];
    rooms[0]->south = rooms[2];
    
    rooms[1]->west = rooms[0];
    rooms[1]->south = rooms[3];
    
    rooms[2]->north = rooms[0];
    
    rooms[3]->north = rooms[1];
    rooms[3]->east = rooms[4];
    
    rooms[4]->west = rooms[3];

    // Add items to rooms
    Item* sword = (Item*)malloc(sizeof(Item));
    strcpy(sword->name, "Sword");
    rooms[0]->items[rooms[0]->item_count++] = sword;

    Item* shield = (Item*)malloc(sizeof(Item));
    strcpy(shield->name, "Shield");
    rooms[2]->items[rooms[2]->item_count++] = shield;

    Item* potion = (Item*)malloc(sizeof(Item));
    strcpy(potion->name, "Potion");
    rooms[4]->items[rooms[4]->item_count++] = potion;

    // Add creatures to rooms
    Creature* goblin = (Creature*)malloc(sizeof(Creature));
    strcpy(goblin->name, "Goblin");
    goblin->health = 30;
    goblin->strength = 5;
    rooms[1]->creature = goblin;

    Creature* dragon = (Creature*)malloc(sizeof(Creature));
    strcpy(dragon->name, "Dragon");
    dragon->health = 100;
    dragon->strength = 20;
    rooms[4]->creature = dragon;

    // Set starting room
    *current_room = rooms[0];
}

// Free Allocated Memory
void free_game(Room** rooms, Player* player) {
    // Free items in inventory
    for(int i = 0; i < player->inventory_capacity; i++) {
        if(player->inventory[i]) {
            free(player->inventory[i]);
        }
    }

    // Free rooms and their contents
    for(int i = 0; i < MAX_ROOMS; i++) {
        if(rooms[i]) {
            // Free items in room
            for(int j = 0; j < rooms[i]->item_count; j++) {
                if(rooms[i]->items[j]) {
                    free(rooms[i]->items[j]);
                }
            }
            // Free creatures
            if(rooms[i]->creature) {
                free(rooms[i]->creature);
            }
            free(rooms[i]);
        }
    }
}

// Game Loop
void game_loop(Player* player, Room** rooms, Room** current_room) {
    char input[100];
    printf("Welcome to the Dungeon Adventure!\n");
    printf("Type 'help' to see available commands.\n");
    while(player->health > 0) {
        printf("\n> ");
        if(!fgets(input, sizeof(input), stdin)) {
            printf("Error reading input.\n");
            continue;
        }
        // Remove newline character
        input[strcspn(input, "\n")] = 0;
        handle_command(input, player, rooms, current_room);
    }
    if(player->health <= 0) {
        printf("You have been defeated. Game Over.\n");
    }
}

// Handle User Commands
void handle_command(char* input, Player* player, Room** rooms, Room** current_room) {
    char* command = strtok(input, " ");
    if(!command) return;

    if(strcmp(command, "move") == 0) {
        char* direction = strtok(NULL, " ");
        if(direction) {
            move_player(direction, current_room);
        } else {
            printf("Specify a direction to move.\n");
        }
    }
    else if(strcmp(command, "look") == 0) {
        look(*current_room);
    }
    else if(strcmp(command, "inventory") == 0) {
        show_inventory(player);
    }
    else if(strcmp(command, "pickup") == 0) {
        char* item_name = strtok(NULL, " ");
        if(item_name) {
            pickup_item(item_name, player, *current_room);
        } else {
            printf("Specify an item to pick up.\n");
        }
    }
    else if(strcmp(command, "attack") == 0) {
        attack_creature(player, *current_room);
    }
    else if(strcmp(command, "save") == 0) {
        char* filepath = strtok(NULL, " ");
        if(filepath) {
            save_game(player, rooms, *current_room, filepath);
        } else {
            printf("Specify a file path to save the game.\n");
        }
    }
    else if(strcmp(command, "load") == 0) {
        char* filepath = strtok(NULL, " ");
        if(filepath) {
            load_game(player, rooms, current_room, filepath);
        } else {
            printf("Specify a file path to load the game.\n");
        }
    }
    else if(strcmp(command, "help") == 0) {
        printf("Available commands:\n");
        printf("move <direction> - Move to a different room (north, south, east, west).\n");
        printf("look - Display the description of the current room.\n");
        printf("inventory - List items you have collected.\n");
        printf("pickup <item> - Add an item to your inventory.\n");
        printf("attack - Attack a creature in the room.\n");
        printf("save <filepath> - Save the current game state.\n");
        printf("load <filepath> - Load a saved game state.\n");
        printf("list - List saved games.\n");
        printf("help - Show this help message.\n");
        printf("exit - Exit the game.\n");
    }
    else if(strcmp(command, "exit") == 0) {
        printf("Exiting game. Goodbye!\n");
        exit(0);
    }
    else {
        printf("Unknown command. Type 'help' to see available commands.\n");
    }
}

// Move Player
void move_player(char* direction, Room** current_room) {
    Room* new_room = NULL;
    if(strcmp(direction, "north") == 0) {
        new_room = (*current_room)->north;
    }
    else if(strcmp(direction, "south") == 0) {
        new_room = (*current_room)->south;
    }
    else if(strcmp(direction, "east") == 0) {
        new_room = (*current_room)->east;
    }
    else if(strcmp(direction, "west") == 0) {
        new_room = (*current_room)->west;
    }
    else {
        printf("Invalid direction. Use north, south, east, or west.\n");
        return;
    }

    if(new_room) {
        *current_room = new_room;
        printf("You moved %s.\n", direction);
        look(*current_room);
    }
    else {
        printf("You can't move in that direction.\n");
    }
}

// Look Around
void look(Room* current_room) {
    printf("\n%s\n", current_room->description);
    if(current_room->item_count > 0) {
        printf("You see the following items:\n");
        for(int i = 0; i < current_room->item_count; i++) {
            printf("- %s\n", current_room->items[i]->name);
        }
    }
    if(current_room->creature && current_room->creature->health > 0) {
        printf("There is a %s here.\n", current_room->creature->name);
    }
}

// Show Inventory
void show_inventory(Player* player) {
    printf("Your Inventory:\n");
    int empty = 1;
    for(int i = 0; i < player->inventory_capacity; i++) {
        if(player->inventory[i]) {
            printf("- %s\n", player->inventory[i]->name);
            empty = 0;
        }
    }
    if(empty) {
        printf("Your inventory is empty.\n");
    }
}

// Pickup Item
void pickup_item(char* item_name, Player* player, Room* current_room) {
    // Find the item in the room
    int found = -1;
    for(int i = 0; i < current_room->item_count; i++) {
        if(strcmp(current_room->items[i]->name, item_name) == 0) {
            found = i;
            break;
        }
    }
    if(found == -1) {
        printf("Item not found in the room.\n");
        return;
    }

    // Check inventory capacity
    int slot = -1;
    for(int i = 0; i < player->inventory_capacity; i++) {
        if(!player->inventory[i]) {
            slot = i;
            break;
        }
    }
    if(slot == -1) {
        printf("Your inventory is full.\n");
        return;
    }

    // Add item to inventory
    player->inventory[slot] = current_room->items[found];
    // Remove item from room
    for(int i = found; i < current_room->item_count - 1; i++) {
        current_room->items[i] = current_room->items[i+1];
    }
    current_room->items[current_room->item_count - 1] = NULL;
    current_room->item_count--;

    printf("You picked up %s.\n", item_name);
}

// Attack Creature
void attack_creature(Player* player, Room* current_room) {
    if(!current_room->creature || current_room->creature->health <= 0) {
        printf("There is no creature to attack here.\n");
        return;
    }

    Creature* creature = current_room->creature;
    printf("You engage in combat with %s!\n", creature->name);

    while(creature->health > 0 && player->health > 0) {
        // Player attacks
        printf("You attack %s for %d damage.\n", creature->name, player->strength);
        creature->health -= player->strength;
        if(creature->health <= 0) {
            printf("You have defeated %s!\n", creature->name);
            // Creature is now dead
            return;
        }

        // Creature attacks
        printf("%s attacks you for %d damage.\n", creature->name, creature->strength);
        player->health -= creature->strength;
        if(player->health <= 0) {
            printf("You have been slain by %s.\n", creature->name);
            return;
        }
    }
}

// Save Game
void save_game(Player* player, Room* rooms[], Room* current_room, char* filepath) {
    FILE* fp = fopen(filepath, "wb");
    if(!fp) {
        printf("Failed to open save file: %s\n", filepath);
        return;
    }

    // Save player stats
    fwrite(&(player->health), sizeof(int), 1, fp);
    fwrite(&(player->strength), sizeof(int), 1, fp);
    fwrite(&(player->inventory_capacity), sizeof(int), 1, fp);

    // Save inventory items count
    int inventory_count = 0;
    for(int i = 0; i < player->inventory_capacity; i++) {
        if(player->inventory[i]) inventory_count++;
    }
    fwrite(&inventory_count, sizeof(int), 1, fp);

    // Save each inventory item name
    for(int i = 0; i < player->inventory_capacity; i++) {
        if(player->inventory[i]) {
            fwrite(player->inventory[i]->name, sizeof(char), MAX_NAME_LEN, fp);
        }
    }

    // Save current room index
    int current_room_index = -1;
    for(int i = 0; i < MAX_ROOMS; i++) {
        if(rooms[i] == current_room) {
            current_room_index = i;
            break;
        }
    }
    fwrite(&current_room_index, sizeof(int), 1, fp);

    // Save all rooms data
    for (int i = 0; i < MAX_ROOMS; i++) {
        // Write item_count
        fwrite(&(rooms[i]->item_count), sizeof(int), 1, fp);
        // Write items
        for (int j = 0; j < rooms[i]->item_count; j++) {
            fwrite(rooms[i]->items[j]->name, sizeof(char), MAX_NAME_LEN, fp);
        }

        // Write creature data
        int creature_present = (rooms[i]->creature != NULL && rooms[i]->creature->health > 0);
        fwrite(&creature_present, sizeof(int), 1, fp);
        if (creature_present) {
            fwrite(rooms[i]->creature->name, sizeof(char), MAX_NAME_LEN, fp);
            fwrite(&(rooms[i]->creature->health), sizeof(int), 1, fp);
            fwrite(&(rooms[i]->creature->strength), sizeof(int), 1, fp);
        }
    }

    fclose(fp);
    printf("Game saved successfully to %s.\n", filepath);
}

// Load Game
void load_game(Player* player, Room* rooms[], Room** current_room, char* filepath) {
    FILE* fp = fopen(filepath, "rb");
    if(!fp) {
        printf("Failed to open save file: %s\n", filepath);
        return;
    }

    // Load player stats
    fread(&(player->health), sizeof(int), 1, fp);
    fread(&(player->strength), sizeof(int), 1, fp);
    fread(&(player->inventory_capacity), sizeof(int), 1, fp);

    // Clear current inventory
    for(int i = 0; i < player->inventory_capacity; i++) {
        if(player->inventory[i]) {
            free(player->inventory[i]);
            player->inventory[i] = NULL;
        }
    }

    // Load inventory items count
    int inventory_count;
    fread(&inventory_count, sizeof(int), 1, fp);

    // Load each inventory item
    for(int i = 0; i < inventory_count; i++) {
        char item_name[MAX_NAME_LEN];
        fread(item_name, sizeof(char), MAX_NAME_LEN, fp);

        // Find the first empty slot in inventory
        int slot = -1;
        for(int j = 0; j < player->inventory_capacity; j++) {
            if(!player->inventory[j]) {
                slot = j;
                break;
            }
        }

        if(slot != -1) {
            Item* new_item = (Item*)malloc(sizeof(Item));
            if(new_item) {
                strncpy(new_item->name, item_name, MAX_NAME_LEN);
                player->inventory[slot] = new_item;
            }
        }
    }

    // Load current room index
    int current_room_index;
    fread(&current_room_index, sizeof(int), 1, fp);
    if(current_room_index >=0 && current_room_index < MAX_ROOMS) {
        *current_room = rooms[current_room_index];
    } else {
        printf("Invalid room index in save file.\n");
    }

    // Load rooms data
    for (int i = 0; i < MAX_ROOMS; i++) {
        // Clear existing items
        for (int j = 0; j < rooms[i]->item_count; j++) {
            free(rooms[i]->items[j]);
            rooms[i]->items[j] = NULL;
        }
        rooms[i]->item_count = 0;

        int item_count;
        fread(&item_count, sizeof(int), 1, fp);
        for (int j = 0; j < item_count; j++) {
            char item_name[MAX_NAME_LEN];
            fread(item_name, sizeof(char), MAX_NAME_LEN, fp);
            Item* new_item = (Item*)malloc(sizeof(Item));
            strcpy(new_item->name, item_name);
            rooms[i]->items[rooms[i]->item_count++] = new_item;
        }

        int creature_present;
        fread(&creature_present, sizeof(int), 1, fp);
        if (creature_present) {
            if (rooms[i]->creature == NULL) {
                rooms[i]->creature = (Creature*)malloc(sizeof(Creature));
            }
            fread(rooms[i]->creature->name, sizeof(char), MAX_NAME_LEN, fp);
            fread(&(rooms[i]->creature->health), sizeof(int), 1, fp);
            fread(&(rooms[i]->creature->strength), sizeof(int), 1, fp);
        } else {
            // If previously allocated, free it
            if (rooms[i]->creature) {
                free(rooms[i]->creature);
                rooms[i]->creature = NULL;
            }
        }
    }

    fclose(fp);
    printf("Game loaded successfully from %s.\n", filepath);
}