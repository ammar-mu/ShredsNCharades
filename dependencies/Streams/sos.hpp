// Cascaded second-order sections IIR filter
// Copyright (C) 2020 Tyler Coy
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

namespace streams
{

struct SOSCoefficients
{
    float b[3];
    float a[2];
};

// Ammar: For some reason VS doesn't do member alignment that is required for SIMD. Modified to allocate in the Heap!! 
template <typename T, int max_num_sections>
class SOSFilter
{
public:
    SOSFilter()
    {   x_[0] = NULL;
        Init(0);
    }

    SOSFilter(int num_sections)
    {   x_[0] = NULL;
        Init(num_sections);
    }

    ~SOSFilter()
    {   if (x_[0] != NULL) _mm_free(x_[0]);
        x_[0] = NULL;
    }

    void Init(int num_sections)
    {   
        int i,n;
        T* p;
        num_sections_ = num_sections;
        if (x_[0] != NULL) _mm_free(x_[0]);
        if (num_sections_ <= 0)
        {    x_[0] = NULL; return;
        }

        n = num_sections_ + 1;
        // Allocate aligned space for SIMD operations
        p = (T *) _mm_malloc(3 * n * sizeof(T), 16);
        for (i = 0; i < n; i++)
        {   x_[i] = p; p += 3;
        }

        Reset();
    }

    void Init(int num_sections, const SOSCoefficients* sections)
    {
        Init(num_sections);
        SetCoefficients(sections);
    }

    void Reset()
    {
        for (int n = 0; n < num_sections_; n++)
        {
            x_[n][0] = 0.f;
            x_[n][1] = 0.f;
            x_[n][2] = 0.f;
        }

        x_[num_sections_][0] = 0.f;
        x_[num_sections_][1] = 0.f;
        x_[num_sections_][2] = 0.f;
    }

    void SetCoefficients(const SOSCoefficients* sections)
    {
        for (int n = 0; n < num_sections_; n++)
        {
            sections_[n].b[0] = sections[n].b[0];
            sections_[n].b[1] = sections[n].b[1];
            sections_[n].b[2] = sections[n].b[2];

            sections_[n].a[0] = sections[n].a[0];
            sections_[n].a[1] = sections[n].a[1];
        }
    }

    T Process(T in)
    {
        for (int n = 0; n < num_sections_; n++)
        {
            // Shift x state
            x_[n][2] = x_[n][1];
            x_[n][1] = x_[n][0];
            x_[n][0] = in;

            T out = 0.f;

            // Add x state
            out += sections_[n].b[0] * x_[n][0];
            out += sections_[n].b[1] * x_[n][1];
            out += sections_[n].b[2] * x_[n][2];

            // Subtract y state
            out -= sections_[n].a[0] * x_[n+1][0];
            out -= sections_[n].a[1] * x_[n+1][1];
            in = out;
        }

        // Shift final section x state
        x_[num_sections_][2] = x_[num_sections_][1];
        x_[num_sections_][1] = x_[num_sections_][0];
        x_[num_sections_][0] = in;

        return in;
    }

protected:
    int num_sections_;
    SOSCoefficients sections_[max_num_sections];
    T *x_[max_num_sections + 1];
};

}
