#include "mpointer/MPointer.h"
#include <iostream>

int main() {
    try {
        // Initialize the connection to the server
        MPointer<int>::Init("0.0.0.0:9999");

        // Create a new MPointer
        MPointer<int> myPtr = MPointer<int>::New();
        std::cout << "Created MPointer with ID: " << &myPtr << std::endl;

        // Assign a value to the MPointer
        *myPtr = 42;
        std::cout << "Assigned value 42 to MPointer." << std::endl;

        // Retrieve the value from the MPointer
        int value = *myPtr;
        std::cout << "Retrieved value from MPointer: " << value << std::endl;

        // Create a second MPointer and assign it the first
        MPointer<int> myPtr2 = MPointer<int>::New();
        myPtr2 = myPtr;
        std::cout << "Assigned myPtr to myPtr2. ID of myPtr2: " << &myPtr2 << std::endl;

        // Verify that the value in myPtr2 is the same
        int value2 = *myPtr2;
        std::cout << "Retrieved value from myPtr2: " << value2 << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return 0;
}