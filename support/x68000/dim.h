//
//  dim.hpp
//  dim2d88
//
//  Created by mura on 2017/11/27.
//  Copyright © 2017年 murasuke. All rights reserved.
//

#ifndef X68000_DIM_H
#define X68000_DIM_H

#include <iostream>
#include <vector>
#include "d88.h"
#include "../../file_io.h"

class DimFile {
public:
    static constexpr uint8_t MAX_TRACK = 170;                 //maximum number of tracks
    static constexpr size_t MAX_DATA_SIZE = MAX_TRACK * 9 * 1024; //maximum dim data size
    static constexpr uint8_t INIT_VALUE = 0xe5;              //initial value of disk image
    static constexpr size_t DIM_STD_SIZE = 1261824;          //size of supported DIM format

    // disk type
    enum class FDType : uint8_t {
        FT_2HD = 0,
        FT_2HS = 1,
        FT_2HC = 2,
        FT_2HDE = 3,
        FT_2HQ = 9,
    };

    //dim file header
    using Header = struct {
        FDType type;
        uint8_t trkflag[MAX_TRACK];
        uint8_t info[15];
        uint8_t date[4];
        uint8_t time[4];
        uint8_t comment[61];
        uint8_t overtrack;
    };

public:
    DimFile() noexcept;

    DimFile(fileTYPE *file) noexcept(false);

    virtual ~DimFile() = default;

    //read dim file
    void read(fileTYPE *filename) noexcept(false);

    friend std::istream &operator>>(std::istream &in, DimFile &dim) noexcept(false);

    //get property
    size_t trackSize() const;

    uint8_t *trackData(int trk);

    FDType type() const;

    bool isValidFDType() const;

    void eachTrack(D88 *d88, D88Track *track);

private:
    void init();

    static const size_t _trksize[];    //FDType->track size

    int _maxtrk;        //maximum track number
    Header _header;     //header
    std::vector<uint8_t> _data;   //row data
};

#endif /* X68000_DIM_H */
