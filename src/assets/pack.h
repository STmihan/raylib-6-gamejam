#ifndef ASSETS_PACK_H
#define ASSETS_PACK_H

#include <string>
#include <vector>

namespace assets
{
// Mount the embedded, DEFLATE-compressed asset pack and route all raylib file
// reads (LoadModel/LoadTexture/LoadShader/LoadSound/LoadFileData/...) through it
// via SetLoadFileDataCallback / SetLoadFileTextCallback.
//
// Must be called once after InitWindow() and before any assets are loaded.
//
// In Debug builds, a lookup miss falls back to reading the file from disk, so
// artists can iterate on assets/ without repacking. In Release builds a miss
// returns null (raylib logs the error) -- the binary is fully self-contained.
void MountPack();

// Does an asset exist? Checks the pack (and disk too, in Debug builds).
// Use instead of raylib's FileExists(), which only sees the real filesystem.
bool Exists(const char* path);

// List packed asset paths that start with prefix (e.g. "assets/sounds/").
// Use instead of LoadDirectoryFiles(), which only enumerates the real
// filesystem. In Debug builds, live-added disk files are merged in.
std::vector<std::string> List(const char* prefix);
}

#endif
