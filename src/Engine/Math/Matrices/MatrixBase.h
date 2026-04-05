#pragma once

#include <Engine/Types/CommonTypes.h>
#include <array>
#include <Engine/Math/Vectors/VecMath.h>

namespace WEngine
{
    template<uint8 Rows, uint8 Columns>
    struct MatrixBase
    {
    public:
        static constexpr uint8 RowCount = Rows;
        static constexpr uint8 ColumnCount = Columns;
        static constexpr uint16 Size = RowCount * ColumnCount;

        MatrixBase() = default;
        MatrixBase(const MatrixBase& other) : m_raw(other.m_raw) {}
        MatrixBase(const std::array<float32, Size>& vals) : m_raw(vals) {}
    private:
        union
        {
            std::array<float32, Size> m_raw;
            std::array<std::array<float32, ColumnCount>, RowCount> m_vals;
        };

    public:
        MatrixBase operator+(const MatrixBase& other) const
        {
            MatrixBase result;
            for (int i = 0; i < Size; i++)
            {
                result.m_vals[i] = m_raw[i] + other.m_vals[i];
            }
            return result;
        }
        MatrixBase operator-(const MatrixBase& other) const
        {
            MatrixBase result;
            for (int i = 0; i < Size; i++)
            {
                result.m_vals[i] = m_raw[i] - other.m_vals[i];
            }
            return result;
        }
        MatrixBase operator*(const float32& scalar) const
        {
            MatrixBase result;
            for (int i = 0; i < Size; i++)
            {
                result.m_vals[i] = m_raw[i] * scalar;
            }
            return result;
        }
        MatrixBase operator*(const MatrixBase& other) const
        {
            MatrixBase result;
            std::array<std::array<float32, ColumnCount>, RowCount> aRows;
            std::array<std::array<float32, RowCount>, ColumnCount> aCols;

            for (int i = 0; i < RowCount; i++)
                aRows[i] = GetRow(i);
            for (int i = 0; i < ColumnCount; i++)
                aCols[i] = other.GetCol(i);

            for (int i = 0; i < RowCount; i++)
            {
                for (int j = 0; j < ColumnCount; j++)
                {
                    m_vals[i][j] = Dot<RowCount>(aRows[i], aCols[j]);
                }
            }
            return result;
        }

    public:
        [[nodiscard]] std::array<float32, ColumnCount> GetRow(uint8 row) const
        {
            return m_vals[row];
        }
        [[nodiscard]] std::array<float32, RowCount> GetCol(uint8 col) const
        {
            // I think it should be just as fast as GetRow, since
            // it's doing the exact same thing. It would all
            // be in cache anyway soooo.

            std::array<float32, RowCount> ret;
            for (uint8 i = 0; i < RowCount; i++)
            {
                ret[i] = GetValue(i, col);
            }
            return ret;
        }

        [[nodiscard]] float32 GetValue(uint8 row, uint8 col) const
        {
            if (row > RowCount - 1 || col > ColumnCount - 1)
                return 0.0f;

            return m_vals[row][col];
        }

        void SetValue(uint8 row, uint8 col, float32 value)
        {
            if (row > RowCount - 1 || col > ColumnCount - 1)
                return;

            m_vals[row][col] = value;
        }

        void Identity()
        {
            // for correctness’s sake
            if constexpr (RowCount != ColumnCount)
                return;

            for (int i = 0; i < RowCount; i++)
            {
                for (int j = 0; j < ColumnCount; j++)
                {
                    if (i == j)
                        m_vals[i][j] = 1;
                    else
                        m_vals[i][j] = 0;
                }
            }
        }
        std::array<float32, Size> GetRawData()
        {
            return m_raw;
        }
    };
}
