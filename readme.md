crender-mt
-----------------------------------

This is a real-time software renderer project written in C++ and x86 assembly (or corresponding intrinsics).

Original project name "Concept Engine". Project was active in 2010-2015.

Goal was to implement in software a typical GPU rendering pipeline. This requires JIT compiler for vertex and
pixel shaders, and proved to be the stumbling block at the time. Eventually project was abandoned in favor
of OpenGL based renderer.

------------------------------------
Folder structure

*software-renderer*

    *include*

        *ceng*

            Ceng API headers. Most classes are pure virtual interfaces, which is necessary when
            they are used from a DLL.

            Division to the subfolders is a bit fuzzy.

            *datatypes*

                Structs and other POD types.

                Includes template string class that supports variable length encodings. This
                allows it to step by symbol rather than by byte.

            *enums*

            *interfaces*

                Most interfaces of the API. Some are in *datatypes* for some reason.

    *ceng-win32*

        Implements platform layer part of Ceng API. Handles I/O, GUI window creation,
        and the Direct3D back-end for displaying a 2d image.

        It's purpose is similar to Simple Directmedia layer (SDL) library, but more limited
        in functionality.

        Builds into a DLL-

    *crender-mt*

        - Software renderer DLL. Implements 3d rendering portion of Ceng API.

        - Makes extensive use of SSE and SSE2 instructions to do:

            - Texture mapping

            - Depth-stencil tests

            - Rasterizer

        - Uses multiple threads in the rasterizer and pixel shader stages since these
          parts of the pipeline are embarassingly parallel by screen area

    *sof-render-test*

        Software renderer debug application. Now it is mainly for profiling the software renderer pipeline.

    *alg-test*

        For occasional testing of x86 assembly (SIMD) versions of algorithms.

    *string-test*

        Occasional testing for the string class.

    *liblog*

        Simple logging library. Provided as static library to avoid overhead of DLL calls.

    *timerlib*

        Simple high precision timer. Provided as static library to avoid overhead of DLL calls.








