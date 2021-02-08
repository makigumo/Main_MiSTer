//
//  dim.cpp
//  dim2d88
//
//  Created by mura on 2017/11/27.
//  Copyright © 2017年 murasuke. All rights reserved.
//
#include <ext/stdio_filebuf.h>
#include <fstream>
#include <string.h>
#include "dim.h"
#include "../../file_io.h"

// http://www.formauri.es/personal/pgimeno/pastes/dim2d88.py

const size_t DimFile::_trksize[] = {
        8192,   //2HD
        9216,   //2HS
        7680,   //2HC
        9216,   //2HDE
        0, 0, 0, 0, 0,
        9216    //2HQ
};

struct membuf : std::streambuf
{
    membuf(char* begin, char* end) {
        this->setg(begin, begin, end);
    }
};


DimFile::DimFile() noexcept {
    init();

}

DimFile::DimFile(fileTYPE *file) {
    init();
    read(file);
}

void DimFile::init() {
    _data.reserve(MAX_DATA_SIZE);
    _data.resize(MAX_DATA_SIZE);
    _maxtrk = 0;
}

void DimFile::read(fileTYPE *file) {
    if (file->zip == nullptr) {
        int posix_handle = fileno(file->filp);
        __gnu_cxx::stdio_filebuf<char> filebuf(posix_handle, std::ios::in);
        std::istream infile(&filebuf);
        if (!infile) throw std::runtime_error("DimFile::read: cannot open");

        infile >> *this;
        if (!infile) throw std::runtime_error("DimFile::read: file read error");
    } else {
        // we support only one DIM format, so this assumption on size is valid
        static char file_buf[DIM_STD_SIZE];
        if (!FileReadAdv(file, file_buf, DIM_STD_SIZE, 0)) {
            throw std::runtime_error("DimFile::read: error reading zip");
        }
        membuf sbuf(file_buf, file_buf + sizeof(file_buf));
        std::istream infile(&sbuf);
        if (!infile) throw std::runtime_error("DimFile::read: cannot open from buf");

        infile >> *this;
        if (!infile) throw std::runtime_error("DimFile::read: file read error");
    }
}

std::istream &operator>>(std::istream &in, DimFile &dim) {
    //read header
    in.read(reinterpret_cast<char *>(&(dim._header)), sizeof(DimFile::Header));
    if (!in) throw std::runtime_error("DimFile::>>: dim header read error");
    if (!dim.isValidFDType()) throw std::runtime_error("DimFile::>>: dim header invalid");

    //read data
    uint8_t *ptr = dim._data.data();
    size_t trksize = dim.trackSize();
    dim._maxtrk = 0;
    for (int i = 0; i < DimFile::MAX_TRACK; i++, ptr += trksize) {
        if (dim._header.trkflag[i]) {
            in.read(reinterpret_cast<char *>(ptr), (std::streamsize) trksize);
            if (!in) throw std::runtime_error("DimFile::>>: dim image read error");
            dim._maxtrk = i;
        } else {
            memset(ptr, DimFile::INIT_VALUE, trksize);
        }
    }

    return (in);
}

size_t DimFile::trackSize() const {
    return (_trksize[static_cast<int>(_header.type)]);
}

uint8_t *DimFile::trackData(int trk) {
    if (trk < 0 || trk > _maxtrk) return nullptr;
    return (_data.data() + trk * trackSize());
}


void DimFile::eachTrack(D88 *d88, D88Track *track) {
    size_t trksize = trackSize();
    for (int trk = 0; trk <= _maxtrk; trk++) {
        track = d88->track(trk, D88::TrackCreate::YES);
        if (track) {
            track->setData(trackData(trk), trksize);
        }
    }
}

DimFile::FDType DimFile::type() const {
    return _header.type;
}

bool DimFile::isValidFDType() const {
    //header.type
    switch (_header.type) {
        case FDType::FT_2HC:
        case FDType::FT_2HD:
        case FDType::FT_2HDE:
        case FDType::FT_2HQ:
        case FDType::FT_2HS:
            break;

        default:
            return false;
    }

    return true;
}
