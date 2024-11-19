#include "Window.h"
#include <time.h> 
#include <stdlib.h>
#include <firebase/app.h>

#include <iostream>


int main()
{
	srand((unsigned int)time(NULL));
	MAYUMI::Window window;
	window.Run();
} 