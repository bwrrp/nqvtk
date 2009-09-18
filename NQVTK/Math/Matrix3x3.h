#pragma once

#include "Vector3.h"
#include <cmath>

namespace NQVTK
{
	class Matrix3x3 
	{
	public:
		union 
		{
			struct 
			{
				double a00, a01, a02;
				double a10, a11, a12;
				double a20, a21, a22;
			};
			double A[3][3];
		};

		Matrix3x3()
		{
			// Initialize to 0-matrix
			for (int j = 0; j < 3; j++) 
			{
				for (int i = 0; i < 3; i++)
				{
					A[i][j] = 0.0;
				}
			}
		}

		Matrix3x3(double (&m)[3][3])
		{
			// Initialize to given values
			for (int j = 0; j < 3; j++)
			{
				for (int i = 0; i < 3; i++)
				{
					A[i][j] = m[i][j];
				}
			}
		}

		static const Matrix3x3 identity()
		{
			double ID[3][3] = {
				{1.0, 0.0, 0.0}, 
				{0.0, 1.0, 0.0}, 
				{0.0, 0.0, 1.0}
			};
			return Matrix3x3(ID);
		}

		static const Matrix3x3 fromCols(Vector3 c1, Vector3 c2, Vector3 c3)
		{
			double M[3][3] = {
				{c1.x, c2.x, c3.x}, 
				{c1.y, c2.y, c3.y}, 
				{c1.z, c2.z, c3.z}
			};
			return Matrix3x3(M);
		}

		static const Matrix3x3 fromRows(Vector3 r1, Vector3 r2, Vector3 r3)
		{
			double M[3][3] = {
				{r1.x, r1.y, r1.z}, 
				{r2.x, r2.y, r2.z}, 
				{r3.x, r3.y, r3.z}
			};
			return Matrix3x3(M);
		}

		Vector3 row(int i) const { return Vector3(A[i][0], A[i][1], A[i][2]); }
		Vector3 col(int i) const { return Vector3(A[0][i], A[1][i], A[2][i]); }

		double trace() const { return a00 + a11 + a22; }

		const Matrix3x3 transpose() const
		{
			Matrix3x3 res;
			for (int j = 0; j < 3; j++) 
			{
				for (int i = 0; i < 3; i++)
				{
					res.A[i][j] = A[j][i];
				}
			}

			return res;
		}

		void eigen(Vector3 &eigenvals, Vector3 eigenvecs[3]) const
		{
			Matrix3x3 copyofme = *this;
			Matrix3x3 evec;
			jacobi(copyofme, eigenvals, evec);
			for (int i = 0; i < 3; i++)
			{
				eigenvecs[i] = evec.col(i);
			}
		}

		double det() const
		{
			return a00 * a11 * a22 - a00 * a12 * a21 - a01 * a10 * a22 
				+ a01 * a12 * a20 + a02 * a10 * a21 - a02 * a11 * a20;
		}

		// Operators
		Matrix3x3 operator+=(const Matrix3x3 &m2)
		{
			for (int j = 0; j < 3; j++) 
			{
				for (int i = 0; i < 3; i++) 
				{
					A[i][j] += m2.A[i][j];
				}
			}
			return *this;
		}

		Matrix3x3 operator-=(const Matrix3x3 &m2)
		{
			for (int j = 0; j < 3; j++) 
			{
				for (int i = 0; i < 3; i++) 
				{
					A[i][j] -= m2.A[i][j];
				}
			}
			return *this;
		}

		Matrix3x3 operator*=(double f)
		{
			for (int j = 0; j < 3; j++) 
			{
				for (int i = 0; i < 3; i++) 
				{
					A[i][j] *= f;
				}
			}
			return *this;
		}

	private:
		// Jacobi iteration for the solution of eigenvectors/eigenvalues of a 
		// 3x3 real symmetric matrix. Square 3x3 matrix a; output eigenvalues 
		// in w; and output eigenvectors in v. Resulting eigenvalues/vectors 
		// are sorted in decreasing order; eigenvectors are normalized.
		// Adapted from VTK (vtkMath.cxx) version 5.0.0.
		#define ROTATE(a,i,j,k,l) g=a[i][j];h=a[k][l];a[i][j]=g-s*(h+g*tau);\
				a[k][l]=h+s*(g-h*tau)
		void jacobi(Matrix3x3 &a, Vector3 &w, Matrix3x3 &v) const
		{
			const int MAX_ROTATIONS = 50;
			int i, j, k, iq, ip, numPos;
			double tresh, theta, tau, t, sm, s, h, g, c, tmp;
			double b[3], z[3];
			
			// initialize
			for (ip = 0; ip < 3; ip++) 
			{
				for (iq = 0; iq < 3; iq++) 
				{
					v.A[ip][iq] = 0.0f;
				}

				v.A[ip][ip] = 1.0f;

				b[ip] = w.V[ip] = a.A[ip][ip];
				z[ip] = 0.0f;
			}
			
			// begin rotation sequence
			for (i = 0; i < MAX_ROTATIONS; i++) 
			{
				sm = 0.0f;
				for (ip = 0; ip < 3 - 1; ip++) 
				{
					for (iq = ip + 1; iq < 3; iq++) 
					{
						sm += std::abs(a.A[ip][iq]);
					}
				}
				
				if (sm == 0.0f) break;
				
				if (i < 3) 
				{ // first 3 sweeps
					tresh = 0.2f * sm / (3.0f * 3.0f);
				} 
				else 
				{
					tresh = 0.0f;
				}
				
				for (ip = 0; ip < 3 - 1; ip++) 
				{
					for (iq = ip + 1; iq < 3; iq++) 
					{
						g = 100.0f * std::abs(a.A[ip][iq]);

						// after 4 sweeps
						if (i > 3 && 
							(std::abs(w.V[ip]) + g) == std::abs(w.V[ip]) && 
							(std::abs(w.V[iq]) + g) == std::abs(w.V[iq])) 
						{	
							a.A[ip][iq] = 0.0f;
						} 
						else if (std::abs(a.A[ip][iq]) > tresh) 
						{
							h = w.V[iq] - w.V[ip];
							if ((std::abs(h) + g) == std::abs(h)) 
							{
								t = (a.A[ip][iq]) / h;
							} 
							else 
							{
								theta = 0.5f * h / (a.A[ip][iq]);
								t = 1.0f / (std::abs(theta) + 
									sqrt(1.0f + theta * theta));
								if (theta < 0.0) t = -t;
							}
							c = 1.0f / sqrt(1.0f + t * t);
							s = t * c;
							tau = s / (1.0f + c);
							h = t * a.A[ip][iq];
							z[ip] -= h;
							z[iq] += h;
							w.V[ip] -= h;
							w.V[iq] += h;
							a.A[ip][iq] = 0.0f;

							// ip already shifted left by 1 unit
							for (j = 0; j <= ip - 1; j++) 
							{
								ROTATE(a.A,j,ip,j,iq);
							}
							// ip and iq already shifted left by 1 unit
							for (j = ip + 1; j <= iq - 1; j++) 
							{
								ROTATE(a.A,ip,j,j,iq);
							}
							// iq already shifted left by 1 unit
							for (j = iq + 1; j < 3; j++) 
							{
								ROTATE(a.A,ip,j,iq,j);
							}
							for (j = 0; j < 3; j++) 
							{
								ROTATE(v.A,j,ip,j,iq);
							}
						}
					}
				}
				
				for (ip = 0; ip < 3; ip++) 
				{
					b[ip] += z[ip];
					w.V[ip] = b[ip];
					z[ip] = 0.0;
				}
			}

			// sort eigenfunctions
			for (j = 0; j < 3 - 1; j++) 
			{
				k = j;
				tmp = w.V[k];
				for (i = j + 1; i < 3; i++) 
				{
					if (w.V[i] > tmp) 
					{
						k = i;
						tmp = w.V[k];
					}
				}
				if (k != j) 
				{
					w.V[k] = w.V[j];
					w.V[j] = tmp;
					for (i = 0; i < 3; i++) 
					{
						tmp = v.A[i][j];
						v.A[i][j] = v.A[i][k];
						v.A[i][k] = tmp;
					}
				}
			}
			// ensure eigenvector consistency (i.e., Jacobi can compute vectors 
			// that are negative of one another (.707,.707,0) and 
			// (-.707,-.707,0). This can wreak havoc in hyperstreamline/other 
			// stuff. We will select the most positive eigenvector.
			for (j = 0; j < 3; j++) 
			{
				for (numPos = 0, i = 0; i < 3; i++) 
				{
					if (v.A[i][j] >= 0.0f) numPos++;
				}
				if (numPos < 2) 
				{
					for(i = 0; i < 3; i++) 
					{
						v.A[i][j] *= -1.0f;
					}
				}
			}
		}
	#undef ROTATE
	};

	// Overloaded operators
	inline const Matrix3x3 operator+(const Matrix3x3 &m1, const Matrix3x3 &m2) 
	{
		Matrix3x3 res;
		for (int j = 0; j < 3; j++) 
		{
			for (int i = 0; i < 3; i++) 
			{
				res.A[i][j] = m1.A[i][j] + m2.A[i][j];
			}
		}
		return res;
	}

	inline const Matrix3x3 operator-(const Matrix3x3 &m1, const Matrix3x3 &m2) 
	{
		Matrix3x3 res;
		for (int j = 0; j < 3; j++) 
		{
			for (int i = 0; i < 3; i++) 
			{
				res.A[i][j] = m1.A[i][j] - m2.A[i][j];
			}
		}
		return res;
	}

	inline const Matrix3x3 operator*(double f, const Matrix3x3 &m) 
	{
		Matrix3x3 res;
		for (int j = 0; j < 3; j++) 
		{
			for (int i = 0; i < 3; i++) 
			{
				res.A[i][j] = f * m.A[i][j];
			}
		}
		return res;	
	}

	inline const Matrix3x3 operator*(const Matrix3x3 &m, double f) 
	{
		return f * m;
	}

	inline const Vector3 operator*(const Matrix3x3 &m, const Vector3 &v) 
	{
		// M . v (column vector)
		return Vector3(m.row(0).dot(v), m.row(1).dot(v), m.row(2).dot(v));
	}

	inline const Vector3 operator*(const Vector3 &v, const Matrix3x3 &m) 
	{
		// v . M (row vector)
		return Vector3(v.dot(m.col(0)), v.dot(m.col(1)), v.dot(m.col(2)));
	}

	inline const Matrix3x3 operator*(const Matrix3x3 &m1, const Matrix3x3 &m2) 
	{
		Matrix3x3 res;
		for (int j = 0; j < 3; j++) 
		{
			for (int i = 0; i < 3; i++) 
			{
				res.A[i][j] = m1.row(i).dot(m2.col(j));
			}
		}
		return res;	
	}

	// Vector direct product operator. The vector direct product of v1 and v2 
	// is the matrix resulting from multiplying v1 as a single-column matrix 
	// with v2 as a single-row matrix.
	inline const Matrix3x3 operator*(const Vector3 &v1, const Vector3 &v2) 
	{
		// v . vT (column vector . row vector)
		double res[3][3] = {
			{v1.x * v2.x, v1.x * v2.y, v1.x * v2.z}, 
			{v1.y * v2.x, v1.y * v2.y, v1.y * v2.z}, 
			{v1.z * v2.x, v1.z * v2.y, v1.z * v2.z}
		};
		return Matrix3x3(res);	
	}

	inline const Matrix3x3 operator/(double f, const Matrix3x3 &m)
	{
		Matrix3x3 res;
		for (int j = 0; j < 3; j++) 
		{
			for (int i = 0; i < 3; i++) 
			{
				res.A[i][j] = f / m.A[i][j];
			}
		}
		return res;
	}

	inline const Matrix3x3 operator/(const Matrix3x3 &m, double f)
	{
		Matrix3x3 res;
		for (int j = 0; j < 3; j++) 
		{
			for (int i = 0; i < 3; i++) 
			{
				res.A[i][j] = m.A[i][j] / f;
			}
		}
		return res;
	}
}
