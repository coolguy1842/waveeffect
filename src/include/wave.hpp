#ifndef __WAVE_HPP__
#define __WAVE_HPP__

#include <stdint.h>
#include "RGBLib/util/rgb.hpp"
#include "RGBLib/util/hsv.hpp"

enum WaveDirection {
    WAVELEFT = 0,
    WAVERIGHT
};

class WaveRow {
private:
    HSV maxHSV;
    HSV minHSV;

    HSV currentHSV;
    WaveDirection direction;

public:
    WaveRow(HSV currentHSV, HSV minHSV, HSV maxHSV, WaveDirection direction) {
        this->currentHSV = currentHSV;
        this->minHSV = minHSV;
        this->maxHSV = maxHSV;

        this->direction = direction;
    }

    void update(HSV shiftAmount) {
        currentHSV.H += shiftAmount.H * (direction == WaveDirection::WAVERIGHT ? -4 : 1);
        currentHSV.S += shiftAmount.S * (direction == WaveDirection::WAVERIGHT ? -4 : 1);
        currentHSV.V += shiftAmount.V * (direction == WaveDirection::WAVERIGHT ? -4 : 1);

        if(currentHSV.H > maxHSV.H) {
            currentHSV = maxHSV;
            direction = WaveDirection::WAVERIGHT;
        }
        else if(currentHSV.H < minHSV.H) {
            currentHSV = minHSV;
            direction = WaveDirection::WAVELEFT;
        }
    }

    HSV getHSV() {
        return this->currentHSV;
    }

};

class Wave {
private:
    WaveRow* rows;
    size_t rowsLen;
    
    double refreshRate;
    WaveDirection direction;


    HSV maxHSV;
    HSV minHSV;


    void init() {
        HSV rowHSV = minHSV;

        HSV addHSV = {
            ((maxHSV.H - minHSV.H) / rowsLen),
            ((maxHSV.S - minHSV.S) / rowsLen),
            ((maxHSV.V - minHSV.V) / rowsLen)
        };

        for(
            size_t i = 0; i < rowsLen;
                rowHSV.H += addHSV.H,
                rowHSV.S += addHSV.S,
                rowHSV.V += addHSV.V,
                i++) {
            rows[i] = WaveRow(rowHSV, minHSV, maxHSV, WaveDirection::WAVELEFT);
        }
    }

public:
    Wave(size_t numRows, HSV minHSV, HSV maxHSV, unsigned int refreshRate, WaveDirection direction) {
        this->rowsLen = numRows;
        this->rows = (WaveRow*)calloc(rowsLen, sizeof(WaveRow));

        this->refreshRate = refreshRate;
        this->direction = direction;

        this->minHSV = minHSV;
        this->maxHSV = maxHSV;

        init();
    }

    ~Wave() {
        free(rows);
    }


    RGB getRGB(int row) {
        return HSVToRGB(rows[row].getHSV());
    }

    void update(double shiftAmount) {
        HSV addHSV = {
            ((maxHSV.H - minHSV.H) / rowsLen) * shiftAmount,
            ((maxHSV.S - minHSV.S) / rowsLen) * shiftAmount,
            ((maxHSV.V - minHSV.V) / rowsLen) * shiftAmount
        };

        for(size_t i = 0; i < rowsLen; i++) {
            rows[i].update(addHSV);
        }
    }
};

#endif