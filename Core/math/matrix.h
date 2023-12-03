#ifndef matrix_mathH
#define matrix_mathH

/******************************************************************

Collection of template matrix-math-functions.

******************************************************************/



#define MATRIX_LOOP(w,h)				for (index_t x = 0; x < (w); x++) for (index_t y = 0; y < (h); y++)

#include "../global_string.h"
#include "vector.h"

namespace DeltaWorks
{
	namespace Math
	{
		template <typename T=float>
			struct TMatrix2
			{
				typedef T			Type;

				union
				{
					struct
					{
						TVec2<T>	x,
									y;
					};
					T				v[4];	//should be fine even with alignment and all
					TVec2<T>		axis[2];
				};
			};


		template <typename T=float>
			struct TMatrix3
			{
				typedef T			Type;

				union
				{
					struct
					{
						TVec3<T>	x,
									y,
									z;
					};
					T				v[9];	//should be fine even with alignment and all
					TVec3<T>		axis[3];
				};
			};

		template <typename T=float>
			struct TMatrix4
			{
				typedef T				Type;

				union
				{
					struct
					{
						TVec4<T>	x,
									y,
									z,
									w;
					};
					T				v[16];	//should be fine even with alignment and all
					TVec4<T>		axis[4];
				};
			};

		template <typename T, count_t Width, count_t Height>
			struct TMatrix
			{
				typedef T				Type;
				static const count_t	Columns=Width,
										Rows=Height;
				union
				{
					TVec<T,Height>		axis[Width];
					T					v[Width*Height];
					TVec<T,Width*Height>asVector;
				};
			};

		template <typename T>
			struct TMatrix<T,4,4>:public TMatrix4<T>
			{};
		template <typename T>
			struct TMatrix<T,3,3>:public TMatrix3<T>
			{};
		template <typename T>
			struct TMatrix<T,2,2>:public TMatrix2<T>
			{};




		template <typename T>
			inline void Serialize(IWriteStream&s, const TMatrix2<T>&v)
			{
				using Serialization::Serialize;
				Serialize(s,v.x);
				Serialize(s,v.y);
			}
		template <typename T>
			inline void Deserialize(IReadStream&s, TMatrix2<T>&v)
			{
				using Serialization::Deserialize;
				Deserialize(s,v.x);
				Deserialize(s,v.y);
			}


		template <typename T>
			inline void Serialize(IWriteStream&s, const TMatrix3<T>&v)
			{
				using Serialization::Serialize;
				Serialize(s,v.x);
				Serialize(s,v.y);
				Serialize(s,v.z);
			}
		template <typename T>
			inline void Deserialize(IReadStream&s, TMatrix3<T>&v)
			{
				using Serialization::Deserialize;
				Deserialize(s,v.x);
				Deserialize(s,v.y);
				Deserialize(s,v.z);
			}

		template <typename T>
			inline void Serialize(IWriteStream&s, const TMatrix4<T>&v)
			{
				using Serialization::Serialize;
				Serialize(s,v.x);
				Serialize(s,v.y);
				Serialize(s,v.z);
				Serialize(s,v.w);
			}
		template <typename T>
			inline void Deserialize(IReadStream&s, TMatrix4<T>&v)
			{
				using Serialization::Deserialize;
				Deserialize(s,v.x);
				Deserialize(s,v.y);
				Deserialize(s,v.z);
				Deserialize(s,v.w);
			}

		#define MFUNCM(_return_type_)	template <class T0, class T1, class T2, count_t MHeight, count_t MWidth, count_t NWidth> MF_DECLARE(_return_type_)
		#define MFUNCD(_return_type_)	template <class C, count_t Rows, count_t Cols> MF_DECLARE(_return_type_)
		#define MFUNCC(_return_type_)	template <class C, count_t Rows, count_t Cols, index_t Col0, index_t Col1> MF_DECLARE(_return_type_)
		#define MFUNCR(_return_type_)	template <class C, count_t Rows, count_t Cols, index_t Row0, index_t Row1> MF_DECLARE(_return_type_)
	
		#if 0
			namespace ByReference
			{
				MFUNC2	(void)		__rotationMatrix(const C0&angle, C1*out, count_t dimension);									//!< Generates a 2x2 rotation-matrix as the top left 2x2 section of the target matrix. Columns and rows larger than 2 are set to identity. \param angle Angle the rotation matrix should rotate(-180 - +180) \param out Out matrix (quadratic) \param dimension Effective target matrix dimension (must be at least 2)
				MFUNC2V (void)		__rotationMatrix(const C0&angle, C1*out);														//!< Generates a 2x2 rotation-matrix as the top left 2x2 section of the target matrix. Columns and rows larger than 2 are set to identity. The matrix target dimension is specified in the last template parameter and must be at least 2 \param angle Angle the rotation matrix should rotate when multiplied(-180 - +180) \param out Out matrix (quadratic)
				MFUNC3	(void)		__rotationMatrix(const C0&angle, const C1 axis[3], C2 out[3*3]);								//!< Generates a 3x3 rotation matrix to rotate about an arbitrary axis and stores the result in \b out \param angle Angle this rotation matrix should rotate when multiplied (-180 - +180) \param axis Rotation axis (must be normalized) \param out Target 3x3 matrix
				MFUNC5	(void)		__rotationMatrix(const C0&angle, const C1&ax, const C2&ay, const C3&az, C4 out[3*3]);			//!< Generates a 3x3 rotation matrix to rotate about an arbitrary axis and stores the result in \b out. The component-wise defined axis can be of any length greater 0  \param angle Angle this rotation matrix should rotate when multiplied (-180 - +180) \param ax X-component of the rotation axis \param ay Y-component of the rotation axis \param az Z-component of the rotation axis \param out Target 3x3 matrix
				MFUNC2	(void)		__scaleSystem(C0 m[4*4], const C1&scalar);																			//!< \deprecated {system matrices should not be scaled}
			}
	

			MFUNC4	(void)		__buildSystem(const C0 base[3],const C1 x[3],const C2 y[3], C3 out[4*4]);						//!< Creates a system matrix based on base, x and y axes. The x axis is primary and used exactly as it is (though normalized). The y axis is normalized and used as close as it is orthogonal to x. @param base 3d vector to the base of the system @param x X axis of the system (any length > 0) @param y Y axis of the system (any length > 0) @param out Out matrix for the finished 4x4 system matrix
			#define				_c9							__copy9
			#define				_c11						__copy11															//note: might as well use the _copyV-functions provided by vector.h (same speed)
			#define				_c12						__copy12
			#define				_c16						__copy16
			MFUNC2	(void)		__copy9(const C0 source[9], C1 destination[9]);													//!< Copies a 3x3 matrix from \b source to \b destination
			MFUNC2	(void)		__copy11(const C0 source[11], C1 destination[11]);												//!< Copies the first 11 elements (i.e. the orientation of a 4x4 system matrix) from \b source to \b destination
			MFUNC2	(void)		__copy12(const C0 source[12], C1 destination[12]);												//!< Copies the first 12 elements (i.e. a reduced system matrix) from \b source to \b destination
			MFUNC2	(void)		__copy16(const C0 source[16], C1 destination[16]);												//!< Copies a 4x4 matrix from \b source to \b destination
			MFUNC	(C)			__determinant(const C*m, count_t rows, count_t cols);											//!< Calculates the determinant of \b m \param m Matrix to calculate the determinant of \param rows Number of rows in \b m \param cols Number of columns in \b m
			MFUNC	(void)		__normalizeSystem(C*m);																			//!< Normalizes first 3 vectors of the 4x4 system \b m
			MFUNC2	(void)		__extractUpper(const C0*m, count_t m_height, C1*out, count_t dimension);						//!< Extracts the upper triangular matrix of \b m \param m_height Number of rows in \b m \param m Matrix to extract the upper triangular matrix of
			MFUNC2	(void)		__extractUpper(const C0*m, C1*out, count_t dimension);											//!< Extracts the upper triangular matrix of \b m \param m Quadratic matrix to extract the upper triangular matrix of
			MFUNC2	(void)		__extractLower(const C0*m, count_t m_height, C1*out, count_t dimension);						//!< Extracts the lower triangular matrix of \b m \param m_height Number of rows in \b m \param m Matrix to extract the lower triangular matrix of
			MFUNC2	(void)		__extractLower(const C0*m, C1*out, count_t dimension);											//!< Extracts the lower triangular matrix of \b m \param m Quadratic matrix to extract the lower triangular matrix of
			MFUNC2	(void)		__extractDiagonal(const C0*m, count_t m_height, C1*out, count_t dimension);					//!< Extracts the diagonal matrix of \b m \param m_height Number of rows in \b m \param m Matrix to extract the triangular matrix of
			MFUNC2	(void)		__extractDiagonal(const C0*m, C1*out, count_t dimension);										//!< Extracts the diagonal matrix of \b m \param m Quadratic matrix to extract the triangular matrix of
			MFUNC2	(bool)		__invertGauss(const C0*m, C1*out, count_t dimension);											//!< Inverts \b m using the plain Gauss algorithm. This function occassionally fails since it does not swap rows where needed \param m Quadratic matrix to invert \param out Matrix target for the inverted matrix \param dimension Number of columns/rows in \b m and \b out \return true if the matrix could be inverted, false otherwise
			MFUNC2	(bool)		__invertMatrix3(const C0 m[3*3], C1 out[3*3]);													//!< Selective version of __invertGauss() for 3x3 matrices. This function successfully inverts most invertible matrices \param m 3x3 matrix to invert \param out Out target for the inverted matrix \return true if the matrix could be inverted, false otherwise
			MFUNC2	(bool)		__invertMatrix4(const C0 m[4*4], C1 out[4*4]);													//!< Selective version of __invertGauss() for 4x4 matrices. This function successfully inverts most invertible matrices \param m 4x4 matrix to invert \param out Out target for the inverted matrix \return true if the matrix could be inverted, false otherwise
			MFUNC2	(void)		__invertSystem(const C0 m[4*4], C1 out[4*4]);													//!< Inverts the system matrix \b m. Inverting system matrices can be done much faster than inverting a general matrix. A system matrix consist of three orthogonal normalized direction vectors and one position vector. The bottom most row is supposed to be 0, 0, 0, 1.
			MFUNC2	(void)		__invertReduced(const C0 m[3*4], C1 out[3*3]);													//!< Inverts the reduced system matrix \b m. A reduced system matrix is identical to a system matrix except that it does not provide the bottom most row, effectivly reducing the matrix to a 3x4 matrix (3 rows, 4 columns).
			MFUNC	(bool)		__isIdentity(const C*matrix, count_t dimension);												//!< Tests whether or not the given \b dimension x \b dimension -matrix is identical or very close to an identity-matrix
			MFUNCV	(bool)		__isIdentity(const C*matrix);																	//!< Tests whether or not the given NxN-matrix is identical or very close to an identity-matrix. The matrix dimension is specified in the last template parameter 
			MFUNC	(bool)		__isIdentity3(const C matrix[3*3]);																//!< Tests whether or not the given 3x3-matrix is identical or very close to an identity-matrix
			MFUNC	(bool)		__isIdentity4(const C matrix[4*4]);																//!< Tests whether or not the given 4x4-matrix is identical or very close to an identity-matrix
			MFUNC17 (void)		__m4(C0 m[4*4], const C1&v11, const C2&v12, const C3&v13, const C4&v14,							//!< Defines a matrix
												const C5&v21, const C6&v22, const C7&v23, const C8&v24,
												const C9&v31, const C10&v32, const C11&v33, const C12&v34,
												const C13&v41, const C14&v42, const C15&v43, const C16&v44);
			MFUNC3	(void)		__makeAxisSystem(const C0 position[3],const C1 axis[3], BYTE direction,C2 out[4*4]);			//!< Creates a system matrix from a point and an axis and stores the result in \b out. \param position Base position of the new system \param axis Orientation axis \param direction Coordinate vector of the final matrix that should point along the specified axis (0-2). For instance passing 1 as \b direction effectivly causes the y axis of the final matrix to point along the specified axis. \param out Out 4x4 matrix for the constructed system matrix.
			MFUNC2	(void)		__makeAxisSystem(const C0 axis[3], BYTE direction,C1 out[3*3]);									//!< Creates an orientation matrix from the specified axis and stores the result in \b out. \param axis Orientation axis \param direction Coordinate vector of the final matrix that should point along the specified axis (0-2). For instance passing 1 as \b direction effectivly causes the y axis of the final matrix to point along the specified axis. \param out Out 3x3 matrix for the constructed orientation matrix.
			MFUNC3	(void)		__multiply(const C0*m, const C1*n, C2*out, count_t height, count_t mwidth, count_t nwidth);	//!< Multiples \b m and \b n and stores the result in \b out. \param m First matrix \param n Second matrix \param out Result matrix \param height Number of rows of the first matrix \param mwidth Number of columns of the first matrix and the number of rows of the second matrix \param nwidth Number of columns in the second matrix
			MFUNC3	(void)		__mult331(const C0 m[3*3], const C1 n[3], C2 out[3]);											//!< Further optimized version of __multiply<*,*,*,3,3,1>()
			MFUNC2	(void)		__mult331(const C0 m[3*3], C1 inout[3]);														//!< @overload
			MFUNC3	(void)		__mult3(const C0 m[3*3], const C1 n[3*3], C2 out[3*3]);											//!< Further optimized version of __multiply<*,*,*,3,3,3>()
			MFUNC3	(void)		__mult441(const C0 m[4*4], const C1 n[4], C2 out[4]);											//!< Further optimized version of __multiply<*,*,*,4,4,1>()
			MFUNC3	(void)		__mult4(const C0 m[4*4], const C1 n[4*4], C2 out[4*4]);											//!< Further optimized version of __multiply<*,*,*,4,4,4>()
			MFUNC3	(void)		__multiplyReduced(const C0 sys0[3*4], const C1 sys1[3*4], C2 out[3*4]);							//!< Transforms the reduced system \b sys1 using the reduced system in \b sys0 and stores the resulting reduced system in \b out \param sys0 First reduced 3x4 system matrix \param sys1 Second reduced 3x4 system matrix \param out Target out reduced system matrix
			MFUNC3	(void)		__rotate(const C0 system[4*4], const C1 vec[3], C2 out[3]);										//!< Rotates \b vec using the system matrix \b system and stores the result in \b out. The translation aspect of \b system is ignored \param system System matrix used to rotate \b vec \param vec Vector to rotate \param out Target vector to store the resulting vector in.
			MFUNC2	(void)		__rotate(const C0 system[4*4], C1 vec[3]);														//!< Rotates \b vec using the system matrix \b system and stores the result back in \b vec. The translation aspect of \b system is ignored \param system System matrix used to rotate \b vec \param vec Vector to rotate
			MFUNC3	(void)		__rotateReduced(const C0 system[3*4], const C1 vec[3], C2 out[3]);								//!< Identical to __rotate() for reduced system matrices
			MFUNC2	(void)		__rotationMatrix(C0 angle, C1*out, count_t dimension);									//!< Generates a 2x2 rotation-matrix as the top left 2x2 section of the target matrix. Columns and rows larger than 2 are set to identity. \param angle Angle the rotation matrix should rotate(-180 - +180) \param out Out matrix (quadratic) \param dimension Effective target matrix dimension (must be at least 2)
			MFUNC2V (void)		__rotationMatrix(C0 angle, C1*out);														//!< Generates a 2x2 rotation-matrix as the top left 2x2 section of the target matrix. Columns and rows larger than 2 are set to identity. The matrix target dimension is specified in the last template parameter and must be at least 2 \param angle Angle the rotation matrix should rotate when multiplied(-180 - +180) \param out Out matrix (quadratic)
			MFUNC3	(void)		__rotationMatrix(C0 angle, const C1 axis[3], C2 out[3*3]);								//!< Generates a 3x3 rotation matrix to rotate about an arbitrary axis and stores the result in \b out \param angle Angle this rotation matrix should rotate when multiplied (-180 - +180) \param axis Rotation axis (must be normalized) \param out Target 3x3 matrix
			MFUNC5	(void)		__rotationMatrix(C0 angle, C1 ax, C2 ay, C3 az, C4 out[3*3]);			//!< Generates a 3x3 rotation matrix to rotate about an arbitrary axis and stores the result in \b out. The component-wise defined axis can be of any length greater 0  \param angle Angle this rotation matrix should rotate when multiplied (-180 - +180) \param ax X-component of the rotation axis \param ay Y-component of the rotation axis \param az Z-component of the rotation axis \param out Target 3x3 matrix
			MFUNC2	(void)		__rowMult(C0*m, count_t width, count_t height, index_t row, C1 value);						//!< Scales one matrix row by \b value \param m Root matrix \param width Number of columns of the root matrix \param Number of rows of the root matrix \param row Index of the row that should be scaled by \b value \param value Value to multiply/scale the specified row
			MFUNC2	(void)		__scaleSystem(C0 m[4*4], C1 scalar);																			//!< \deprecated {system matrices should not be scaled}
			MFUNC3	(void)		__solveGaussSeidel(const C0*m, count_t m_height, const C1*result, C2*out, count_t dimension, unsigned accuracy);	//solve system
			MFUNC3	(void)		__solveGaussSeidel(const C0*m, const C1*result, C2*out, count_t dimension, unsigned accuracy);						//solve system
			MFUNC3	(void)		__solveJacobi(const C0*m, count_t m_height, const C1*result, C2*out, count_t dimension, unsigned accuracy);		//solve system
			MFUNC3	(void)		__solveJacobi(const C0*m, const C1*result, C2*out, count_t dimension, unsigned accuracy);							//solve system
			MFUNC3	(void)		__transform(const C0 system[4*4], const C1 point[3], C2 out[3]);								//!< Transforms (rotates and translates) \b point using the system matrix \b system and writes the resulting vector to \b out \param system System matrix to use for the transformation \param point Point to transform \param out Transformed point
			MFUNC2	(void)		__transform(const C0 system[4*4], C1 inout_point[3]);											//!< Transforms (rotates and translates) \b inout_point using the system matrix \b system and writes the resulting vector to \b inout_point \param system System matrix to use for the transformation \param inout_point Point to transform
			MFUNC3	(void)		__transformSystem(const C0 system0[4*4], const C1 system1[4*4], C2 out[4*4]);					//!< Transforms (rotates and translates) the system matrix \b system1 using the system matrix \b system0 and writes the resulting system matrix to \b out. The transformation (product) of two system matrices is again a system matrix \param system0 System matrix to use for the transformation \param system1 System matrix to transform \param out Resulting system matrix
			MFUNC2	(void)		__transformSystem(const C0 system0[4*4], C1 inout_system1[4*4]);								//!< Transforms (rotates and translates) the system matrix \b inout_system1 using the system matrix \b system0 and writes the resulting system matrix to \b inout_system1. The transformation (product) of two system matrices is again a system matrix \param system0 System matrix to use for the transformation \param inout_system1 System matrix to transform and write the result to
			MFUNC3	(void)		__transformReduced(const C0 sys[3*4], const C1 point[3], C2 out[3]);							//!< Identical to __transformSystem() except for reduced system matrices
			MFUNC2V (void)		__transpose(const C0*m, C1*out);																//!< Calculates the matrix transpose (reflection along the diagonal axis) of \b m and stores the result in \b out. The dimension of \b m is specified by the last template parameter \param m Quadratic matrix to calculate the transpose of \param out Out matrix transpose
			MFUNC2	(void)		__transpose(const C0*m, C1*out, count_t dimensions);											//!< Calculates the matrix transpose (reflection along the diagonal axis) of \b m and stores the result in \b out \param m Quadratic matrix to calculate the transpose of \param out Out matrix transpose \param dimensions Columns/rows of \b m and \b out
			MFUNC2	(void)		__transpose3(const C0 m[3*3], C1 out[3*3]);														//!< Calculates the matrix transpose reflection along the diagonal axis) of the 3x3 matrix \b m and stores the result in \b out \param m Quadratic matrix to calculate the transpose of \param out Out matrix transpose
		#endif /*0*/

		template <typename T=float>
		class Matrix
		{
		public:
		static	const	TMatrix2<T>	eye2;
		static	const	TMatrix3<T>	eye3;
		static	const	TMatrix4<T>	eye4;
		};

	


		/**
		\brief Assembler style matrix operators

		Though these functions may look primitive, they are highly optimized. <br />
		\b Important: Unless specified other pointers to matrices or vectors passed to these functions must be distinct (not passed to the same function as different parameters at the same time).
		*/
		namespace Mat
		{
			MFUNC(void)			Eye(TMatrix3<C>&matrix)		{matrix = Matrix<C>::eye3;}
			MFUNC(void)			Eye(TMatrix4<C>&matrix)		{matrix = Matrix<C>::eye4;}

			MFUNC3(void)		Mult(const TMatrix2<C0>&m0, const TMatrix2<C1>&m1, TMatrix2<C2>&result);
			MFUNC3(void)		Mult(const TMatrix2<C0>&m, const TVec2<C1>&v, TVec2<C2>&result);
			MFUNC2(void)		Mult(const TMatrix2<C0>&m, TVec2<C1>&v);
			MFUNC3(void)		Mult(const TMatrix3<C0>&m0, const TMatrix3<C1>&m1, TMatrix3<C2>&result);
			MFUNC3(void)		Mult(const TMatrix3<C0>&m, const TVec3<C1>&v, TVec3<C2>&result);
			MFUNC2(void)		Mult(const TMatrix3<C0>&m, TVec3<C1>&v);
			MFUNC3(void)		Mult(const TMatrix4<C0>&m0, const TMatrix4<C1>&m1, TMatrix4<C2>&result);
			MFUNC3(void)		Mult(const TMatrix4<C0>&m, const TVec4<C1>&v, TVec4<C2>&result);
			MFUNC2(void)		Mult(const TMatrix4<C0>&m, TVec4<C1>&v);
			MFUNCM(void)		Mult(const TMatrix<T0,MWidth,MHeight>&m, const TMatrix<T1,NWidth,MWidth>&n, TMatrix<T2,NWidth,MHeight>&outResult);
			template <typename T0, typename T1, typename T2, count_t Rows, count_t Cols>
				void			Mult(const TMatrix<T0,Cols,Rows>&m, const TVec<T1,Cols>&v, TVec<T2,Rows>&outResult);

			/**
			Creates a 2d rotation matrix
			@param angle Rotation angle in [-180,180]
			@param[out] result Resulting rotation matrix
			*/
			MFUNC2(void)		BuildRotationMatrix(C0 angle, TMatrix2<C1>&result);
			/**
			Creates a 3d rotation matrix, rotating around the X axis
			@param angle Rotation angle in [-180,180]
			@param[out] result Resulting rotation matrix
			*/
			MFUNC2(void)		FillXRotationMatrix(C0 angle, TMatrix3<C1>&result);
			/**
			Creates a 3d rotation matrix, rotating around the Y axis
			@param angle Rotation angle in [-180,180]
			@param[out] result Resulting rotation matrix
			*/
			MFUNC2(void)		FillYRotationMatrix(C0 angle, TMatrix3<C1>&result);
			/**
			Creates a 3d rotation matrix, rotating around the Z axis
			@param angle Rotation angle in [-180,180]
			@param[out] result Resulting rotation matrix
			*/
			MFUNC2(void)		FillZRotationMatrix(C0 angle, TMatrix3<C1>&result);
			/**
			Generates a 3x3 rotation matrix to rotate about an arbitrary axis and stores the result in \b out.
			The component-wise defined axis can be of any length greater 0 
			@param angle Angle this rotation matrix should rotate when multiplied (-180 - +180)
			@param axis Normalized rotation axis
			@param[out] Target 3x3 matrix
			*/
			MFUNC3(void)		BuildRotationMatrix(C0 angle,const TVec3<C1>&axis,TMatrix3<C2>&result);
			/**
			Generates a 3x3 rotation matrix to rotate about an arbitrary axis and stores the result in \b out.
			The component-wise defined axis can be of any length greater 0 
			\param angle Angle this rotation matrix should rotate when multiplied (-180 - +180)
			\param ax X-component of the rotation axis
			\param ay Y-component of the rotation axis
			\param az Z-component of the rotation axis
			\param[out] Target 3x3 matrix
			*/
			MFUNC5(void)		BuildRotationMatrix(C0 angle, C1 ax, C2 ay, C3 az, TMatrix3<C4>&result);			

			MFUNC2(void)		transpose(const TMatrix4<C0>&m, TMatrix4<C1>&result);
			MFUNC(void)			transpose(TMatrix4<C>&m);
			MFUNC(TMatrix4<C>)	transposed(const TMatrix4<C>&m);
			MFUNC2(void)		transpose(const TMatrix3<C0>&m, TMatrix3<C1>&result);
			MFUNC(void)			transpose(TMatrix3<C>&m);
			MFUNC(TMatrix3<C>)	transposed(const TMatrix3<C>&m);
			MFUNC2(void)		transpose(const TMatrix2<C0>&m, TMatrix2<C1>&result);
			MFUNC(void)			transpose(TMatrix2<C>&m);
			MFUNC(TMatrix2<C>)	transposed(const TMatrix2<C>&m);

			MFUNC2(void)		cast(const TMatrix4<C0>&from, TMatrix4<C1>&to);
			MFUNC2(void)		copy(const TMatrix4<C0>&from, TMatrix4<C1>&to)		/**@copydoc cast()*/ {cast(from,to);}

			MFUNC3(void)		rotate(const TMatrix4<C0>&system, const TVec3<C1>&vec, TVec3<C2>&result);
			MFUNC2(void)		rotate(const TMatrix4<C0>&system, TVec3<C1>&vec);
			MFUNC3(void)		Transform(const TMatrix4<C0>&system, const TVec3<C1>&vec, TVec3<C2>&result);
			MFUNC2(void)		Transform(const TMatrix4<C0>&system, TVec3<C1>&vec);
			MFUNC3(void)		rotate(const TMatrix4<C0>*system, const TVec3<C1>&vec, TVec3<C2>&result);
			MFUNC2(void)		rotate(const TMatrix4<C0>*system, TVec3<C1>&vec);
			MFUNC3(void)		Transform(const TMatrix4<C0>*system, const TVec3<C1>&vec, TVec3<C2>&result);
			MFUNC2(void)		Transform(const TMatrix4<C0>*system, TVec3<C1>&vec);

			MFUNC2(void)		Rotate(const TMatrix3<C0>& system, TVec2<C1>& vec);
			MFUNC3(void)		Rotate(const TMatrix3<C0>& system, const TVec2<C1>& vec, TVec2<C2>& result);

			MFUNC4(void)		reflectSystemN(const TVec3<C0>&base, const TVec3<C1>&normal, const TMatrix4<C2>&matrix, TMatrix4<C3>&result);
			MFUNC4(void)		reflectMatrixN(const TVec3<C0>&base, const TVec3<C1>&normal, const TMatrix4<C2>&matrix, TMatrix4<C3>&result);

			MFUNC3(void)		transformSystem(const TMatrix4<C0>&system0, const TMatrix4<C1>&system1, TMatrix4<C2>&result);
			MFUNC2(void)		transformSystem(const TMatrix4<C0>&system0, TMatrix4<C1>&system1);

			MFUNC2(void)		invertSystem(const TMatrix4<C0>&sys, TMatrix4<C1>&result);
			MFUNC2(bool)		invert(const TMatrix4<C0>&sys, TMatrix4<C1>&result);
			MFUNC2(bool)		invert(const TMatrix3<C0>&sys, TMatrix3<C1>&result);
			MFUNC3(void)		makeAxisSystem(const TVec3<C0>&position,const TVec3<C1>&axis, BYTE direction,TMatrix4<C2>&result);			//!< Creates a system matrix from a point and an axis and stores the result in \b out. \param position Base position of the new system \param axis Orientation axis \param direction Coordinate vector of the final matrix that should point along the specified axis (0-2). For instance passing 1 as \b direction effectivly causes the y axis of the final matrix to point along the specified axis. \param result Out 4x4 matrix for the constructed system matrix.
			MFUNC2(void)		makeAxisSystem(const TVec3<C0>&axis, BYTE direction,TMatrix3<C1>&result);									//!< Creates an orientation matrix from the specified axis and stores the result in \b out. \param axis Orientation axis \param direction Coordinate vector of the final matrix that should point along the specified axis (0-2). For instance passing 1 as \b direction effectivly causes the y axis of the final matrix to point along the specified axis. \param result Out 3x3 matrix for the constructed orientation matrix.

			MFUNC(void)			resetBottomRow(TMatrix3<C>&matrix);																			//!< Resets the final row of the specified matrix to (0, 0, 1)
			MFUNC(void)			resetBottomRow(TMatrix4<C>&matrix);																			//!< Resets the final row of the specified matrix to (0, 0, 0, 1)
			MFUNC2(void)		copyOrientation(const TMatrix4<C0>&from,TMatrix4<C1>&to);													//!< Copies the x, y, and z components of the first three axes to @b target. both the final row and final column of @b to are left unchanged
			MFUNC2(void)		copyOrientation(const TMatrix4<C0>&from,TMatrix3<C1>&to);													//!< Copies the x, y, and z components of the first three axes to @b target
			MFUNC2(void)		copyOrientation(const TMatrix3<C0>&from,TMatrix4<C1>&to);													//!< Copies the x, y, and z components of the first three axes to @b target
			MFUNC4(void)		buildSystem(const TVec3<C0>&base,const TVec3<C1>&x,const TVec3<C2>&y, TMatrix4<C3>&out);						//!< Creates a system matrix based on base, x and y axes. The x axis is primary and used exactly as it is (though normalized). The y axis is normalized and used as close as it is orthogonal to x. @param base 3d vector to the base of the system @param x X axis of the system (any length > 0) @param y Y axis of the system (any length > 0) @param out Out matrix for the finished 4x4 system matrix

			MFUNC(String)		ToString(const TMatrix4<C>&matrix);
			MFUNC(String)		ToString(const TMatrix3<C>&matrix);

			MFUNCV(bool)		Invert(const TMatrix<C,Dimensions,Dimensions>&source,TMatrix<C,Dimensions,Dimensions>&resultOut);

			namespace Raw
			{
				MFUNCV (void)	Eye(C*out);

				MFUNC (String)	ToStringD(const C*m, count_t rows, count_t cols);
				MFUNC (String)	ToString4x4(const C m[16])	{return ToStringD(m,4,4);}

				MFUNC (void)    SwapRowsD(C*m, count_t rows, count_t cols, index_t r1, index_t r2);
				MFUNC (void)    SwapColsD(C*m, count_t rows, count_t cols, index_t c1, index_t c2);
				MFUNCD (void)	SwapColsD(C*m, index_t c1, index_t c2);
				MFUNCC (void)	SwapCols(C*m);
				MFUNCR (void)	SwapRows(C*m);
				MFUNCD (void)	SwapRows(C*m, index_t r1, index_t r2);

				MFUNC2V (bool)  Invert(const C0*m, C1*out);
				MFUNC (void)    EyeD(C*out, count_t dimension);
				MFUNC (void)    ClearD(C*m, count_t width, count_t height);
				MFUNC2 (bool)	Invert3x3(const C0 m[3*3], C1 out[3*3]);
				MFUNC2 (bool)	Invert4x4(const C0 m[4*4], C1 out[4*4]);
				MFUNC2 (bool)	InvertD(const C0*m, C1*out, count_t dimension);

				MFUNCM (void)	Mult(const T0*m, const T1*n, T2*out);
				MFUNC3 (void)	MultD(const C0*m, const C1*n, C2*out, count_t height, count_t mwidth, count_t nwidth);
			}
		}





		//------------------------------------------------------------------------------

		#include "matrix.tpl.h"
	}


	namespace Mat = M::Mat;
}

#endif
