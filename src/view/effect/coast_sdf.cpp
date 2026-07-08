#include "view/effect/coast_sdf.h"

#include <cmath>
#include <vector>

#include "data/space/hex.h"
#include "data/space/vec.h"
#include "data/space/world_config.h"

namespace view
{
namespace
{
    constexpr int SdfResolution = 1024;
    constexpr float HexCircumradius = 1.0f;

    void RoundToCell(float q, float r, int& col, int& row)
    {
        float x = q;
        float z = r;
        float y = -x - z;
        float rx = std::round(x);
        float ry = std::round(y);
        float rz = std::round(z);
        float dx = std::fabs(rx - x);
        float dy = std::fabs(ry - y);
        float dz = std::fabs(rz - z);
        if (dx > dy && dx > dz)
        {
            rx = -ry - rz;
        }
        else if (dy > dz)
        {
            ry = -rx - rz;
        }
        else
        {
            rz = -rx - ry;
        }
        int cubeX = static_cast<int>(rx);
        int cubeZ = static_cast<int>(rz);
        col = cubeX + (cubeZ - (cubeZ & 1)) / 2;
        row = cubeZ;
    }

    void SquaredDistance1D(const std::vector<float>& f, std::vector<float>& d, std::vector<int>& v,
                           std::vector<float>& z, int n)
    {
        const float inf = 1e18f;
        int k = 0;
        v[0] = 0;
        z[0] = -inf;
        z[1] = inf;
        for (int q = 1; q < n; q++)
        {
            float s = ((f[q] + static_cast<float>(q * q)) - (f[v[k]] + static_cast<float>(v[k] * v[k]))) /
                static_cast<float>(2 * q - 2 * v[k]);
            while (s <= z[k])
            {
                k--;
                s = ((f[q] + static_cast<float>(q * q)) - (f[v[k]] + static_cast<float>(v[k] * v[k]))) / static_cast
                    <float>(2 * q - 2 * v[k]);
            }
            k++;
            v[k] = q;
            z[k] = s;
            z[k + 1] = inf;
        }
        k = 0;
        for (int q = 0; q < n; q++)
        {
            while (z[k + 1] < static_cast<float>(q)) k++;
            float dq = static_cast<float>(q - v[k]);
            d[q] = dq * dq + f[v[k]];
        }
    }
}

Texture2D BuildCoastSdf(Vector2& originOut, float& worldSizeOut)
{
    float minX = 1e9f, maxX = -1e9f, minZ = 1e9f, maxZ = -1e9f;
    for (int row = 0; row < data::FieldRows; row++)
    {
        for (int col = 0; col < data::FieldCols; col++)
        {
            data::Vec2 logic = data::CellToLogic(col, row);
            float wx = logic.x * data::RenderScale;
            float wz = logic.y * data::RenderScale;
            minX = fminf(minX, wx);
            maxX = fmaxf(maxX, wx);
            minZ = fminf(minZ, wz);
            maxZ = fmaxf(maxZ, wz);
        }
    }

    float centerX = (minX + maxX) * 0.5f;
    float centerZ = (minZ + maxZ) * 0.5f;
    float half = fmaxf(maxX - minX, maxZ - minZ) * 0.5f + HexCircumradius + SdfMaxDist;
    Vector2 origin = {centerX - half, centerZ - half};
    float worldSize = half * 2.0f;
    int res = SdfResolution;
    float worldPerTexel = worldSize / static_cast<float>(res);

    const float inf = 1e18f;
    std::vector<float> grid(static_cast<std::size_t>(res) * res);
    for (int j = 0; j < res; j++)
    {
        for (int i = 0; i < res; i++)
        {
            float wx = origin.x + (static_cast<float>(i) + 0.5f) * worldPerTexel;
            float wz = origin.y + (static_cast<float>(j) + 0.5f) * worldPerTexel;
            float q = 0.5773503f * wx - 0.3333333f * wz;
            float r = 0.6666667f * wz;
            int col = 0, row = 0;
            RoundToCell(q, r, col, row);
            bool land = col >= 0 && col < data::FieldCols && row >= 0 && row < data::FieldRows;
            grid[static_cast<std::size_t>(j) * res + i] = land ? 0.0f : inf;
        }
    }

    std::vector<float> f(res), d(res), z(res + 1);
    std::vector<int> v(res);
    for (int i = 0; i < res; i++)
    {
        for (int j = 0; j < res; j++) f[j] = grid[static_cast<std::size_t>(j) * res + i];
        SquaredDistance1D(f, d, v, z, res);
        for (int j = 0; j < res; j++) grid[static_cast<std::size_t>(j) * res + i] = d[j];
    }
    for (int j = 0; j < res; j++)
    {
        for (int i = 0; i < res; i++) f[i] = grid[static_cast<std::size_t>(j) * res + i];
        SquaredDistance1D(f, d, v, z, res);
        for (int i = 0; i < res; i++) grid[static_cast<std::size_t>(j) * res + i] = d[i];
    }

    std::vector<unsigned char> pixels(static_cast<std::size_t>(res) * res);
    float invMax = 1.0f / SdfMaxDist;
    for (std::size_t k = 0; k < pixels.size(); k++)
    {
        float dist = std::sqrt(grid[k]) * worldPerTexel;
        float normalized = fminf(fmaxf(dist * invMax, 0.0f), 1.0f);
        pixels[k] = static_cast<unsigned char>(normalized * 255.0f + 0.5f);
    }

    Image image = {pixels.data(), res, res, 1, PIXELFORMAT_UNCOMPRESSED_GRAYSCALE};
    Texture2D texture = LoadTextureFromImage(image);
    SetTextureFilter(texture, TEXTURE_FILTER_BILINEAR);
    SetTextureWrap(texture, TEXTURE_WRAP_CLAMP);

    originOut = origin;
    worldSizeOut = worldSize;
    return texture;
}
}
