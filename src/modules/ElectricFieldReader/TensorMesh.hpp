#pragma once

#include <iostream>
#include <optional>

#include <Eigen/Dense>

#include <Math/Point3D.h>
#include <Math/Vector3D.h>


struct TensorMesh
{
    constexpr static int Dimension=3;
    using Scalar = float;

    Scalar& operator[](std::array<int, Dimension> c);

    const Scalar& operator[](std::array<int, Dimension> c) const;

    /**
    Apply Laplace transform to src.
    at the grid-points, the dimensions are independent, the filter can be pre-computed

    Laplace(dst, src)
    */

    ROOT::Math::XYZVector gradient(const ROOT::Math::XYZPoint& pos) const;

    std::array<std::vector<Scalar>,Dimension> axes;
    std::vector<Scalar> data;

private:
    mutable Eigen::Matrix3d m_A;
    mutable Eigen::JacobiSVD<Eigen::Matrix3d> m_svd;
};

std::optional<TensorMesh> LoadTensorMeshAscii(std::istream& is);
