#include <iostream>
#include <string>
#include "Windows.h"
#include "main.h"

int main()
{
  red("Hello World!");
  system("pause");
  return 0;
}

void red(std::string message)
{
  SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED);
  std::cout << message << std::endl;
  SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED);
}