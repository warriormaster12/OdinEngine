#include "Include/core.h"

int main(int argc, char* argv[])
{

	Core::CoreInit();
	Core::CoreUpdate();
	Core::CoreCleanup();
	return 0;
}


