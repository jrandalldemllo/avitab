/*
 *   AviTab - Aviator's Virtual Tablet
 *   Copyright (C) 2018 Folke Will <folko@solhost.org>
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU Affero General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Affero General Public License for more details.
 *
 *   You should have received a copy of the GNU Affero General Public License
 *   along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef SRC_LIBIMG_STITCHER_TILESOURCE_H_
#define SRC_LIBIMG_STITCHER_TILESOURCE_H_

#include "src/libimg/Image.h"
#include <string>
#include <vector>

namespace img {

template<typename T>
struct Point {
    T x {};
    T y {};
};

class TileSource {
public:
    // Basic information
    virtual int getMinZoomLevel() = 0;
    virtual int getMaxZoomLevel() = 0;
    virtual int getInitialZoomLevel() = 0;
    virtual Point<int> getTileDimensions(int zoom) = 0;
    virtual bool supportsWorldCoords() = 0;
    virtual Point<double> transformZoomedPoint(double oldX, double oldY, int oldZoom, int newZoom) = 0;

    // Control the underlying loader
    virtual void cancelPendingLoads() = 0;
    virtual void resumeLoading() = 0;

    // Query and load tile information
    virtual bool checkAndCorrectTileCoordinates(int &x, int &y, int zoom) = 0;
    virtual std::string suggestFilePathForTile(int x, int y, int zoom) = 0;
    virtual std::vector<uint8_t> loadTileImage(int x, int y, int zoom) = 0;

    // World position support
    virtual Point<double> worldToXY(double lon, double lat, int zoom) = 0;
    virtual Point<double> xyToWorld(double x, double y, int zoom) = 0;

    virtual ~TileSource() = default;
};

}

#endif /* SRC_LIBIMG_STITCHER_TILESOURCE_H_ */