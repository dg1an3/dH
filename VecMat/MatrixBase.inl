//////////////////////////////////////////////////////////////////////
// MatrixBase.inl : implementation of the CMatrixBase class.
//
// Copyright (C) 2002
// $Id$
// U.S. Patent Pending
//////////////////////////////////////////////////////////////////////

#ifndef _MATRIXBASE_INL_
#define _MATRIXBASE_INL_

// class declaration
#include "MatrixBase.h"

// CVectorN
#include "VectorN.h"

// helper macros
#define SQR(a) ((a)*(a))
#define SIGN(a,b) ((b) >= 0.0 ? fabs(a) : -fabs(a))

// maximum iterations for SVD
#define MAX_ITER 30

//////////////////////////////////////////////////////////////////////
// pythag
//
// computes the c side of a triangle from a and b
//////////////////////////////////////////////////////////////////////
template<class TYPE>
TYPE pythag(TYPE a, TYPE b)
{
	TYPE absa = (TYPE) fabs(a);
	TYPE absb = (TYPE) fabs(b);

	if (absa > absb)
	{
		return absa * (TYPE) sqrt(1.0 + SQR(absb / absa));
	}
	else
	{
		return (absb == 0.0 ? 0.0 : absb * (TYPE) sqrt(1.0 + SQR(absa / absb)));
	}

}	// pythag


//////////////////////////////////////////////////////////////////////
// CMatrixBase<TYPE>::SetElements
//
// sets the elements of the matrix
//////////////////////////////////////////////////////////////////////
template<class TYPE>
void CMatrixBase<TYPE>::SetElements(int nCols, int nRows, TYPE *pElements)
{
	// delete previous data
	if (m_bFreeElements && NULL != m_pElements)
	{
		delete [] m_pElements;
		m_pElements = NULL;
	}

	if (NULL != m_arrColumns)
	{
		delete [] m_arrColumns;
		m_arrColumns = NULL;
	}

	m_nCols = nCols;
	m_nRows = nRows;

	m_pElements = pElements;

	// set up the column vectors
	if (0 != m_nCols)
	{
		// allocate column vectors
		m_arrColumns = new CVectorBase<TYPE>[GetCols()];

		// initialize the column vectors and the pointers
		for (int nAt = 0; nAt < GetCols(); nAt++)
		{
			// initialize the column vector
			m_arrColumns[nAt].SetElements(m_nRows, &m_pElements[nAt * GetRows()]);
		}
	}

	m_bFreeElements = FALSE;

}	// CMatrixBase<TYPE>::SetElements


//////////////////////////////////////////////////////////////////////
// CMatrixBase<TYPE>::Reshape
//
// sets the rows and columns of the matrix
//////////////////////////////////////////////////////////////////////
template<class TYPE>
void CMatrixBase<TYPE>::Reshape(int nCols, int nRows)
{
	if (GetRows() == nRows && GetCols() == nCols)
	{
		return;
	}

	// delete previous data
	if (m_bFreeElements && NULL != m_pElements)
	{
		delete [] m_pElements;
		m_pElements = NULL;
	}

	if (NULL != m_arrColumns)
	{
		delete [] m_arrColumns;
		m_arrColumns = NULL;
	}

	// assign the dimensions
	m_nRows = nRows;
	m_nCols = nCols;

	// set up the column vectors
	if (0 != m_nCols)
	{
		// allocate elements
		m_pElements = new TYPE[GetCols() * GetRows()];

		// allocate column vectors
		m_arrColumns = new CVectorBase<TYPE>[GetCols()];

		// initialize the column vectors and the pointers
		for (int nAt = 0; nAt < GetCols(); nAt++)
		{
			// initialize the column vector
			m_arrColumns[nAt].SetElements(m_nRows, &m_pElements[nAt * GetRows()]);
		}
	}

	// populate as an identity matrix
	SetIdentity();

}	// CMatrixBase<TYPE>::Reshape


//////////////////////////////////////////////////////////////////////
// CMatrixBase<TYPE>::Transpose
//
// transposes the matrix
//////////////////////////////////////////////////////////////////////
template<class TYPE>
void CMatrixBase<TYPE>::Transpose()
{
	CMatrixBase<TYPE> copy;
	copy.Reshape(GetRows(), GetCols());
	
	for (int nCol = 0; nCol < GetCols(); nCol++)
	{
		for (int nRow = 0; nRow < GetRows(); nRow++)
		{
			copy[nRow][nCol] = (*this)[nCol][nRow];
		}
	}

	(*this) = copy;

}	// CMatrixBase<TYPE>::Transpose


//////////////////////////////////////////////////////////////////////
// CMatrixBase<TYPE>::IsOrthogonal
//
// tests for orthogonality of the matrix
//////////////////////////////////////////////////////////////////////
template<class TYPE>
BOOL CMatrixBase<TYPE>::IsOrthogonal() const
{
	CMatrixBase<TYPE> mTrans = (*this);
	mTrans.Transpose();
	mTrans *= (*this);

	CMatrixBase<TYPE> mIdent(GetCols(), GetCols());
	mIdent.SetIdentity();

	return mTrans.IsApproxEqual(mTrans);

}	// CMatrixBase<TYPE>::IsOrthogonal


//////////////////////////////////////////////////////////////////////
// CMatrixBase<TYPE>::Orthogonalize
//
// orthogonalizes the input matrix using GSO
//////////////////////////////////////////////////////////////////////
template<class TYPE>
void CMatrixBase<TYPE>::Orthogonalize()
{
	// make a working copy of the matrix
	CMatrixBase<TYPE> mOrtho(*this);

	// normalize the first column vector
	mOrtho[0].Normalize();

	// apply to each row vector after the zero-th
	for (int nAtCol = 1; nAtCol < GetCols(); nAtCol++)
	{
		// normalize the next column vector
		mOrtho[nAtCol].Normalize();

		// ensure orthogonality with all previous column vectors
		for (int nAtOrthoCol = nAtCol - 1; nAtOrthoCol >= 0; 
				nAtOrthoCol--)
		{
			// compute the scale factor
			TYPE scalar = (mOrtho[nAtCol] * mOrtho[nAtOrthoCol]) 
				/ (mOrtho[nAtOrthoCol] * mOrtho[nAtOrthoCol]);

			mOrtho[nAtCol] -= scalar * mOrtho[nAtOrthoCol];

			// make sure we are now orthogonal to this
			ASSERT(mOrtho[nAtCol] * mOrtho[nAtOrthoCol] < EPS);
		}
	}

	// assign the result
	(*this) = mOrtho;

}	// CMatrixBase<TYPE>::Orthogonalize


//////////////////////////////////////////////////////////////////////
// CMatrixBase<TYPE>::Invert
//
// swaps two rows of the matrix
//////////////////////////////////////////////////////////////////////
template<class TYPE>
void CMatrixBase<TYPE>::Invert(BOOL bFullPivot)
{
	// only invert square matrices
	ASSERT(GetCols() == GetRows());

	// Gauss-Jordan elimination
	CMatrixBase<TYPE> mCopy(*this);		// the copy of this matrix
	CMatrixBase<TYPE> mInv(*this);		// stores the built inverse
	mInv.SetIdentity();

	// stores the sequence of column swaps for a full pivot
	valarray<int> arrColumnSwaps(GetCols());

	// helper vectors to hold rows
	CVectorN<TYPE> vRow(GetCols());
	CVectorN<TYPE> vOtherRow(GetCols());
		
	for (int nCol = 0; nCol < GetCols(); nCol++)
	{
		if (bFullPivot)
		{
			// CHECK THE FULL PIVOT
			// find the full pivot element
			int nPivotRow, nPivotCol;
			mCopy.FindPivotElem(nCol, &nPivotRow, &nPivotCol);

			// interchange the rows
			mCopy.InterchangeRows(nCol, nPivotRow);
			mCopy.InterchangeCols(nCol, nPivotCol);

			// interchange the columns
			mInv.InterchangeRows(nCol, nPivotRow);
			mInv.InterchangeCols(nCol, nPivotCol);
			
			// store the column interchange for later restoring
			arrColumnSwaps[nCol] = nPivotCol;
		}
		else	// partial pivot
		{
			// pivot if necessary
			int nPivotRow = mCopy.FindPivotRow(nCol);
			mCopy.InterchangeRows(nCol, nPivotRow);
			mInv.InterchangeRows(nCol, nPivotRow);
		}

    	// obtain a 1 in the diagonal position
		//		(the pivot ensures that copy[nCol][nCol] is not zero)
		// make sure we are numerically stable
		ASSERT(fabs(mCopy[nCol][nCol]) > 1e-8);

		// scale factor to be applied
    	TYPE scale = 1.0 / mCopy[nCol][nCol];	

		// scale the copy to get a 1.0 in the diagonal
		mCopy.GetRow(nCol, vRow);
		vRow *= scale;
		mCopy.SetRow(nCol, vRow);

		// scale the inverse by the same amount
		mInv.GetRow(nCol, vRow);
		vRow *= scale;
		mInv.SetRow(nCol, vRow);

		// obtain zeros in the off-diagonal elements
		int nRow;	// for index
	  	for (nRow = 0; nRow < GetRows(); nRow++) 
		{
    		if (nRow != nCol) 
			{
				// get the scale factor to be applied
    			scale = -mCopy[nCol][nRow];

				// add a scaled version of the diagonal row
				//		to obtain a zero at this row and column
				mCopy.GetRow(nCol, vRow);
				mCopy.GetRow(nRow, vOtherRow);
				vOtherRow += vRow * scale;
				mCopy.SetRow(nRow, vOtherRow);

				// same operation on the inverse
				mInv.GetRow(nCol, vRow);
				mInv.GetRow(nRow, vOtherRow);
				vOtherRow += vRow * scale;
				mInv.SetRow(nRow, vOtherRow);
    		}
    	}
	}

	if (bFullPivot)
	{
		// restores the sequence of columns
		for (int nAtCol = GetCols()-1; nAtCol >= 0; nAtCol--)
		{
			mInv.InterchangeCols(nAtCol, arrColumnSwaps[nAtCol]);
		}
	}

	// assign this to inverse
	(*this) = mInv; 

}	// CMatrixBase<TYPE>::Invert


//////////////////////////////////////////////////////////////////////
// CMatrixBase<TYPE>::InterchangeRows
//
// swaps two rows of the matrix
//////////////////////////////////////////////////////////////////////
template<class TYPE>
void CMatrixBase<TYPE>::InterchangeRows(int nRow1, int nRow2) 
{
	// check that the rows are not the same
	if (nRow1 != nRow2)
	{
		for (int nAtCol = 0; nAtCol < GetCols(); nAtCol++)
		{
			TYPE temp = (*this)[nAtCol][nRow1];
			(*this)[nAtCol][nRow1] = (*this)[nAtCol][nRow2];
			(*this)[nAtCol][nRow2] = temp;
		}
	}

}	// CMatrixBase<TYPE>::InterchangeRows

    
//////////////////////////////////////////////////////////////////////
// CMatrixBase<TYPE>::InterchangeCols
//
// swaps two COLUMNS of the matrix
//////////////////////////////////////////////////////////////////////
template<class TYPE>
void CMatrixBase<TYPE>::InterchangeCols(int nCol1, int nCol2) 
{
	// check that the cols are not the same
	if (nCol1 != nCol2)
	{
		// temporary vector storage
		CVectorN<TYPE> vTemp = (*this)[nCol1];	

		// and swap the two rows
		(*this)[nCol1] = (*this)[nCol2];
		(*this)[nCol2] = vTemp;
	}

}	// CMatrixBase<TYPE>::InterchangeRows

    
//////////////////////////////////////////////////////////////////////
// CMatrixBase<TYPE>::FindPivotRow
//
// finds the pivot element (returns the row of the element in the
//		the given column)
//////////////////////////////////////////////////////////////////////
template<class TYPE>
int CMatrixBase<TYPE>::FindPivotRow(int nDiag)
{
	int nBestRow = nDiag;	// stores the current best row

	if (fabs((*this)[nDiag][nDiag]) < MAX_TO_PIVOT) 
	{
		int nRow;	// for index
    	for (nRow = nDiag + 1; nRow < GetRows(); nRow++)
		{
			if (fabs((*this)[nDiag][nRow]) > fabs((*this)[nDiag][nBestRow]))
			{
    			nBestRow = nRow;
			}
		}
	}

	return nBestRow;

}	// CMatrixBase<TYPE>::FindPivotRow


//////////////////////////////////////////////////////////////////////
// CMatrixBase<TYPE>::FindPivotElem
//
// finds the pivot element, returning the row and column
//////////////////////////////////////////////////////////////////////
template<class TYPE>
void CMatrixBase<TYPE>::FindPivotElem(int nDiag, 
		int *pBestRow, int *pBestCol)
{
	(*pBestCol) = nDiag;
	(*pBestRow) = nDiag;

	if (fabs((*this)[nDiag][nDiag]) < MAX_TO_PIVOT) 
	{
		for (int nCol = nDiag; nCol < GetCols(); nCol++)
		{
    		for (int nRow = nDiag; nRow < GetRows(); nRow++)
			{
				if (fabs((*this)[nCol][nRow]) 
					> fabs((*this)[*pBestCol][*pBestRow]))
				{
					(*pBestCol) = nCol;
					(*pBestRow) = nRow;
				}
			}
		}
	}

}	// CMatrixBase<TYPE>::FindPivotElem


//////////////////////////////////////////////////////////////////////
// CMatrixBase<TYPE>::SVD
//
// computes the singular-valued decomposition of this matrix,
//		leaving U in the matrix and returning the singular values
//		in w and v (not v^T)
//////////////////////////////////////////////////////////////////////
template<class TYPE>
BOOL CMatrixBase<TYPE>::SVD(CVectorBase<TYPE>& w, CMatrixBase<TYPE>& v)
{
	// stored reduction vector
	CVectorN<TYPE> rv1(GetCols());

	// perform the householder reduction
	TYPE anorm = Householder(w, rv1);

	// accumulation of right-hand transformations
	AccumulateRH(v, rv1);

	// accumulation of left-hand transformations
	AccumulateLH(w);

	int flag, i, its, j, jj, k, l, nm;
	TYPE c, f, g, h, s, x, y, z;

	// Diagonalization of the bidiagonal form
	for (k = GetCols()-1; k >= 0; k--)
	{
		for (its = 1; its < MAX_ITER; its++)
		{
			flag = 1;
			for (l = k; l >= 0; l--)
			{
				nm = l-1;
				if ((TYPE)(fabs(rv1[l]) + anorm) == anorm)
				{
					flag = 0;
					break;
				}
				if ((TYPE)(fabs(w[nm]) + anorm) == anorm)
					break;
			}
			if (flag)
			{
				c = 0.0;
				s = 1.0;
				for (i = l; i <= k; i++)
				{
					f = s * rv1[i];
					rv1[i] = c * rv1[i];
					if ((TYPE)(fabs(f) + anorm) == anorm)
						break;
					g = w[i];
					h = pythag(f,g);
					w[i] = h;
					h = 1.0 / h;
					c = g * h;
					s = -f * h;
					for (j = 0; j < GetRows(); j++)
					{
						y = (*this)[nm][j];
						z = (*this)[i][j];
						(*this)[nm][j] = y * c + z * s;
						(*this)[i][j] = z * c - y * s;
					}	// for
				}	// for
			}	// if

			z = w[k];

			// test for convergence
			if (l == k)
			{
				if (z < 0.0)
				{
					// singular value is made non-negative
					w[k] = -z;
					for (j = 0; j < GetCols(); j++)
						v[k][j] = -v[k][j];
				}
				break;
			}	// if

			x = w[l];	// shift from bottom 2x2 minor
			nm = k - 1;
			y = w[nm];
			g = rv1[nm];
			h = rv1[k];
			f = ((y - z) * (y + z) + (g - h) * (g + h)) 
				/ (2.0 * h * y);
			g = pythag<TYPE>(f, 1.0);
			f = ((x - z) * (x + z) + h * ((y / (f + SIGN(g,f))) - h))
				/ x;
			c = s = 1.0;		// next QR transformation:
			for (j = 0; j <= nm; j++)
			{
				i = j + 1;
				g = rv1[i];
				y = w[i];
				h = s * g;
				g = c * g;
				z = pythag<TYPE>(f, h);
				rv1[j] = z;
				c = f / z;
				s = h / z;
				f = x * c + g * s;
				g = g * c - x * s;
				h = y * s;
				y *= c;
				for (jj = 0; jj < GetCols(); jj++)
				{
					x = v[j][jj];
					z = v[i][jj];
					v[j][jj] = x * c + z * s;
					v[i][jj] = z * c - x * s;
				}
				z = pythag(f,h);
				w[j] = z;
				if (z != 0.0)
				{
					z = 1.0/z;
					c = f * z;
					s = h * z;
				}
				f = c * g + s * y;
				x = c * y - s * g;
				for (jj = 0; jj < GetRows(); jj++)
				{
					y = (*this)[j][jj];
					z = (*this)[i][jj];
					(*this)[j][jj] = y * c + z * s;
					(*this)[i][jj] = z * c - y * s;
				}
			}	// for
			rv1[l] = 0.0;
			rv1[k] = f;
			w[k] = x;
		}

		if (its == MAX_ITER)
		{
			TRACE("SVD: no convergence in %i steps\n", MAX_ITER);
			return FALSE;
		}
	}

	return TRUE;

}	// CMatrixBase<TYPE>::SVD


//////////////////////////////////////////////////////////////////////
// function CMatrixBase<TYPE>::Householder
//
// helper function for SVD to perform Householder decomposition
//////////////////////////////////////////////////////////////////////
template<class TYPE>
TYPE CMatrixBase<TYPE>::Householder(CVectorBase<TYPE>& w, 
									CVectorBase<TYPE>& rv1)
{
	int i, j, k, l;
	TYPE f, g, h, s, scale, anorm;

	g = scale = anorm = 0.0;

	// Householder reduction to bidiagonal form
	for (i = 0; i < GetCols(); i++)
	{
		l = i + 1;
		rv1[i] = scale * g;
		g = s = scale = 0.0;
		if (i <= GetRows()-1)
		{
			for (k = i; k < GetRows(); k++)
			{
				scale += fabs((*this)[i][k]);
			}
			if (scale != 0.0)
			{
				for (k = i; k < GetRows(); k++)
				{
					(*this)[i][k] /= scale;
					s += (*this)[i][k] * (*this)[i][k];
				}
				f = (*this)[i][i];
				g = -SIGN(sqrt(s),f);
				h = f * g - s;
				(*this)[i][i] = f - g;
				for (j = l; j < GetCols(); j++)
				{
					for (s = 0.0, k = i; k < GetRows(); k++)
					{
						s += (*this)[i][k] * (*this)[j][k];
					}
					f = s / h;
					for (k = i; k < GetRows(); k++)
					{
						(*this)[j][k] += f * (*this)[i][k];
					}
				}	// for
				for (k = i; k < GetRows(); k++)
				{
					(*this)[i][k] *= scale;
				}
			}	// if
		}	// if
		w[i] = scale * g;
		g = s = scale = 0.0;
		if (i < GetRows() && i != GetCols()-1)
		{
			for (k = l; k < GetCols(); k++)
			{
				scale += fabs((*this)[k][i]);
			}
			if (scale != 0.0)
			{
				for (k = l; k < GetCols(); k++)
				{
					(*this)[k][i] /= scale;
					s += (*this)[k][i] * (*this)[k][i];
				}
				f = (*this)[l][i];
				g = -SIGN(sqrt(s), f);
				h = f * g - s;
				(*this)[l][i] = f - g;
				for (k = l; k < GetCols(); k++)
				{
					rv1[k] = (*this)[k][i] / h;
				}
				for (j = l; j < GetRows(); j++)
				{
					for (s = 0.0, k = l; k < GetCols(); k++)
					{
						s += (*this)[k][j] * (*this)[k][i];
					}
					for (k = l; k < GetCols(); k++)
					{
						(*this)[k][j] += s * rv1[k];
					}
				}
				for (k = l; k < GetCols(); k++)
				{
					(*this)[k][i] *= scale;
				}
			}	// if
		}	// if

		anorm = __max(anorm, (fabs(w[i]) + fabs(rv1[i])));

	}	// for

	return anorm;

}	// CMatrixBase<TYPE>::Householder


//////////////////////////////////////////////////////////////////////
// function CMatrixBase<TYPE>::AccumulateRH
//
// helper function for SVD to accumulate right-hand products
//////////////////////////////////////////////////////////////////////
template<class TYPE>
void CMatrixBase<TYPE>::AccumulateRH(CMatrixBase<TYPE>& v, 
									 const CVectorBase<TYPE>& rv1)
{
	int i, j, k, l;
	TYPE g, s;

	// Accumulation of right-hand transformations
	for (i = GetCols()-1; i >= 0; i--)
	{
		if (i < GetCols()-1)
		{
			if (g != 0.0)
			{
				for (j = l; j < GetCols(); j++)
					v[i][j] = ((*this)[j][i] / (*this)[l][i]) / g;
				for (j = l; j < GetCols(); j++)
				{
					for (s = 0.0, k = l; k < GetCols(); k++)
						s += (*this)[k][i] * v[j][k];
					for (k = l; k < GetCols(); k++)
						v[j][k] += s * v[i][k];
				}	// for
			}	// if
			for (j = l; j < GetCols(); j++)
				v[j][i] = v[i][j] = 0.0;
		}	// if
		v[i][i] = 1.0;
		g = rv1[i];
		l = i;
	}

}	// CMatrixBase<TYPE>::AccumulateRH


//////////////////////////////////////////////////////////////////////
// CMatrixBase<TYPE>::AccumulateLH
//
// helper function for SVD to accumulate left-hand products
//////////////////////////////////////////////////////////////////////
template<class TYPE>
void CMatrixBase<TYPE>::AccumulateLH(CVectorBase<TYPE>& w)
{
	int i, j, k, l;
	TYPE f, g, s;

	for (i = __min(GetRows()-1, GetCols()-1); i >= 0; i--)
	{
		l = i + 1;
		g = w[i];
		for (j = l; j < GetCols(); j++)
		{
			(*this)[j][i] = 0.0;
		}
		if (g != 0.0)
		{
			g = 1.0 / g;
			for (j = l; j < GetCols(); j++)
			{
				for (s = 0.0, k = l; k < GetRows(); k++)
				{
					s += (*this)[i][k] * (*this)[j][k];
				}
				f = (s / (*this)[i][i]) * g;
				for (k = i; k < GetRows(); k++)
				{
					(*this)[j][k] += f * (*this)[i][k];
				}
			}	// for
			for (j = i; j < GetRows(); j++)
			{
				(*this)[i][j] *= g;
			}
		}
		else
		{
			for (j = i; j < GetRows(); j++)
			{
				(*this)[i][j] = 0.0;
			}
		}
		++(*this)[i][i];
	}

}	// CMatrixBase<TYPE>::AccumulateLH


//////////////////////////////////////////////////////////////////
// CMatrixBase<TYPE>::Pseudoinvert
// 
// in-place Moore-Penrose pseudoinversion
//////////////////////////////////////////////////////////////////
template<class TYPE>
BOOL CMatrixBase<TYPE>::Pseudoinvert()
{
	CVectorN<TYPE> w(GetCols());
	CMatrixBase<TYPE> v(GetCols(), GetCols());
	if (!SVD(w, v))
	{
		return FALSE;
	}

	// using the formula (A+)^T = U * {1/w} * V^T

	// form the S matrix (S^T * S)^-1 * S^T
	CMatrixBase<TYPE> s(GetCols(), GetCols());
	for (int nAt = 0; nAt < GetCols(); nAt++)
	{
		s[nAt][nAt] = (w[nAt] > 1e-8) ? 1.0 / w[nAt] : 0.0;
	}
	(*this) *= s;

	// multiply by V^T
	v.Transpose();
	(*this) *= v;

	// and transpose the result to finish with A+
	Transpose();

	return TRUE;

}	// CMatrixBase<TYPE>::Pseudoinvert


#endif // define _MATRIXBASE_INL_
