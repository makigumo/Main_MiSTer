//
//  d88.cpp
//  dim2d88
//
//  Created by mura on 2017/11/27.
//  Copyright © 2017年 murasuke. All rights reserved.
//

#include <fstream>
#include <sstream>

#include "d88.h"
#include "string.h"

/*
 * D88Track
 */

D88Track::D88Track(D88 *d88, int track) {
    if (d88->numSide() == D88::FDSide::TWO) {
        _header.track = track >> 1;
        _header.side = track & 1;
    } else {
        _header.track = track;
        _header.side = 0;
    }

    _header.sector = 0;
    _fullsectorsize = d88->sectorSize();
    setSectorSize(_fullsectorsize);
    _header.numsector = d88->numSector();

    setDensity(d88->fdType());

    _header.deleted = 0;
    _header.status = 0;
    _header.size = 0;
    _data = nullptr;
    memset(_header.reserve, 0, 5);
}

D88Track::~D88Track() {
    delete[] _data;
}

bool D88Track::setSectorSize(size_t size) {
    switch (size) {
        case 128:
            _header.sectorsize = SectorSize::S128;
            break;
        case 256:
            _header.sectorsize = SectorSize::S256;
            break;
        case 512:
            _header.sectorsize = SectorSize::S512;
            break;
        case 1024:
            _header.sectorsize = SectorSize::S1024;
            break;
        default:
            return false;
    }
    return true;
}

bool D88Track::setDensity(D88::FDType type) {
    switch (type) {
        case D88::FDType::FD2D:
        case D88::FDType::FD2DD:
            _header.density = Density::FDENS_D;
            break;
        case D88::FDType::FD2HD:
            // this should probably be
            // _header.density = Density::FDENS_HD;
            // but Virtual Floppy Image Converter (https://www.vector.co.jp/soft/dl/win95/util/se151106.html)
            // is doing this and the core only supports this
            _header.density = Density::FDENS_D;
            break;

        default:
            return false;
    }
    return true;
}

void D88Track::setData(const uint8_t *data, size_t size) {
    delete[] _data;

    int bufsz = _header.numsector * _fullsectorsize;
    _data = new uint8_t[bufsz];
    _header.size = _fullsectorsize;

    if (data) {
        memcpy(_data, data, size);
    } else {
        size = 0;
    }
    memset(_data + size, 0, bufsz - size);
}

int D88Track::sectorSize() const { return (_fullsectorsize); }

const D88Track::SectorHeader *D88Track::header() const { return (&_header); }

const uint8_t *D88Track::data() const { return (_data); }

int D88Track::dataSize() const { return (_header.size); }

int D88Track::wholeSize() const { return ((_header.size + D88SECTOR_HDR_SIZE) * _header.numsector); }


void D88Track::writeSector(std::ostream &str) {
    uint8_t *dataptr = _data;
    for (uint8_t sector = 1; sector <= _header.numsector; sector++, dataptr += _fullsectorsize) {
        _header.sector = sector;
        str.write(reinterpret_cast<const char *>(&_header), D88Track::D88SECTOR_HDR_SIZE);
        str.write(reinterpret_cast<const char *>(dataptr), _fullsectorsize);
    }
}

/*
 * D88
 */

D88::D88(FDType type, const std::string &name) {
    if (!setFDType(type)) throw std::runtime_error("D88::D88: Unsupported type");
    strncpy((char *) &_header.name, name.c_str(), 16);
    _header.name[16] = 0;
    memset(_header.reserve, 0, 9);
    _header.protect = 0;

    for (auto &i : _track) {
        i = nullptr;
    }
}

D88::~D88() {
    for (auto &i : _track) {
        delete i;
    }
}

bool D88::setFDType(FDType type) {
    switch (type) {
        case FDType::FD2D:
            setFDParm(40, FDSide::TWO, 16, 256, type);
            break;
        case FDType::FD2DD:
            setFDParm(80, FDSide::TWO, 16, 256, type);
            break;
        case FDType::FD2HD:
            setFDParm(77, FDSide::TWO, 8, 1024, type);
            break;
        default:
            return false;
    }
    return true;
}

void D88::setFDParm(int trk, FDSide side, int sector, int secsize, FDType type) {
    _numtrack = trk;
    _numside = side;
    _numsector = sector;
    _sectorsize = secsize;
    _header.type = type;
    _type = type;
}

int D88::numTrack() const { return (_numtrack); }

int D88::sectorSize() const { return (_sectorsize); }

uint8_t D88::numSector() const { return (_numsector); }

D88::FDSide D88::numSide() const { return (_numside); }

D88::FDType D88::fdType() const { return (_type); }

const D88::Header *D88::header() const { return (&_header); }


D88Track *D88::track(int trk, TrackCreate create) {
    if (trk < 0 || trk >= MAX_TRACKS) return nullptr;
    if (!_track[trk] && create == TrackCreate::YES) {
        _track[trk] = new D88Track(this, trk);
    }
    return (_track[trk]);
}


void D88::fixHeader() {
    uint32_t secptr = HDR_SIZE;
    for (int i = 0; i < MAX_TRACKS; i++) {
        if (_track[i]) {
            _header.table[i] = secptr;
            secptr += _track[i]->wholeSize();
        } else {
            _header.table[i] = 0;
        }
    }
    _header.size = secptr;
}

bool D88::write(const char *filename) {
    std::ofstream out;
    out.open(filename, std::ios_base::binary);
    if (!out) throw std::runtime_error("ERROR: D88::write: file write error");
    out << *this;
    out.close();
    return true;
}

bool D88::write(fileTYPE *f_out) {
    if (!FileSeek(f_out, 0, SEEK_SET)) return false;
    std::stringstream str;
    str << *this;
    std::string s = str.str();
    if (!FileWriteAdv(f_out, const_cast<char *>(s.c_str()), s.length())) {
        return false;
    }
    f_out->size = FileGetSize(f_out);
    printf("INFO: Virtual D88 size = %llu\n", f_out->size);
    return true;
}

std::ostream &operator<<(std::ostream &str, D88 &d88) {
    d88.fixHeader();

    D88Track *trk;
    str.write(reinterpret_cast<const char *>(d88.header()), D88::HDR_SIZE);
    for (int i = 0; i < D88::MAX_TRACKS; i++) {
        trk = d88.track(i);
        if (trk) {
            trk->writeSector(str);
        }
    }
    return (str);
}

