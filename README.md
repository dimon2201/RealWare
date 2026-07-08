# Triton Engine

High-performance C++23 game engine.

Inspired by Urho3D, Wicked Engine, HPL1 Engine, UE4.

# Features
- Modern C++23
- Disk file IO
- TrueType fonts
- Memory allocator (UE4-like)
- Multithreading (separate thread for rendering with double buffering, work queue)
- Event subscription/sending
- PhysX 5.3 physics
- OpenGL 4.3 rendering
- OpenAL 1.1 sound
- Assimp 3D model loading
- Weighted blended order-independent transparency (WBOIT)
- Single atlas for textures
- User-defined GLSL shaders

# Build
## Windows
1. Get CMake 4.4.0-rc3 or higher
2. Get Visual Studio 17 2022
3. For Debug, run "build_debug_all_windows.bat" (this will build engine static library, editor executable and example executables)
4. Built files are located in "engine/lib", "editor/bin", "Example*/bin" folders respectively

# License
MIT License

Copyright (c) 2023-2026, Dmitry Sirachenko

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
