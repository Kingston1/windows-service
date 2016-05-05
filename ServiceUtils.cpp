// ServiceUtils.cpp
//
// Author: Mikko Saarinki
// Copyright (c) 2016 Mikko Saarinki. All rights reserved.
//
#pragma once
#include "ServiceUtils.h"

#include <comdef.h> //for _com_error

#define _BUFFER_SIZE 256

namespace ServiceUtils
{
    std::string t_convert(const TCHAR *argh)
    {
#ifdef UNICODE
        size_t i = 0;
        char *buf = static_cast<char*>(malloc(_BUFFER_SIZE));

        // Conversion
        wcstombs_s(&i,
            buf, static_cast<size_t>(_BUFFER_SIZE),
            argh, static_cast<size_t>(_BUFFER_SIZE));

        std::string r(buf);
        if (buf) free(buf); // Free multibyte character buffer
        return r;
#else
        return std::string(argh);
#endif
    }

    //convenience method for human readable GetLastError()
    std::string win32Error(DWORD error)
    {
        _com_error com_error(HRESULT_FROM_WIN32(error));
        auto msg = com_error.ErrorMessage();
        return t_convert(msg);
    }
}
