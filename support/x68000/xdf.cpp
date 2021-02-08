#include <ext/stdio_filebuf.h>
#include "xdf.h"

struct membuf : std::streambuf {
    membuf(char *begin, char *end) {
        this->setg(begin, begin, end);
    }
};

XdfFile::XdfFile() noexcept {
    init();
}

XdfFile::XdfFile(fileTYPE *file) {
    init();
    read(file);
}

void XdfFile::read(fileTYPE *file) {
    if (file->zip == nullptr) {
        int posix_handle = fileno(file->filp);
        __gnu_cxx::stdio_filebuf<char> filebuf(posix_handle, std::ios::in);
        std::istream infile(&filebuf);
        if (!infile) throw std::runtime_error("XdfFile::read: cannot open");

        infile >> *this;
        if (!infile) throw std::runtime_error("XdfFile::read: file read error");
    } else {
        // XDF/2HD size is fixed
        static char file_buf[XDF_STD_SIZE];
        FileSeek(file, 0, 0);
        if (!FileReadAdv(file, file_buf, XDF_STD_SIZE, 1)) {
            throw std::runtime_error("XdfFile::read: error reading zip");
        }
        membuf sbuf(file_buf, file_buf + sizeof(file_buf));
        std::istream infile(&sbuf);
        if (!infile) throw std::runtime_error("XdfFile::read: cannot open from buf");

        infile >> *this;
        if (!infile) throw std::runtime_error("XdfFile::read: file read error");
    }
}

void XdfFile::init() {
    _data.reserve(MAX_DATA_SIZE);
    _data.resize(MAX_DATA_SIZE);
}

std::istream &operator>>(std::istream &in, XdfFile &xdf) {
    uint8_t *ptr = xdf._data.data();
    in.read(reinterpret_cast<char *>(ptr), XdfFile::MAX_DATA_SIZE);
    if (!in) throw std::runtime_error("XdfFile::>>: xdf image read error");

    return (in);
}

void XdfFile::eachTrack(D88 *d88, D88Track *track) {
    for (int trk = 0; trk <= XdfFile::TRACK_COUNT; trk++) {
        track = d88->track(trk, D88::TrackCreate::YES);
        if (track) {
            track->setData(trackData(trk), XdfFile::TRACK_SIZE);
        }
    }
}

uint8_t *XdfFile::trackData(int trk) {
    if (trk < 0 || trk > XdfFile::TRACK_COUNT) return nullptr;
    return (_data.data() + trk * XdfFile::TRACK_SIZE);
};
