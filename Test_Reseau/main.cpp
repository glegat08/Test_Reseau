#include "Client.h"
#include "Serveur.h"
#include <iostream>

int main(int argc, char** argv)
{
    int choice;

    printf("1. Start server\n");
    printf("2. Start client\n");
    printf("Your choice: ");

    std::cin >> choice;
    std::cin.ignore();

    if (choice == 1)
    {
        printf("\n=== SERVER MODE ===\n");
        startServeur();
    }
    else if (choice == 2)
    {
        printf("\n=== CLIENT MODE ===\n");
        startClient();
    }
    else
    {
        printf("Invalid choice!\n");
    }

    return 0;
}