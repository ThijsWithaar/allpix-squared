#include "TensorMesh.hpp"


TensorMesh::Scalar& TensorMesh::operator[](std::array<int, Dimension> c)
{
    int offset = c[0];
    for(size_t d=1; d < Dimension; d++)
    {
        offset *= axes[d-1].size();
        offset += std::clamp<int>(c[d], 0, static_cast<int>(axes[d].size()) - 1);
    }
    return data[static_cast<size_t>(offset)];
}


const TensorMesh::Scalar& TensorMesh::operator[](std::array<int, Dimension> c) const
{
    return (*const_cast<TensorMesh*>(this))[c];
}


ROOT::Math::XYZVector TensorMesh::gradient(const ROOT::Math::XYZPoint& pos) const
{
    std::array<double,Dimension> c;
    pos.GetCoordinates(c[0], c[1], c[2]);

    // Lookup nearest neighbour coordinates
    std::array<int,Dimension> idx;
    for(size_t d=0; d < Dimension; d++)
    {
        auto it = std::lower_bound(begin(axes[d]), end(axes[d]), c[d]);
        idx[d] = static_cast<int>(std::distance(begin(axes[d]), it));
    }

    // Per dimension: lookup a neighbourhood of values, fit a quadratic function and evaluate
    Eigen::Array3d loc, grad;
    Eigen::Vector3d val;
    for(size_t d=0; d < Dimension; d++)
    {
        /* ToDo: Match the interpolation used in the Poisson solver
            a. Evaluate 2nd order derivative at idx[d] and idx[d+1], then interpolate linearly
            b. Linear mix (precomputed) filter-coefficients 3-taps lo, hi idex into 1 4-taps filter
        */

        // Lookup a neighbourhoud of (location, value) for dimension d
        idx[d]--;
        for(int i=0; i < 3; i++, idx[d]++)
        {
            val[i] = (*this)[idx];
            loc[i] = std::clamp(idx[d], 0, static_cast<int>(axes[d].size()) - 1);
        }
        idx[d] -= 2;

        // Fit a quadratic polynomial, return derivative at coordinate c
        for(int i=0; i < 3; i++)
            m_A.col(i) = loc.pow(i);
        m_svd.compute(m_A);

        // Evaluate the derivative of the polynomial
        auto& cf = m_svd.solve(val);
        grad[static_cast<int>(d)] = 2*cf[2]*c[d] + cf[1];
    }
    return {grad[0], grad[1], grad[2]};
}


std::optional<TensorMesh> LoadTensorMeshAscii(std::istream& is)
{
	std::string line, elem;
	std::vector<std::string> elems;
	auto ReadCsvLine = [&]() -> std::vector<std::string>&
	{
		std::getline(is, line, '\n');
        elems.clear();
        std::istringstream liness(line);
        while(std::getline(liness, elem, ','))
            elems.push_back(elem);
		return elems;
	};

	auto h_fmt = ReadCsvLine();
	if(h_fmt.size() != 1 || h_fmt[0] != "TensorMesh")
		return std::nullopt;
	auto h_dim = ReadCsvLine();
	if(h_dim[0] != "dimension")
		return std::nullopt;
	std::string modality{ReadCsvLine().at(1)};

    TensorMesh mesh;

	for(size_t d=0; d < TensorMesh::Dimension; d++)
	{
		auto axis_el = ReadCsvLine();
		for(auto el = axis_el.begin()+1; el != axis_el.end(); ++el)
			mesh.axes[d].push_back(std::stof(std::string(*el)));
	}

	std::getline(is, line, ',');
    assert(line == "data");
	while(std::getline(is, line, ','))
        mesh.data.push_back(std::stof(line));

	return mesh;
}
