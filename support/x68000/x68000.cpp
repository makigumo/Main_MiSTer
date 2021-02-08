//
// Created by Dan on 2021/02/06.
//
#include "dim.h"
#include "d88.h"
#include "xdf.h"

#include "../../file_io.h"

static int x68000_convert_dim_to_d88(fileTYPE *f_in, fileTYPE *f_out) {
    try {
        //read dim image
        DimFile dim(f_in);
        if (dim.type() != DimFile::FDType::FT_2HD) throw std::runtime_error("this file not supported");

        //set image into d88
        D88 d88(D88::FDType::FD2HD, "virtual dim");
        D88Track *track = nullptr;
        dim.eachTrack(&d88, track);

        //write d88 image
        d88.write(f_out);

    } catch (std::exception &e) {
        printf("ERROR: %s\n", e.what());
        return 0;
    }
    return 1;
}

static int x68000_convert_xdf_to_d88(fileTYPE *f_in, fileTYPE *f_out) {
    try {
        //read xdf image
        XdfFile xdf(f_in);

        //set image into d88
        D88 d88(D88::FDType::FD2HD, "virtual xdf");
        D88Track *track = nullptr;
        xdf.eachTrack(&d88, track);

        //write d88 image
        d88.write(f_out);

    } catch (std::exception &e) {
        printf("ERROR: %s\n", e.what());
        return 0;
    }
    return 1;
}

int x68000_openDIM(const char *path, fileTYPE *f) {
    if (!FileOpenEx(f, "vdsk", -1)) {
        printf("ERROR: Failed to create vdsk\n");
        return 0;
    }

    fileTYPE f_in;
    if (!FileOpen(&f_in, path)) {
        FileClose(f);
        return 0;
    }

    int ret = x68000_convert_dim_to_d88(&f_in, f);
    FileClose(&f_in);

    if (!ret) {
        printf("X68000: Failed to convert DIM (%s).\n", path);
        FileClose(f);
    }

    return ret;
}

int x68000_open2HDXDF(const char *path, fileTYPE *f) {
    if (!FileOpenEx(f, "vdsk", -1)) {
        printf("ERROR: Failed to create vdsk\n");
        return 0;
    }

    fileTYPE f_in;
    if (!FileOpen(&f_in, path)) {
        FileClose(f);
        return 0;
    }

    int ret = x68000_convert_xdf_to_d88(&f_in, f);
    FileClose(&f_in);

    if (!ret) {
        printf("X68000: Failed to convert XDF (%s).\n", path);
        FileClose(f);
    }

    return ret;
}
