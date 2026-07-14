#include "assets/pack.h"

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <string>
#include <unordered_map>

#include "raylib.h"

#include "assets_pak.h" // generated: g_assetsPak[], g_assetsPakSize

namespace assets
{
namespace
{
struct Entry
{
    unsigned int offset;   // into the blob section
    unsigned int rawSize;  // decompressed size
    unsigned int compSize; // compressed (raw DEFLATE) size
};

std::unordered_map<std::string, Entry> g_index;
const unsigned char* g_blobs = nullptr;
bool g_mounted = false;

unsigned int ReadU32(const unsigned char* p) { return static_cast<unsigned int>(p[0]) | (static_cast<unsigned int>(p[1]) << 8) | (static_cast<unsigned int>(p[2]) << 16) | (static_cast<unsigned int>(p[3]) << 24); }

// Parse the pack header + index. Blob offsets are relative to the blob section,
// so we resolve g_blobs to point at the first byte past the index.
bool ParseIndex()
{
    if (g_assetsPakSize < 12 || std::memcmp(g_assetsPak, "SWPK", 4) != 0) return false;

    const unsigned char* p = g_assetsPak + 4;
    const unsigned int version = ReadU32(p);
    p += 4;
    const unsigned int count = ReadU32(p);
    p += 4;
    if (version != 1) return false;

    const unsigned char* end = g_assetsPak + g_assetsPakSize;
    for (unsigned int i = 0; i < count; ++i)
    {
        if (p + 4 > end) return false;
        const unsigned int pathLen = ReadU32(p);
        p += 4;
        if (p + pathLen + 12 > end) return false;
        std::string path(reinterpret_cast<const char*>(p), pathLen);
        p += pathLen;
        Entry e;
        e.offset = ReadU32(p);
        e.rawSize = ReadU32(p + 4);
        e.compSize = ReadU32(p + 8);
        p += 12;
        g_index.emplace(std::move(path), e);
    }
    g_blobs = p; // blob section starts right after the index
    return true;
}

// Read a file straight from disk (Debug fallback). Cannot use raylib's
// LoadFileData here: the callback is installed, so that would recurse.
unsigned char* ReadDiskData(const char* fileName, int* dataSize)
{
    std::FILE* f = std::fopen(fileName, "rb");
    if (f == nullptr) return nullptr;
    std::fseek(f, 0, SEEK_END);
    long size = std::ftell(f);
    std::fseek(f, 0, SEEK_SET);
    if (size <= 0)
    {
        std::fclose(f);
        return nullptr;
    }
    unsigned char* buffer = static_cast<unsigned char*>(MemAlloc(static_cast<unsigned int>(size)));
    size_t read = std::fread(buffer, 1, static_cast<size_t>(size), f);
    std::fclose(f);
    if (read != static_cast<size_t>(size))
    {
        MemFree(buffer);
        return nullptr;
    }
    if (dataSize != nullptr) *dataSize = static_cast<int>(size);
    return buffer;
}

// Decompress a pack entry into a freshly allocated buffer (raylib takes
// ownership and frees it via MemFree/UnloadFileText).
unsigned char* Inflate(const Entry& e, int* dataSize)
{
    int outSize = 0;
    unsigned char* data = DecompressData(g_blobs + e.offset, static_cast<int>(e.compSize), &outSize);
    if (data == nullptr) return nullptr;
    if (dataSize != nullptr) *dataSize = outSize;
    return data;
}

unsigned char* LoadDataCb(const char* fileName, int* dataSize)
{
    auto it = g_index.find(fileName);
    if (it != g_index.end()) return Inflate(it->second, dataSize);

#if defined(DEBUG_BUILD)
    unsigned char* disk = ReadDiskData(fileName, dataSize);
    if (disk != nullptr) return disk;
#endif
    TraceLog(LOG_WARNING, "PACK: asset not found: %s", fileName);
    if (dataSize != nullptr) *dataSize = 0;
    return nullptr;
}

char* LoadTextCb(const char* fileName)
{
    int size = 0;
    unsigned char* data = LoadDataCb(fileName, &size);
    if (data == nullptr) return static_cast<char*>(MemAlloc(1)); // empty, null-terminated

    // raylib frees text via UnloadFileText -> RL_FREE, so allocate with MemAlloc
    // and null-terminate. The inflated/disk buffer isn't guaranteed to have room.
    char* text = static_cast<char*>(MemAlloc(static_cast<unsigned int>(size) + 1));
    std::memcpy(text, data, static_cast<size_t>(size));
    text[size] = '\0';
    MemFree(data);
    return text;
}
} // namespace

bool Exists(const char* path)
{
    if (path == nullptr) return false;
    if (g_index.find(path) != g_index.end()) return true;
#if defined(DEBUG_BUILD)
    return FileExists(path);
#else
    return false;
#endif
}

std::vector<std::string> List(const char* prefix)
{
    std::vector<std::string> out;
    const std::string pre = prefix != nullptr ? prefix : "";
    for (const auto& kv : g_index)
        if (kv.first.compare(0, pre.size(), pre) == 0) out.push_back(kv.first);

#if defined(DEBUG_BUILD)
    // Merge live-added disk files (not yet repacked) so Debug iteration works.
    if (DirectoryExists(pre.c_str()))
    {
        FilePathList disk = LoadDirectoryFiles(pre.c_str());
        for (unsigned int i = 0; i < disk.count; ++i)
        {
            std::string p = disk.paths[i];
            std::replace(p.begin(), p.end(), '\\', '/');
            if (std::find(out.begin(), out.end(), p) == out.end()) out.push_back(p);
        }
        UnloadDirectoryFiles(disk);
    }
#endif
    return out;
}

void MountPack()
{
    if (g_mounted) return;
    if (!ParseIndex())
    {
        TraceLog(LOG_ERROR, "PACK: failed to parse embedded asset pack");
        return;
    }
    SetLoadFileDataCallback(LoadDataCb);
    SetLoadFileTextCallback(LoadTextCb);
    g_mounted = true;
    TraceLog(LOG_INFO, "PACK: mounted %i assets", static_cast<int>(g_index.size()));
}
} // namespace assets
