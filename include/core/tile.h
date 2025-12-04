#pragma once
#ifndef TILE_H
#define TILE_H

#include <QObject>
#include <QSize>

class Tile
{
public:
    Tile();
    Tile(uint16_t tileId, uint16_t xflip, uint16_t yflip, uint16_t palette);
    Tile(uint32_t raw);

public:
    uint32_t tileId:10;
    uint32_t xflip:1;
    uint32_t yflip:1;
    uint32_t tileIndex:4;
    uint32_t palette:8;
    uint32_t unused:8;
    uint32_t rawValue() const;

    Qt::Orientations orientation() const;
    void flip(QImage *image) const;

    static int getIndexInTileset(int);

    static const uint32_t maxValue;

    static constexpr int pixelWidth() { return 8; }
    static constexpr int pixelHeight() { return 8; }
    static constexpr QSize pixelSize() { return QSize(Tile::pixelWidth(), Tile::pixelHeight()); }
    static constexpr int numPixels() { return Tile::pixelWidth() * Tile::pixelHeight(); }
    static constexpr int sizeInBytes() { return sizeof(uint32_t); }
};

inline bool operator==(const Tile &a, const Tile &b) {
    return a.tileId  == b.tileId &&
           a.xflip   == b.xflip &&
           a.yflip   == b.yflip &&
           a.palette == b.palette;
}

inline bool operator!=(const Tile &a, const Tile &b) {
    return !(operator==(a, b));
}

#endif // TILE_H
