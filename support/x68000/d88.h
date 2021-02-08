//
//  d88.hpp
//  dim2d88
//
//  Created by mura on 2017/11/27.
//  Copyright © 2017年 murasuke. All rights reserved.
//

#ifndef X68000_D88_H
#define X68000_D88_H

#include <iostream>
#include <string>
#include "../../file_io.h"

/*
 * D88
 */
class D88Track;

class D88 {
public:
    static constexpr int MAX_TRACKS = 164;
    static constexpr int HDR_SIZE = 0x2b0;

    enum class FDType : uint8_t {
        FD2D = 0,
        FD2DD = 0x10,
        FD2HD = 0x20,
    };

    enum class TrackCreate : uint8_t {
        YES,
        NO
    };

    enum class FDSide : uint8_t {
        ONE = 0,
        TWO = 1,
    };

    using Header = struct {
        uint8_t name[17];
        uint8_t reserve[9];
        uint8_t protect;
        FDType type;
        uint32_t size;
        uint32_t table[MAX_TRACKS];
    };

public:
    D88(FDType type, const std::string &name);

    virtual ~D88();

    bool setFDType(FDType type);

    void setFDParm(int trk, FDSide side, int sector, int secsize, FDType type);

    int numTrack() const;

    int sectorSize() const;

    FDSide numSide() const;

    uint8_t numSector() const;

    FDType fdType() const;

    //トラックが存在しないとき、トラックを作成するなら２番めの引数に true を指定
    //未指定（または false）ならNULLが返る
    D88Track *track(int trk, TrackCreate create = TrackCreate::NO);

    const Header *header() const;

    //ヘッダのtableを確定する
    //ファイル出力をする前にかならずcallすること
    void fixHeader();

    bool write(const char *filename);

    bool write(fileTYPE *file);

private:
    //static const uint8_t _fdtype[];

protected:
    Header _header;
    D88Track *_track[MAX_TRACKS];

    int _numtrack;
    FDSide _numside;
    int _numsector;
    int _sectorsize;
    FDType _type;
};

//ファイル出力
std::ostream &operator<<(std::ostream &str, D88 &d88);



/*
 * トラック
 */

class D88Track {
public:
    static constexpr int D88SECTOR_HDR_SIZE = 0x10;

    /*
     * セクタサイズ
     */
    enum class SectorSize : uint8_t {
        S128 = 0,
        S256 = 1,
        S512 = 2,
        S1024 = 3
    };

    enum class Density : uint8_t {
        FDENS_D = 0,
        FDENS_S = 0x40,
        FDENS_HD = 1
    };

    /*
     *  セクタヘッダ
     */
    using SectorHeader = struct {
        uint8_t track;
        uint8_t side;
        uint8_t sector;
        SectorSize sectorsize;
        uint16_t numsector;
        Density density;
        uint8_t deleted;
        uint8_t status;
        uint8_t reserve[5];
        uint16_t size;
    };

public:
    D88Track(D88 *d88, int track);

    virtual ~D88Track();

    void setData(const uint8_t *data, size_t size);

    int sectorSize() const;

    const SectorHeader *header() const;

    const uint8_t *data() const;

    int dataSize() const;    //データのサイズ
    int wholeSize() const;    //全体サイズ（ヘッダ＋データ）

    void writeSector(std::ostream &str);

private:
//    void initSector();
//    int incrementSector();

private:
    //static const uint8_t _density[];

    bool setSectorSize(size_t size);

    bool setDensity(D88::FDType type);

private:
    SectorHeader _header;
    uint8_t *_data;
    int _fullsectorsize;
};


#endif /* X68000_D88_H */
