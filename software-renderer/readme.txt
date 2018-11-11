Folder structure

asm-test: 

	x86 assembler prototype

ceng-win32: 

	Platform layer for Windows. Provides:

	- threads
	- display of 2d images through direct3d
	- keyboard input
	- DLL management

	- Window creation

		NOTE: 
		Since WinAPI's window handler callback cannot provide a this-pointer, a dynamically generated
		assembly function with hardcoded this-pointer is used as callback to redirect WinAPI call to 
		correct window object's member function. See winproc-thunk.cpp

crender-mt:

	Software renderer back-end to the API. -mt for multithreaded.

	The three API root level classes

	cr-setup.cpp : implements RenderDevice
	crender-context.cpp, cr-scene.cpp : implements RenderContext
	cr-swap-chain.cpp : implements SwapChain

	Assembly or intrinsic heavy

	pshader-instance.h, pshader-instance.cpp:

		Parts of the pixel shader that are used to link input and output to class variables so that ShaderFunction() can have as simple syntax as possible. This
		approach could be modified to allow the client side to implement its own shaders with minimal effort, but that would require the
		client to know rendered implementation details.

	pshader-input.h:

		Calculates values of pixel shader input parameters for the current 2x2 quad. 

	shader-float.h, shader-float4.cpp:

		Classes used to write shader function is C++ in a syntax that is relatively close to HLSL.

	pshader-output.h, pshader-output.cpp:

		Class that is used in shader functions to write either local variable or texture sampling result into
		a render target.

	rtarget-buffer-swap.cpp:

		Pixel shader writes 2x2 quads to the render target in vertical layout (BBBB, GGGG, RRRR, AAAA) and this must be changed back to array of structs of 
		(ARGB, ARGB, ARGB, ARGB) and then converted into scanlines before sending to the display.

	ras-coverage-test.cpp:

		Methods used to determine for each pixel in a 8x8 tile whether that pixel is inside the triangle or not.
		SSE2 version is in intrinsics, but the old assembly version is still in the comments.

	cr-shader-view-tex2d : 

		Texture mapping functions. Functions Sample2D(), Linear_MipNearest_Port() and Linear_MipNearest_SSE2() implement bilinear
		texture filtering. SSE2 version is written in SSE intrinsics.



include:

	Headers for platform layer, renderer API and helper classes.

soft-render-test:

	Test application. Configures the renderer and displays texture inside a rectangular surface.



