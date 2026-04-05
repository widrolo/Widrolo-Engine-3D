#pragma once

#include <Engine/Types/CommonTypes.h>

#include "MatrixBase.h"
#include <glm.hpp>

#include "CommonMatracies.h"

namespace WEngine
{
    template <uint8 m1Rows, uint8 m1Columns, uint8 m2Rows, uint8 m2Columns>
    MatrixBase<m1Rows, m2Columns> MatrixMultiply(const MatrixBase<m1Rows, m1Columns>& m1, const MatrixBase<m2Rows, m2Columns>& m2)
    {
        constexpr uint8 Rows = m1Rows;
        constexpr uint8 Cols = m2Columns;

        MatrixBase<Rows, Cols> result;
        result.Identity();
        if (Rows != Cols)
            return result;

        std::array<std::array<float32, m1Columns>, Rows> aRows;
        std::array<std::array<float32, m2Rows>, Cols> aCols;

        for (int i = 0; i < Rows; i++)
            aRows[i] = m1.GetRow(i);
        for (int i = 0; i < Cols; i++)
            aCols[i] = m2.GetCol(i);

        for (int i = 0; i < Rows; i++)
        {
            for (int j = 0; j < Cols; j++)
            {
                result.SetValue(i, j, Dot<m2Rows>(aRows[i], aCols[j]));
            }
        }
        return result;
    }

    static Mat4x4 Glm4x4ToMat4x4(glm::mat4x4 mat)
    {
        Mat4x4 result;
        for (int i = 0; i < 4; i++)
        {
            for (int j = 0; j < 4; j++)
            {
                result.SetValue(i, j, mat[i][j]);
            }
        }
        return result;
    }
}
