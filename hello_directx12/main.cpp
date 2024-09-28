// Liam Wynn, 9/26/2024, Hello DirectX 12

/*
	This is a simple program to render a triangle with
	a texture. I will start by doing the implementation
	as found here:

	https://github.com/microsoft/DirectX-Graphics-Samples/tree/master/Samples/Desktop/D3D12HelloWorld/src/HelloTexture

	And then expand it to load a texture from a PNG and
	use that data. After that, I would like to add color
	values to each vertex so we can get a mix of a texture
	+ a color.
*/

#include "stdafx.h"
#include "system_handler.h"

int main() {
	bool result;
	system_handler* system;

	system = new system_handler;
	result = initialize_system(system, 640, 480, false);

	if (result) {
		run_system(system);
	}

	shutdown_system(system);
	delete system;
	system = NULL;

	return 0;
}
