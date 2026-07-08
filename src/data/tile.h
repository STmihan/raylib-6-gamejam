#ifndef DATA_TILE_H
#define DATA_TILE_H

namespace data
{
enum class TileType
{
    ConcreteRoad,
    Base,
    Wall,
    Field,
    Forest,
    SwampEdge,
    SwampCorner,
    SwampCenter,
    RedBorder,
};

struct TileConfig
{
    float moveModifier;
    bool passable;
    bool blocksGroundDamage;
};

inline TileType TileFromChar(char symbol)
{
    switch (symbol)
    {
    case 'R': return TileType::ConcreteRoad;
    case 'B': return TileType::Base;
    case 'W': return TileType::Wall;
    case '.': return TileType::Field;
    case 'T': return TileType::Forest;
    case 'E': return TileType::SwampEdge;
    case 'D': return TileType::SwampCorner;
    case 'C': return TileType::SwampCenter;
    default: return TileType::RedBorder;
    }
}

inline TileConfig TileConfigOf(TileType type)
{
    switch (type)
    {
    case TileType::ConcreteRoad: return {1.3f, true, false};
    case TileType::Base: return {1.0f, false, false};
    case TileType::Wall: return {1.0f, false, true};
    case TileType::Field: return {1.0f, true, false};
    case TileType::Forest: return {0.7f, true, false};
    case TileType::SwampEdge: return {1.0f, true, false};
    case TileType::SwampCorner: return {1.0f, true, false};
    case TileType::SwampCenter: return {0.7f, true, false};
    case TileType::RedBorder: return {1.0f, false, false};
    }
    return {1.0f, true, false};
}
}

#endif
