#ifndef X68000_XDF_H
#define X68000_XDF_H

#include <iostream>
#include <vector>
#include "d88.h"

class XdfFile {
public:
    static constexpr uint8_t TRACK_COUNT = 77;
    static constexpr size_t TRACK_SIZE = 8 /* sectors */ * 1024 /* sector data size */;
    static constexpr size_t MAX_DATA_SIZE = TRACK_COUNT * 2 /* disk sides */ * TRACK_SIZE;
    static constexpr size_t XDF_STD_SIZE = MAX_DATA_SIZE;          //size of XDF format 1261568 bytes

    void eachTrack(D88 *pD88, D88Track *pTrack);

public:
    XdfFile() noexcept;

    XdfFile(fileTYPE *file) noexcept(false);

    virtual ~XdfFile() = default;

    //read dim file
    void read(fileTYPE *file) noexcept(false);

    friend std::istream &operator>>(std::istream &in, XdfFile &dim) noexcept(false);

private:
    void init();

    std::vector<uint8_t> _data;   //row data
    uint8_t *trackData(int trk);
};

#endif /* xdf_h */
