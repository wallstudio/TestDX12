#pragma once

#include "StringUtility.h"


inline void AssertOKImpl(HRESULT result, const char* code)
{
    using namespace std;

    if (FAILED(result))
    {
        auto sb = stringstream();
        sb << "Failed\n\n" << code << "\n\n" << ToMultiByte(ToTString(result).data());
        throw exception(sb.str().data());
    }
}
void AssertOK(HRESULT reuslt) { AssertOKImpl(reuslt, ""); }
// #define AssertOK(operation) AssertOKImpl(operation, #operation)