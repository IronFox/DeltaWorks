#ifndef matrix_mathTplH
#define matrix_mathTplH

/******************************************************************

Collection of template matrix-math-functions.

******************************************************************/


//namespace Math
//{

	//template <typename T>
	//	T			Matrix<T>::eye2[2*2] = {1,0, 0,1};
	/*template <typename T>
		T			Matrix<T>::eye3[3*3] = {1,0,0,0,1,0,0,0,1};
	template <typename T>
		T			Matrix<T>::eye4[4*4] = {1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1};*/
	template <typename T>
		const	TMatrix2<T>	Matrix<T>::eye2 = {1,0, 0,1};
	template <typename T>
		const	TMatrix3<T>	Matrix<T>::eye3 = {1,0,0, 0,1,0, 0,0,1};
	template <typename T>
		const	TMatrix4<T>	Matrix<T>::eye4 = {1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1};

	template <class C, count_t Dimensions, index_t DY = Dimensions, index_t DX = Dimensions> 
		class MQEye;



    template <class C, count_t Step, count_t Count> class MQSwap
    {
    public:
            static MF_DECLARE	(void)    perform(C*m,C*n)
                            {
                                swp((*m),(*n));
                                MQSwap<C,Step,Count-1>::perform(m+Step,n+Step);
                            }
    };
	template <class C, count_t Step> class MQSwap<C,Step,1>
	{
	public:
			static MF_DECLARE	(void)    perform(C*m,C*n)
							{
								swp((*m),(*n));
							}
	};


	
	namespace ByReference
	{
	
		MFUNC2
			(void)    __scaleSystem(C0 m[4*4], const C1&scalar)
			{
				_multiply(m,scalar);
				_multiply(m+4,scalar);
				_multiply(m+8,scalar);
			}

		MFUNC2V
			(void)    __rotationMatrix(const C0&angle, C1*out)
			{
				C1   bow = angle*M_PI/180;
				MATRIX_LOOP(Dimensions-2,Dimensions)
					out[(x+2)*Dimensions+y] = x+2==y;
				MATRIX_LOOP(Dimensions,Dimensions-2)
					out[x*Dimensions+(y+2)] = x==y+2;
				MATRIX_LOOP(Dimensions-2,Dimensions-2)
					out[(x+2)*Dimensions+(y+2)] = x==y;
				out[0]= cos(bow);
				out[1] = -sin(bow);
				out[Dimensions]= sin(bow);
				out[Dimensions+1] = cos(bow);
			}

		MFUNC2
			(void)    __rotationMatrix(const C0&angle, C1*out, count_t dimension)
			{
				C1   bow = angle*M_PI/180;
				MATRIX_LOOP(dimension-2,dimension)
					out[(x+2)*dimension+y] = x+2==y;
				MATRIX_LOOP(dimension,dimension-2)
					out[x*dimension+(y+2)] = x==y+2;
				MATRIX_LOOP(dimension-2,dimension-2)
					out[(x+2)*dimension+(y+2)] = x==y;
				out[0]= cos(bow);
				out[1] = -sin(bow);
				out[dimension]= sin(bow);
				out[dimension+1] = cos(bow);
			}



		MFUNC5
			(void)    __rotationMatrix(const C0&angle, const C1&ax, const C2&ay, const C3&az, C4 out[3*3])
			{
				float   c = cos(angle*M_PI/180),
						t = 1-c,
						s = sin(angle*M_PI/180),
						l = sqrt(ax*ax+ay*ay+az*az),
						x = ax/l,
						y = ay/l,
						z = az/l;

				out[0] = t*x*x + c;
				out[1] = t*x*y - s*z;
				out[2] = t*x*z + s*y;

				out[3] = t*x*y + s*z;
				out[4] = t*y*y + c;
				out[5] = t*y*z - s*x;

				out[6] = t*x*z - s*y;
				out[7] = t*y*z + s*x;
				out[8] = t*z*z + c;
			}

		MFUNC3
			(void)    __rotationMatrix(const C0&angle, const C1 axis[3], C2 out[3*3])
			{
				float   c = cos(angle*M_PI/180),
						t = 1-c,
						s = sin(angle*M_PI/180),
						x = axis[0],
						y = axis[1],
						z = axis[2];

				out[0] = t*x*x + c;
				out[1] = t*x*y - s*z;
				out[2] = t*x*z + s*y;

				out[3] = t*x*y + s*z;
				out[4] = t*y*y + c;
				out[5] = t*y*z - s*x;

				out[6] = t*x*z - s*y;
				out[7] = t*y*z + s*x;
				out[8] = t*z*z + c;
			}




	
	
	}
	
	MFUNC4	(void)		__buildSystem(const C0 base[3],const C1 x[3],const C2 y[3], C3 out[4*4])
	{
		_c3(base,out+12);
		_c3(x,out);
		_normalize0(out);
		_cross(out,y,out+8);
		if (_dot(out+8)<GetError<C3>())
		{
			_v3(out+4,vabs(out[1])+1,out[2],out[0]);
			_cross(out,out+4,out+8);
		}
		_normalize(out+8);
		_cross(out+8,out,out+4);
		_normalize(out+4);
		
		out[3] = out[7] = out[11] = 0;
		out[15] = 1;
	}
		
		
    MFUNC17 (void)        __m4(C0 m[4*4], const C1&v11, const C2&v12, const C3&v13, const C4&v14,
                                        const C5&v21, const C6&v22, const C7&v23, const C8&v24,
                                        const C9&v31, const C10&v32, const C11&v33, const C12&v34,
                                        const C13&v41, const C14&v42, const C15&v43, const C16&v44)
    {
        m[0] = v11;
        m[1] = v21;
        m[2] = v31;
        m[3] = v41;

        m[4] = v12;
        m[5] = v22;
        m[6] = v32;
        m[7] = v42;

        m[8] = v13;
        m[9] = v23;
        m[10] = v33;
        m[11] = v43;

        m[12] = v14;
        m[13] = v24;
        m[14] = v34;
        m[15] = v44;
    }





    
    

    MFUNC3 (void)    __mult441(const C0 m[4*4], const C1 n[4], C2 out[4])
    {
        out[0] = m[0] * n[0] + m[4] * n[1] + m[8]  * n[2] + m[12] * n[3];
        out[1] = m[1] * n[0] + m[5] * n[1] + m[9]  * n[2] + m[13] * n[3];
        out[2] = m[2] * n[0] + m[6] * n[1] + m[10] * n[2] + m[14] * n[3];
        out[3] = m[3] * n[0] + m[7] * n[1] + m[11] * n[2] + m[15] * n[3];
    }


    MFUNC3 (void)    __mult4(const C0 m[4*4], const C1 n[4*4], C2 out[4*4])
    {
        __mult441(m,n,out);
        __mult441(m,n+4,out+4);
        __mult441(m,n+8,out+8);
        __mult441(m,n+12,out+12);
    }

	namespace Mat
	{

		/**
		Matrix operations on raw pointers
		*/
		namespace Raw
		{

			MFUNCV	(void)        Eye(C*out)
			{
				MQEye<C,Dimensions>::perform(out);
			}

    

			MFUNC (String) ToStringD(const C*m, count_t rows, count_t cols)
			{
				String rs = "matrix  ("+String(rows)+"x"+String(cols)+")";
				rs.Set(7,vChar<C>());
        
				using StringConversion::ToString;
				for (index_t row = 0; row < rows; row++)
				{
					rs+='\n';
					for (index_t col = 0; col < cols; col++)
						rs+=ToString(m[col*rows+row])+' ';

				}
				return rs;
			}


			MFUNC3 (void)	MultD(const C0*m, const C1*n, C2*out, count_t height, count_t mwidth, count_t nwidth)
			{
				MATRIX_LOOP(nwidth,height)
				{
					C0  sum = m[y]*n[x*mwidth];
					for (index_t k = 1; k < mwidth; k++)
						sum += m[k*height+y]*n[x*mwidth+k];
					out[x*height+y] = sum;
				}
			}

			MFUNCM	(void)    Mult(const T0*m, const T1*n, T2*out)
			{
				MultD(m,n,out,MHeight,MWidth,NWidth);
				//MQMult<T0,T1,T2,MHeight,MWidth,NWidth,MHeight,MWidth,NWidth>::perform(m,n,out,0);	//seems to be broken atm
			}

			MFUNC (void)    EyeD(C*out, count_t dimension)
			{
				MATRIX_LOOP(dimension,dimension)
					out[x*dimension+y]=x==y;
			}

			MFUNC (void)    ClearD(C*m, count_t width, count_t height)
			{
				Vec::clearD(m,width*height);
			}



			MFUNC2 (bool) Invert3x3(const C0 m[3*3], C1 out[3*3])
			{
				return Invert<C0,C1,3>(m,out);
			}

			MFUNC2 (bool) Invert4x4(const C0 m[4*4], C1 out[4*4])
			{
				return Invert<C0,C1,4>(m,out);
			}


			MFUNC (void)    SwapRowsD(C*m, count_t rows, count_t cols, index_t r1, index_t r2)
			{
				VECTOR_LOOP(cols)
				{
					C save = m[el*rows+r1];
					m[el*rows+r1] = m[el*rows+r2];
					m[el*rows+r2] = save;
				}
			}
			MFUNC (void)    SwapColsD(C*m, count_t rows, count_t cols, index_t c1, index_t c2)
			{
				VECTOR_LOOP(rows)
				{
					C save = m[c1*rows+el];
					m[c1*rows+el] = m[c2*rows+el];
					m[c2*rows+el]=save;
				}
			}
			MFUNCD	(void) SwapColsD(C*m, index_t c1, index_t c2)
			{
				MQSwap<C,1,Rows>::perform(m+c1*Rows,m+c2*Rows);
			}

			MFUNCC	(void)    SwapCols(C*m)
			{
				MQSwap<C,1,Rows>::perform(m+Col0*Rows,m+Col1*Rows);
			}


			MFUNCR	(void)    SwapRows(C*m)
			{
				MQSwap<C,Rows,Cols>::perform(m+Row0,m+Row1);
			}
    
			MFUNCD	(void)	SwapRows(C*m, index_t r1, index_t r2)
			{
				MQSwap<C,Rows,Cols>::perform(m+r1,m+r2);
			}



			MFUNC2 (bool) InvertD(const C0*m, C1*out, count_t dimension)
			{
				C0*buffer = alloc<C0>(2*dimension*dimension);
				Vec::copyD(m,buffer,dimension*dimension);
				EyeD(&buffer[dimension*dimension],dimension);
				for (index_t line = 0; line < dimension; line++)
				{
					index_t targetline = line;
					while (vabs(buffer[line*dimension+targetline]) <= GetError<C0>() && targetline < dimension)
						targetline++;
					if (targetline == dimension)
					{
						dealloc(buffer);
						return false;
					}
					if (targetline != line)
						SwapRowsD(buffer,dimension,2*dimension,targetline,line);
					for (index_t i = line+1; i < dimension; i++)
					{
						C0 a = buffer[line*dimension+i]/buffer[line*dimension+line];
						for (index_t j = line+1; j < 2*dimension; j++)
							buffer[j*dimension+i] -= buffer[j*dimension+line]*a;
						buffer[line*dimension+i] = 0;
					}
				}
				Vec::clearD(out,dimension*dimension);
				for (index_t i = 0; i < dimension; i++)
					for (index_t line=dimension-1; line < dimension; line--)
					{
						C0 a = 0;
						for (index_t inner=0; inner < dimension-line; inner++)
							a+=buffer[(inner+line)*dimension+line]*out[i*dimension+(inner+line)];
						if (!buffer[line*dimension+line])
						{
							dealloc(buffer);
							return false;
						}
						out[i*dimension+line]=(buffer[(dimension+i)*dimension+line]-a)/buffer[line*dimension+line];
					}
				dealloc(buffer);
				return true;
			}


			MFUNC2V (bool)    Invert(const C0*m, C1*out)
			{
				C0  buffer[2*Dimensions*Dimensions];
				VecUnroll<Dimensions*Dimensions>::copy(m,buffer);
				Eye<C0,Dimensions>(&buffer[Dimensions*Dimensions]);

				using std::fabs;
				for (index_t line = 0; line < Dimensions; line++)
				{
					index_t targetline = line;
					while (fabs(buffer[line*Dimensions+targetline]) <= GetError<C0>() && targetline < Dimensions)
						targetline++;
					if (targetline == Dimensions)
						return false;
					if (targetline != line)
						SwapRows<C0,Dimensions,2*Dimensions>(buffer,targetline,line);
					for (index_t i = line+1; i < Dimensions; i++)
					{
						C0 a = buffer[line*Dimensions+i]/buffer[line*Dimensions+line];
						for (index_t j = line+1; j < 2*Dimensions; j++)
							buffer[j*Dimensions+i] -= buffer[j*Dimensions+line]*a;
						buffer[line*Dimensions+i] = 0;
					}
				}
				VecUnroll<Dimensions*Dimensions>::clear(out);
				for (index_t i = 0; i < Dimensions; i++)
					for (index_t line=Dimensions-1; line < Dimensions; line--)
					{
						C0 a = 0;
						for (index_t inner=0; inner < Dimensions-line; inner++)
							a+=buffer[(inner+line)*Dimensions+line]*out[i*Dimensions+(inner+line)];
						if (!buffer[line*Dimensions+line])
							return false;
						out[i*Dimensions+line]=(buffer[(Dimensions+i)*Dimensions+line]-a)/buffer[line*Dimensions+line];
					}
				return true;
			}
		}


		MFUNCM(void)		Mult(const TMatrix<T0,MWidth,MHeight>&m, const TMatrix<T1,NWidth,MWidth>&n, TMatrix<T2,NWidth,MHeight>&outResult)
		{
			Raw::Mult<T0,T1,T2,MHeight,MWidth,NWidth>(m.v,n.v, outResult.v);
		}

		template <typename T0, typename T1, typename T2, count_t Rows, count_t Cols>
			void			Mult(const TMatrix<T0,Cols,Rows>&m, const TVec<T1,Cols>&v, TVec<T2,Rows>&outResult)
			{
				Raw::Mult<T0,T1,T2,Rows,Cols,1>(m.v,v.v,outResult.v);
			}


		MFUNCV(bool)		Invert(const TMatrix<C,Dimensions,Dimensions>&source,TMatrix<C,Dimensions,Dimensions>&resultOut)
		{
			return Raw::Invert<C,C,Dimensions>(source.v,resultOut.v);
		}



		MFUNC4(void)		buildSystem(const TVec3<C0>&base,const TVec3<C1>&x,const TVec3<C2>&y, TMatrix4<C3>&out)
		{
			Vec::copy(base,out.w.xyz);
			Vec::copy(x,out.x.xyz);
			Vec::normalize0(out.x.xyz);

			Vec::cross(out.x.xyz,y,out.z.xyz);
			if (Vec::dot(out.z.xyz) <= TypeInfo<C3>::error)
			{
				Vec::def(out.y.xyz,vabs(out.x.y)+1,out.x.z,out.x.x);
				Vec::cross(out.x.xyz,out.y.xyz,out.z.xyz);
			}
			Vec::normalize(out.z.xyz);
			Vec::cross(out.z.xyz,out.x.xyz,out.y.xyz);
			Vec::normalize(out.y.xyz);
			M::Mat::resetBottomRow(out);
		}

		MFUNC4(void)	reflectSystemN(const TVec3<C0>&base, const TVec3<C1>&normal, const TMatrix4<C2>&matrix, TMatrix4<C3>&result)
		{
			Vec::reflectVectorN(normal,matrix.x.xyz,result.x.xyz);
			Vec::reflectVectorN(normal,matrix.y.xyz,result.y.xyz);
			Vec::reflectVectorN(normal,matrix.z.xyz,result.z.xyz);
			Vec::reflectN(base,normal,matrix.w.xyz,result.w.xyz);
			resetBottomRow(result);
		}

		MFUNC4(void)	reflectMatrixN(const TVec3<C0>&base, const TVec3<C1>&normal, const TMatrix4<C2>&matrix, TMatrix4<C3>&result)
		{
			TVec4<C0>	reflect = {normal.x,normal.y,normal.z,-Vec::dot(normal,base)};
			Vec::mad(matrix.x,reflect,-2*Vec::dot(matrix.x,reflect),result.x);
			Vec::mad(matrix.y,reflect,-2*Vec::dot(matrix.y,reflect),result.y);
			Vec::mad(matrix.z,reflect,-2*Vec::dot(matrix.z,reflect),result.z);
			Vec::mad(matrix.w,reflect,-2*Vec::dot(matrix.w,reflect),result.w);
		}



		MFUNC3(void)	makeAxisSystem(const TVec3<C0>&position,const TVec3<C1>&axis, BYTE d,TMatrix4<C2>&result)
		{
			BYTE x = (d+1)%3,
				 y = (d+2)%3;
			//d *= 4;
			//x *= 4;
			//y *= 4;
			using std::fabs;
			Vec::def(result.axis[x].xyz,	fabs(axis.y)+1,axis.z,axis.x);
			Vec::copy(axis,result.axis[d].xyz);
			Vec::cross(result.axis[x].xyz,result.axis[d].xyz,result.axis[y].xyz);
			if (d == 2)
				Vec::cross(result.axis[y].xyz, result.axis[d].xyz, result.axis[x].xyz);	//there seems to be an orientation problem with z
			else
				Vec::cross(result.axis[d].xyz, result.axis[y].xyz, result.axis[x].xyz);

			Vec::normalize(result.x.xyz);
			Vec::normalize(result.y.xyz);
			Vec::normalize(result.z.xyz);
			Vec::copy(position,result.w.xyz);

			result.x.w = 0;
			result.y.w = 0;
			result.z.w = 0;
			result.w.w = 1;
		}
		MFUNC2(void)	makeAxisSystem(const TVec3<C0>&axis, BYTE d,TMatrix3<C1>&result)
		{
			BYTE x = (d+1)%3,
				 y = (d+2)%3;
			//d *= 3;
			//x *= 3;
			//y *= 3;
			using std::fabs;
			Vec::def(result.axis[x],	fabs(axis.y)+1,axis.z,axis.x);
			Vec::copy(axis, result.axis[d]);
			Vec::cross(result.axis[x],result.axis[d],result.axis[y]);
			Vec::cross(result.axis[d],result.axis[y],result.axis[x]);
			Vec::normalize(result.x);
			Vec::normalize(result.y);
			Vec::normalize(result.z);
		}



		MFUNC3(void)	Mult(const TMatrix2<C0>&m, const TVec2<C1>&v, TVec2<C2>&out)
		{
			out.x = m.x.x * v.x + m.y.x * v.y;
			out.y = m.x.y * v.x + m.y.y * v.y;
		}

		MFUNC2(void)	Mult(const TMatrix2<C0>&m, TVec2<C1>&v)
		{
			TVec2<C1> tmp = v;
			Mult(m,tmp,v);
		}

		MFUNC3(void)	Mult(const TMatrix2<C0>&m0, const TMatrix2<C1>&m1, TMatrix2<C2>&result)
		{
			Mult(m0,m1.x,result.x);
			Mult(m0,m1.y,result.y);
		}




		MFUNC3(void)	Mult(const TMatrix3<C0>&m, const TVec3<C1>&v, TVec3<C2>&out)
		{
			out.x = m.x.x * v.x + m.y.x * v.y + m.z.x * v.z;
			out.y = m.x.y * v.x + m.y.y * v.y + m.z.y * v.z;
			out.z = m.x.z * v.x + m.y.z * v.y + m.z.z * v.z;
		}

		MFUNC2(void)	Mult(const TMatrix3<C0>&m, TVec3<C1>&v)
		{
			TVec3<C1> tmp = v;
			Mult(m,tmp,v);
		}

		MFUNC3(void)	Mult(const TMatrix3<C0>&m0, const TMatrix3<C1>&m1, TMatrix3<C2>&result)
		{
			Mult(m0,m1.x,result.x);
			Mult(m0,m1.y,result.y);
			Mult(m0,m1.z,result.z);
		}

		MFUNC3(void)	Mult(const TMatrix4<C0>&m, const TVec4<C1>&v, TVec4<C2>&result)
		{
			result.x = m.x.x * v.x + m.y.x * v.y + m.z.x * v.z + m.w.x * v.w;
			result.y = m.x.y * v.x + m.y.y * v.y + m.z.y * v.z + m.w.y * v.w;
			result.z = m.x.z * v.x + m.y.z * v.y + m.z.z * v.z + m.w.z * v.w;
			result.w = m.x.w * v.x + m.y.w * v.y + m.z.w * v.z + m.w.w * v.w;
		}
		MFUNC2(void)	Mult(const TMatrix4<C0>&m, TVec4<C1>&v)
		{
			TVec4<C1> temp = v;
			Mult(m,temp,v);
		}

		MFUNC3(void)	Mult(const TMatrix4<C0>&m0, const TMatrix4<C1>&m1, TMatrix4<C2>&result)
		{
			Mult(m0,m1.x,result.x);
			Mult(m0,m1.y,result.y);
			Mult(m0,m1.z,result.z);
			Mult(m0,m1.w,result.w);
		}


		MFUNC2 (void) BuildRotationMatrix(C0 angle, TMatrix2<C1>&out)
		{
			C1   bow = angle*M_PI/180;
			C1	cs = cos(bow),
				sn = sin(bow);
			out.x.x = cs;
			out.x.y = -sn;
			out.y.x = sn;
			out.y.y = cs;
		}

		MFUNC2(void)		FillXRotationMatrix(C0 angle, TMatrix3<C1>&out)
		{
			C1   bow = angle*M_PI/180;
			C1	cs = cos(bow),
				sn = sin(bow);
			out.x.x = 1;
			out.x.y = 0;
			out.x.z = 0;
			out.y.x = 0;
			out.y.y = cs;
			out.y.z = -sn;
			out.z.x = 0;
			out.z.y = sn;
			out.z.z = cs;
		}

		MFUNC2(void)		FillYRotationMatrix(C0 angle, TMatrix3<C1>&out)
		{
			C1   bow = angle*M_PI/180;
			C1	cs = cos(bow),
				sn = sin(bow);
			out.x.x = cs;
			out.x.y = 0;
			out.x.z = -sn;
			out.y.x = 0;
			out.y.y = 1;
			out.y.z = 0;
			out.z.x = sn;
			out.z.y = 0;
			out.z.z = cs;
		}

		MFUNC2(void)		FillZRotationMatrix(C0 angle, TMatrix3<C1>&out)
		{
			C1   bow = angle*M_PI/180;
			C1	cs = cos(bow),
				sn = sin(bow);
			out.x.x = cs;
			out.x.y = -sn;
			out.x.z = 0;
			out.y.x = sn;
			out.y.y = cs;
			out.y.z = 0;
			out.z.x = 0;
			out.z.y = 0;
			out.z.z = 1;
		}

		MFUNC3(void)	BuildRotationMatrix(C0 angle,const TVec3<C1>&axis,TMatrix3<C2>&result)
		{
			C2		c = cos(angle*M_PI/180),
					t = 1-c,
					s = sin(angle*M_PI/180),
					x = axis.x,
					y = axis.y,
					z = axis.z;

			result.x.x = t*x*x + c;
			result.x.y = t*x*y - s*z;
			result.x.z = t*x*z + s*y;

			result.y.x = t*x*y + s*z;
			result.y.y = t*y*y + c;
			result.y.z = t*y*z - s*x;

			result.z.x = t*x*z - s*y;
			result.z.y = t*y*z + s*x;
			result.z.z = t*z*z + c;
		}

		MFUNC5(void)	BuildRotationMatrix(C0 angle, C1 ax, C2 ay, C3 az, TMatrix3<C4>&result)
		{
			C4		c = cos(angle*M_PI/180),
					t = 1-c,
					s = sin(angle*M_PI/180),
					l = sqrt(ax*ax+ay*ay+az*az),
					x = ax/l,
					y = ay/l,
					z = az/l;

			result.x.x = t*x*x + c;
			result.x.y = t*x*y - s*z;
			result.x.z = t*x*z + s*y;

			result.y.x = t*x*y + s*z;
			result.y.y = t*y*y + c;
			result.y.z = t*y*z - s*x;

			result.z.x = t*x*z - s*y;
			result.z.y = t*y*z + s*x;
			result.z.z = t*z*z + c;
		}

		MFUNC2(void)	transpose(const TMatrix4<C0>&m, TMatrix4<C1>&out)
		{
			out.x.x = m.x.x;
			out.x.y = m.y.x;
			out.x.z = m.z.x;
			out.x.w = m.w.x;

			out.y.x = m.x.y;
			out.y.y = m.y.y;
			out.y.z = m.z.y;
			out.y.w = m.w.y;

			out.z.x = m.x.z;
			out.z.y = m.y.z;
			out.z.z = m.z.z;
			out.z.w = m.w.z;

			out.w.x = m.x.w;
			out.w.y = m.y.w;
			out.w.z = m.z.w;
			out.w.w = m.w.w;
		}



		MFUNC2(void)	transpose(const TMatrix3<C0>&m, TMatrix3<C1>&out)
		{
			out.x.x = m.x.x;
			out.x.y = m.y.x;
			out.x.z = m.z.x;
			out.y.x = m.x.y;
			out.y.y = m.y.y;
			out.y.z = m.z.y;
			out.z.x = m.x.z;
			out.z.y = m.y.z;
			out.z.z = m.z.z;
		}

		MFUNC(void)	transpose(TMatrix3<C>&m)
		{
			swp(m.x.y,m.y.x);
			swp(m.x.z,m.z.x);
			swp(m.y.z,m.z.y);
		}

		MFUNC(void)	transpose(TMatrix4<C>&m)
		{
			swp(m.x.y,m.y.x);
			swp(m.x.z,m.z.x);
			swp(m.x.w,m.w.x);

			swp(m.y.z,m.z.y);
			swp(m.y.w,m.w.y);

			swp(m.z.w,m.w.z);
		}

		MFUNC(TMatrix3<C>)	transposed(const TMatrix3<C>&m)
		{
			TMatrix3<C>	tmp;
			transpose(m,tmp);
			return tmp;
		}

		MFUNC(TMatrix4<C>)	transposed(const TMatrix4<C>&m)
		{
			TMatrix4<C>	tmp;
			transpose(m,tmp);
			return tmp;
		}


		MFUNC2(void)	transpose(const TMatrix2<C0>&m, TMatrix2<C1>&out)
		{
			out.x.x = m.x.x;
			out.x.y = m.y.x;
			out.y.x = m.x.y;
			out.y.y = m.y.y;
		}

		MFUNC(void)	transpose(TMatrix2<C>&m)
		{
			swp(m.x.y,m.y.x);
		}

		MFUNC(TMatrix2<C>)	transposed(const TMatrix2<C>&m)
		{
			TMatrix2<C>	tmp;
			transpose(m,tmp);
			return tmp;
		}


		MFUNC3 (void)    rotate(const TMatrix4<C0>&system, const TVec3<C1>&vec, TVec3<C2>&result)
		{
			result.x = system.x.x*vec.x + system.y.x*vec.y + system.z.x*vec.z;
			result.y = system.x.y*vec.x + system.y.y*vec.y + system.z.y*vec.z;
			result.z = system.x.z*vec.x + system.y.z*vec.y + system.z.z*vec.z;
		}
		MFUNC2(void)    rotate(const TMatrix4<C0>&system, TVec3<C1>&vec)
		{
			TVec3<C1>	temp = vec;
			rotate(system,temp,vec);
		}

		MFUNC2 (void)    Rotate(const TMatrix4<C0>&system, TVec3<C1>&vec)
		{
			rotate(system,vec);
		}
		MFUNC3 (void)    Rotate(const TMatrix4<C0>&system, const TVec3<C1>&vec, TVec3<C2>&result)
		{
			rotate(system,vec,result);
		}
		MFUNC3 (void)    Transform(const TMatrix4<C0>&system, const TVec3<C1>&vec, TVec3<C2>&result)
		{
			result.x = system.x.x*vec.x + system.y.x*vec.y + system.z.x *vec.z + system.w.x;
			result.y = system.x.y*vec.x + system.y.y*vec.y + system.z.y *vec.z + system.w.y;
			result.z = system.x.z*vec.x + system.y.z*vec.y + system.z.z *vec.z + system.w.z;
		}

		MFUNC2(void)    Transform(const TMatrix4<C0>&system, TVec3<C1>&vec)
		{
			TVec3<C1>	temp = vec;
			Transform(system,temp,vec);
		}


		MFUNC3 (void)    rotate(const TMatrix4<C0>*system, const TVec3<C1>&vec, TVec3<C2>&result)
		{
			rotate(*system,vec,result);
		}
		MFUNC2(void)    rotate(const TMatrix4<C0>*system, TVec3<C1>&vec)
		{
			TVec3<C1>	temp = vec;
			rotate(*system,temp,vec);
		}

		MFUNC3 (void)    Transform(const TMatrix4<C0>*system, const TVec3<C1>&vec, TVec3<C2>&result)
		{
			Transform(*system,vec,result);
		}

		MFUNC2(void)    Transform(const TMatrix4<C0>*system, TVec3<C1>&vec)
		{
			TVec3<C1>	temp = vec;
			Transform(*system,temp,vec);
		}


		MFUNC3(void)	transformSystem(const TMatrix4<C0>&system0, const TMatrix4<C1>&system1, TMatrix4<C2>&result)
		{
			rotate(system0,system1.x.xyz,result.x.xyz);
			rotate(system0,system1.y.xyz,result.y.xyz);
			rotate(system0,system1.z.xyz,result.z.xyz);
			Transform(system0,system1.w.xyz,result.w.xyz);
			result.x.w = result.y.w = result.z.w = 0;
			result.w.w = 1;
		}

		MFUNC2(void)	transformSystem(const TMatrix4<C0>&system0, TMatrix4<C1>&system1)
		{
			TMatrix4<C1> temp = system1;
			transformSystem(system0,temp,system1);
		}


		MFUNC(void)		resetBottomRow(TMatrix3<C>&matrix)
		{
			matrix.x.z = matrix.y.z = 0;
			matrix.z.z = 1;
		}

		MFUNC(void)		resetBottomRow(TMatrix4<C>&matrix)
		{
			matrix.x.w = matrix.y.w = matrix.z.w = 0;
			matrix.w.w = 1;
		}

		MFUNC2(void)	copyOrientation(const TMatrix4<C0>&from,TMatrix4<C1>&to)
		{
			Vec::copy(from.x.xyz,to.x.xyz);
			Vec::copy(from.y.xyz,to.y.xyz);
			Vec::copy(from.z.xyz,to.z.xyz);
		}

		MFUNC2(void)	copyOrientation(const TMatrix4<C0>&from,TMatrix3<C1>&to)
		{
			Vec::copy(from.x.xyz,to.x);
			Vec::copy(from.y.xyz,to.y);
			Vec::copy(from.z.xyz,to.z);
		}

		MFUNC2(void)	copyOrientation(const TMatrix3<C0>&from,TMatrix4<C1>&to)
		{
			Vec::copy(from.x,to.x.xyz);
			Vec::copy(from.y,to.y.xyz);
			Vec::copy(from.z,to.z.xyz);
		}

		MFUNC2(void)		cast(const TMatrix4<C0>&from, TMatrix4<C1>&to)
		{
			Vec::copy(from.x,to.x);
			Vec::copy(from.y,to.y);
			Vec::copy(from.z,to.z);
			Vec::copy(from.w,to.w);

		}




		MFUNC2(void)	invertSystem(const TMatrix4<C0>&sys, TMatrix4<C1>&result)
		{
			Vec::def(result.x,	sys.x.x, sys.y.x, sys.z.x, 0);
			Vec::def(result.y,	sys.x.y, sys.y.y, sys.z.y, 0);
			Vec::def(result.z,	sys.x.z, sys.y.z, sys.z.z, 0);

			rotate(result,sys.w.xyz,result.w.xyz);
			result.w.w = 1;
			Vec::multiply(result.w.xyz,(C1)-1);
		}

		MFUNC2(bool)	invert(const TMatrix4<C0>&sys, TMatrix4<C1>&result)
		{
	        return Raw::Invert<C0,C1,4>(sys.v,result.v);
		}

		MFUNC2(bool)	invert(const TMatrix3<C0>&sys, TMatrix3<C1>&result)
		{
	        return Raw::Invert<C0,C1,3>(sys.v,result.v);
		}

		MFUNC(String)		ToString(const TMatrix4<C>&matrix)
		{
			return Raw::ToStringD(matrix.v,4,4);
		}
		MFUNC(String)		ToString(const TMatrix3<C>&matrix)
		{
			return Raw::ToStringD(matrix.v,3,3);
		}


	}

    MFUNC3 (void)    __mult331(const C0 m[3*3], const C1 n[3], C2 out[3])
    {
        out[0] = m[0] * n[0] + m[3] * n[1] + m[6] * n[2];
        out[1] = m[1] * n[0] + m[4] * n[1] + m[7] * n[2];
        out[2] = m[2] * n[0] + m[5] * n[1] + m[8] * n[2];
    }

    MFUNC2 (void)    __mult331(const C0 m[3*3], C1 n[3])
    {
        C1	x = m[0] * n[0] + m[3] * n[1] + m[6] * n[2],
			y = m[1] * n[0] + m[4] * n[1] + m[7] * n[2],
			z = m[2] * n[0] + m[5] * n[1] + m[8] * n[2];
		n[0] = x;
		n[1] = y;
		n[2] = z;
    }

    MFUNC3 (void)    __mult3(const C0 m[3*3], const C1 n[3*3], C2 out[3*3])
    {
        __mult331(m,n,out);
        __mult331(m,n+3,out+3);
        __mult331(m,n+6,out+6);
    }

    MFUNC3  (void)    __transform(const C0 system[4*4], const C1 point[3], C2 out[3])
    {
        out[0] = system[0]*point[0] + system[4]*point[1] + system[8] *point[2] + system[12];
        out[1] = system[1]*point[0] + system[5]*point[1] + system[9] *point[2] + system[13];
        out[2] = system[2]*point[0] + system[6]*point[1] + system[10]*point[2] + system[14];
    }
       
	MFUNC2  (void)    __transform(const C0 system[4*4], C1 point[3])
    {
        C0	x  = system[0]*point[0] + system[4]*point[1] + system[8] *point[2] + system[12],
			y = system[1]*point[0] + system[5]*point[1] + system[9] *point[2] + system[13];
        point[2] = system[2]*point[0] + system[6]*point[1] + system[10]*point[2] + system[14];
		point[0] = x;
		point[1] = y;
    }
    
    MFUNC3 (void)     __transformSystem(const C0 system0[4*4], const C1 system1[4*4], C2 out[4*4])
    {
        __rotate(system0,system1,out);
        __rotate(system0,system1+4,out+4);
        __rotate(system0,system1+8,out+8);
        __transform(system0,system1+12,out+12);
        out[3] = out[7] = out[11] = 0;
        out[15] = 1;
    }

    MFUNC2 (void)     __transformSystem(const C0 system0[4*4], C1 inout_system1[4*4])
    {
		C1 temp[16];
		__transformSystem(system0,inout_system1,temp);
		_c16(temp,inout_system1);
    }

    MFUNC3  (void)    __transformReduced(const C0 system[3*4], const C1 point[3], C2 out[3])
    {
        out[0] = system[0]*point[0] + system[3]*point[1] + system[6]*point[2] + system[9];
        out[1] = system[1]*point[0] + system[4]*point[1] + system[7]*point[2] + system[10];
        out[2] = system[2]*point[0] + system[5]*point[1] + system[8]*point[2] + system[11];
    }

    MFUNC3 (void)    __rotateReduced(const C0 system[3*4], const C1 vec[3], C2 out[3])
    {
        out[0] = system[0]*vec[0] + system[3]*vec[1] + system[6]*vec[2];
        out[1] = system[1]*vec[0] + system[4]*vec[1] + system[7]*vec[2];
        out[2] = system[2]*vec[0] + system[5]*vec[1] + system[8]*vec[2];
    }

    MFUNC3  (void)    __multiplyReduced(const C0 sys0[3*4], const C1 sys1[3*4], C2 out[3*4])
    {
        __multiply<C0,C1,C2,3,3,1>(sys0,sys1,out);
        __multiply<C0,C1,C2,3,3,1>(sys0,sys1+3,out+3);
        __multiply<C0,C1,C2,3,3,1>(sys0,sys1+6,out+6);
        __transformReduced(sys0,sys1+9,out+9);
    }

    MFUNC3  (void)    __rotate(const C0 system[4*4], const C1 vec[3], C2 out[3])
    {
        out[0] = system[0]*vec[0] + system[4]*vec[1] + system[8] *vec[2];
        out[1] = system[1]*vec[0] + system[5]*vec[1] + system[9] *vec[2];
        out[2] = system[2]*vec[0] + system[6]*vec[1] + system[10]*vec[2];
    }

    MFUNC2  (void)    __rotate(const C0 system[4*4], C1 vec[3])
    {
        C0 x  = system[0]*vec[0] + system[4]*vec[1] + system[8] *vec[2],
			y = system[1]*vec[0] + system[5]*vec[1] + system[9] *vec[2];
        vec[2] = system[2]*vec[0] + system[6]*vec[1] + system[10]*vec[2];
		vec[0] = x;
		vec[1] = y;
    }

    MFUNC3 (void)    __add(const C0*m, count_t m_height, const C1*n, count_t n_height, C2*out, count_t width, count_t height)
    {
        MATRIX_LOOP(width,height)
            out[x*height+y] = m[x*m_height+y]+n[x*n_height+y];
    }

    MFUNC3 (void)    __subtract(const C0*m, count_t m_height, const C1*n, count_t n_height, C2*out, count_t width, count_t height)
    {
        MATRIX_LOOP(width,height)
            out[x*height+y] = m[x*m_height+y]-n[x*n_height+y];
    }

    MFUNC2 (void)    __extractUpper(const C0*m, count_t m_height, C1*out, count_t dimension)
    {
        MATRIX_LOOP(dimension,dimension)
            out[x*dimension+y] = y>=x?x==y:m[x*m_height+y];
    }

    MFUNC2 (void)    __extractUpper(const C0*m, C1*out, count_t dimension)
    {
        MATRIX_LOOP(dimension,dimension)
            out[x*dimension+y] = y>=x?x==y:m[x*dimension+y];
    }

    MFUNC2 (void)    __extractLower(const C0*m, count_t m_height, C1*out, count_t dimension)
    {
        MATRIX_LOOP(dimension,dimension)
            out[x*dimension+y] = y<=x?x==y:m[x*m_height+y];
    }

    MFUNC2 (void)    __extractLower(const C0*m, C1*out, count_t dimension)
    {
        MATRIX_LOOP(dimension,dimension)
            out[x*dimension+y] = y<=x?x==y:m[x*dimension+y];
    }

    MFUNC2 (void)    __extractDiagonal(const C0*m, count_t m_height, C1*out, count_t dimension)
    {
        MATRIX_LOOP(dimension,dimension)
            out[x*dimension+y] = y!=x?0:m[x*m_height+y];
    }

    MFUNC2 (void)    __extractDiagonal(const C0*m, C1*out, count_t dimension)
    {
        MATRIX_LOOP(dimension,dimension)
            out[x*dimension+y] = y!=x?0:m[x*dimension+y];
    }


    MFUNC3 (void)    __solveGaussSeidel(const C0*m, count_t m_height, const C1*result, C2*out, count_t dimension, unsigned accuracy)
    {
        C2 *old = alloc<C2>(dimension);
        _clear(old,dimension);
        for (unsigned k = 0; k < accuracy; k++)
        {
            for (index_t i = 0; i < dimension; i++)
            {
                C0 temp = result[i],
                  sum = 0;
                for (index_t j = 0; j < i; j++)
                    sum+=m[j*m_height+i]*out[j];
                temp-=sum;
                sum = 0;
                for (index_t j = i+1; j<dimension; j++)
                    sum+=m[j*m_height+i]*old[j];
                temp-=sum;
                if (temp < 0)
                    temp *=-1;
                out[i] = temp/m[i*m_height+i];
            }
            if (k<accuracy-1)
                _copy(out,old,dimension);
        }
        dealloc(old);
    }

    MFUNC3 (void)    __solveGaussSeidel(const C0*m, const C1*result, C2*out, count_t dimension, unsigned accuracy)
    {
        __solveGaussSeidel(m,dimension,result,out,dimension,accuracy);
    }

    MFUNC3 (void)    __solveJacobi(const C0*m, count_t m_height, const C1*result, C2*out, count_t dimension, unsigned accuracy)
    {
        C2*old = alloc<C2>(dimension);
        _clear(old,dimension);
        for (unsigned k = 0; k < accuracy; k++)
        {
            for (index_t i = 0; i < dimension; i++)
            {
                out[i] = 0;
                for (index_t j = 0; j < dimension; j++)
                    if (j!=i)
                        out[i]+= m[j*m_height+i]*old[j];
                out[i] = (result[i]-out[i])/m[i*m_height+i];
            }
            if (k<accuracy-1)
                _copy(out,old,dimension);
        }
        dealloc(old);
    }

    MFUNC3 (void)    __solveJacobi(const C0*m, const C1*result, C2*out, count_t dimension, unsigned accuracy)
    {
        __solveJacobi(m,dimension,result,out,dimension,accuracy);
    }

    MFUNC2 (void)     __invertSystem(const C0 m[4*4], C1 out[4*4])
    {
        _v4(out,    m[0],m[4],m[8],0);
        _v4(out+4,m[1],m[5],m[9],0);
        _v4(out+8,m[2],m[6],m[10],0);
        __rotate(out,m+12,out+12);
        out[15] = 1;
        _multiply(out+12,(C1)-1);
    }


    MFUNC2 (void)     __invertReduced(const C0 m[3*4], C1 out[3*4])
    {
        _v3(out,m[0],m[3],m[6]);
        _v3(out+3,m[1],m[4],m[7]);
        _v3(out+6,m[2],m[5],m[8]);
        _v3(out+9,- (out[0] * m[9] + out[3] * m[10] + out[6] * m[11]),- (out[1] * m[9] + out[4] * m[10] + out[7] * m[11]),- (out[2] * m[9] + out[5] * m[10] + out[8] * m[11]));
    }


    MFUNC2 (bool)    __invertGauss(const C0*m, C1*out, count_t dimension)
    {
        C0*buffer = alloc<C0>(2*dimension*dimension);
        _copy(m,buffer,dimension*dimension);
        M::Mat::Raw::Eye(&buffer[dimension*dimension],dimension);
        for (index_t line = 1; line < dimension; line++)
            for (index_t i = 0; i < line; i++)
            {
                if (!buffer[i*dimension+i])
                {
                    dealloc(buffer);
                    return false;
                }
                C0 a = -buffer[i*dimension+line]/buffer[i*dimension+i];
                for (index_t j = 0; j < 2*dimension; j++)
                    buffer[j*dimension+line]+=a*buffer[j*dimension+i];
            }
        __clear(out,dimension,dimension);
        for (index_t i = 0; i < dimension; i++)
            for (index_t line=dimension-1; line < dimension; line--)
            {
                C0 a = 0;
                for (index_t inner=line; inner < dimension; inner++)
                    a+=buffer[inner*dimension+line]*out[i*dimension+inner];
                if (!buffer[line*dimension+line])
                {
                    dealloc(buffer);
                    return false;
                }
                out[i*dimension+line]=(buffer[(dimension+i)*dimension+line]-a)/buffer[line*dimension+line];
            }
        dealloc(buffer);
        return true;
    }
    

    MFUNC2 (void)    __transpose(const C0*m, C1*out, count_t dimension)
    {
        MATRIX_LOOP(dimension,dimension)
            out[x*dimension+y] = m[y*dimension+x];
    }

    MFUNC2 (void)        __transpose3(const C0 m[3*3], C1 out[3*3])
    {
        out[0] = m[0];
        out[1] = m[3];
        out[2] = m[6];
        out[3] = m[1];
        out[4] = m[4];
        out[5] = m[7];
        out[6] = m[2];
        out[7] = m[5];
        out[8] = m[8];
    }




    MFUNC (void)    __normalizeSystem(C m[4*4])
    {
        _normalize(m);
        _normalize(m+4);
        _normalize(m+8);
    }

    MFUNC2
		(void)    __scaleSystem(C0 m[4*4], C1 scalar)
		{
			_multiply(m,scalar);
			_multiply(m+4,scalar);
			_multiply(m+8,scalar);
		}


    MFUNC2 (void)    __copy9(const C0 source[9], C1 destination[9])
    {
        destination[0] = source[0];     //i manually unroll this cause some compilers seem to lack this optimization
        destination[1] = source[1];
        destination[2] = source[2];
        destination[3] = source[3];
        destination[4] = source[4];
        destination[5] = source[5];
        destination[6] = source[6];
        destination[7] = source[7];
        destination[8] = source[8];
    }

    MFUNC2 (void)    __copy11(const C0 source[11], C1 destination[11])
    {
        destination[0x0] = source[0x0];     //i manually unroll this cause some compilers seem to lack this optimization
        destination[0x1] = source[0x1];
        destination[0x2] = source[0x2];
        destination[0x3] = source[0x3];
        destination[0x4] = source[0x4];
        destination[0x5] = source[0x5];
        destination[0x6] = source[0x6];
        destination[0x7] = source[0x7];
        destination[0x8] = source[0x8];
        destination[0x9] = source[0x9];
        destination[0xA] = source[0xA];
    }

    MFUNC2 (void)    __copy12(const C0 source[12], C1 destination[12])
    {
        destination[0x0] = source[0x0];     //i manually unroll this cause some compilers seem to lack this optimization
        destination[0x1] = source[0x1];
        destination[0x2] = source[0x2];
        destination[0x3] = source[0x3];
        destination[0x4] = source[0x4];
        destination[0x5] = source[0x5];
        destination[0x6] = source[0x6];
        destination[0x7] = source[0x7];
        destination[0x8] = source[0x8];
        destination[0x9] = source[0x9];
        destination[0xA] = source[0xA];
        destination[0xB] = source[0xB];
    }

    MFUNC2 (void)    __copy16(const C0 source[16], C1 destination[16])
    {
        destination[0x0] = source[0x0];     //i manually unroll this cause some compilers seem to lack this optimization
        destination[0x1] = source[0x1];
        destination[0x2] = source[0x2];
        destination[0x3] = source[0x3];
        destination[0x4] = source[0x4];
        destination[0x5] = source[0x5];
        destination[0x6] = source[0x6];
        destination[0x7] = source[0x7];
        destination[0x8] = source[0x8];
        destination[0x9] = source[0x9];
        destination[0xA] = source[0xA];
        destination[0xB] = source[0xB];
        destination[0xC] = source[0xC];
        destination[0xD] = source[0xD];
        destination[0xE] = source[0xE];
        destination[0xF] = source[0xF];
    }

    MFUNC2 (void)    __copy(const C0*source, count_t m_height, C1*destination, count_t out_height, count_t width, count_t height)
    {
        MATRIX_LOOP(width,height)
            destination[x*out_height+y] = source[x*m_height+y];
    }
    
    MFUNCD	(C) __determinant(const C*m)
    {
        C temp[Cols*Rows],rs(1);
        VecUnroll<Cols*Rows>::copy(m,temp);
        count_t dim = Cols<=Rows?Cols:Rows;
        for (index_t row = 0; row < dim-1; row++)
        {
            index_t x = row;
            while (vabs(temp[row*Rows+x]) < GetError<C>() && x < dim)
                x++;
            if (x == dim)
                return 0;
            if (x != row)
            {
                __swapRows<Rows,Cols>(temp,row,x);
                rs*=-1;
            }
            for (index_t i = row+1; i < dim; i++)
            {
                C a = temp[row*Rows+i]/temp[row*Rows+row];
                for (index_t j = row+1; j < dim; j++)
                    temp[j*Rows+i]-= temp[j*Rows+row]*a;
                temp[row*Rows+i] = 0;
            }
        }
        for (index_t i = 0; i < dim; i++)
            rs*=temp[i*Rows+i];
        return rs;
    }

    MFUNC (C)       __determinant(const C*m, count_t rows, count_t cols)
    {
        C*temp = alloc<C>(cols*rows),rs(1);
        _copy(m,temp,cols*rows);
        count_t dim = vmin(cols,rows);
        for (index_t row = 0; row < dim-1; row++)
        {
            index_t x = row;
            while (vabs(temp[row*rows+x]) < GetError<C>() && x < dim)
                x++;
            if (x == dim)
                return 0;
            if (x != row)
            {
                __swapRows(temp,rows,dim,row,x);
                rs*=-1;
            }
            for (index_t i = row+1; i < dim; i++)
            {
                C a = temp[row*rows+i]/temp[row*rows+row];
                for (index_t j = row+1; j < dim; j++)
                    temp[j*rows+i]-= temp[j*rows+row]*a;
                temp[row*rows+i] = 0;
            }
        }
        for (index_t i = 0; i < dim; i++)
            rs*=temp[i*rows+i];
        dealloc(temp);
        return rs;
    }




    

    

    MFUNC2 (void)    __rowMult(C0*m, count_t width, count_t height, index_t row, C1 value)
    {
        VECTOR_LOOP(width)
            m[el*height+row]*=value;
    }




    MFUNC2V
		(void)    __rotationMatrix(C0 angle, C1*out)
		{
			C1   bow = angle*M_PI/180;
			MATRIX_LOOP(Dimensions-2,Dimensions)
				out[(x+2)*Dimensions+y] = x+2==y;
			MATRIX_LOOP(Dimensions,Dimensions-2)
				out[x*Dimensions+(y+2)] = x==y+2;
			MATRIX_LOOP(Dimensions-2,Dimensions-2)
				out[(x+2)*Dimensions+(y+2)] = x==y;
			out[0]= cos(bow);
			out[1] = -sin(bow);
			out[Dimensions]= sin(bow);
			out[Dimensions+1] = cos(bow);
		}


    MFUNC2
		(void)    __rotationMatrix(C0 angle, C1*out, count_t dimension)
		{
			C1   bow = angle*M_PI/180;
			MATRIX_LOOP(dimension-2,dimension)
				out[(x+2)*dimension+y] = x+2==y;
			MATRIX_LOOP(dimension,dimension-2)
				out[x*dimension+(y+2)] = x==y+2;
			MATRIX_LOOP(dimension-2,dimension-2)
				out[(x+2)*dimension+(y+2)] = x==y;
			out[0]= cos(bow);
			out[1] = -sin(bow);
			out[dimension]= sin(bow);
			out[dimension+1] = cos(bow);
		}

	MFUNC2 (void)		__makeAxisSystem(const C0 axis[3], BYTE d,C1 out[3*3])
	{
        BYTE x = (d+1)%3,
             y = (d+2)%3;
        d *= 3;
        x *= 3;
        y *= 3;
        _v3(out+x,vabs(axis[1])+1,axis[2],axis[0]);
        _c3(axis,out+d);
        _cross(out+x,out+d,out+y);
        _cross(out+d,out+y,out+x);
        _normalize(out);
        _normalize(out+3);
        _normalize(out+6);
	}


    MFUNC3 (void)     __makeAxisSystem(const C0 position[3],const C1 axis[3], BYTE d,C2 out[4*4])
    {
        BYTE x = (d+1)%3,
             y = (d+2)%3;
        d *= 4;
        x *= 4;
        y *= 4;
        _v3(out+x,vabs(axis[1])+1,axis[2],axis[0]);
        _c3(axis,out+d);
        _cross(out+x,out+d,out+y);
        _cross(out+d,out+y,out+x);
        _normalize(out);
        _normalize(out+4);
        _normalize(out+8);
        _c3(position,out+12);
        out[3]  = 0;
        out[7]  = 0;
        out[11] = 0;
        out[15] = 1;
    }


    MFUNC5
		(void)    __rotationMatrix(C0 angle, C1 ax, C2 ay, C3 az, C4 out[3*3])
		{
			float   c = cos(angle*M_PI/180),
					t = 1-c,
					s = sin(angle*M_PI/180),
					l = sqrt(ax*ax+ay*ay+az*az),
					x = ax/l,
					y = ay/l,
					z = az/l;

			out[0] = t*x*x + c;
			out[1] = t*x*y - s*z;
			out[2] = t*x*z + s*y;

			out[3] = t*x*y + s*z;
			out[4] = t*y*y + c;
			out[5] = t*y*z - s*x;

			out[6] = t*x*z - s*y;
			out[7] = t*y*z + s*x;
			out[8] = t*z*z + c;
		}

    MFUNC3
		(void)    __rotationMatrix(C0 angle, const C1 axis[3], C2 out[3*3])
		{
			float   c = cos(angle*M_PI/180),
					t = 1-c,
					s = sin(angle*M_PI/180),
					x = axis[0],
					y = axis[1],
					z = axis[2];

			out[0] = t*x*x + c;
			out[1] = t*x*y - s*z;
			out[2] = t*x*z + s*y;

			out[3] = t*x*y + s*z;
			out[4] = t*y*y + c;
			out[5] = t*y*z - s*x;

			out[6] = t*x*z - s*y;
			out[7] = t*y*z + s*x;
			out[8] = t*z*z + c;
		}


    MFUNC (bool)        __isIdentity(const C*matrix, count_t dimension)
    {
        for (index_t x = 0; x < dimension; x++)
            for (index_t y = 0; y < dimension; y++)
                if (vabs((C)(x==y) - matrix[x*4+y]) > GetError<C>)
                    return false;
        return true;
    }
    
	#ifndef __CUDACC__
    MFUNC (bool)        __isIdentity3(const C matrix[3*3])
    {
        return vabs(matrix[0]-(C)1)+vabs(matrix[1])+vabs(matrix[2])+vabs(matrix[3])+vabs(matrix[4]-(C)1)+vabs(matrix[5])+vabs(matrix[6])+vabs(matrix[7])+vabs(matrix[8]-(C)1)<=GetError<C>*9;
    }
	#endif
    
    MFUNC (bool)        __isIdentity4(const C matrix[4*4])
    {
        return  vabs(matrix[0]-(C)1)
                +vabs(matrix[1])
                +vabs(matrix[2])
                +vabs(matrix[3])
                +vabs(matrix[4])
                +vabs(matrix[5]-(C)1)
                +vabs(matrix[6])
                +vabs(matrix[7])
                +vabs(matrix[8])
                +vabs(matrix[9])
                +vabs(matrix[10]-(C)1)
                +vabs(matrix[11])
                +vabs(matrix[12])
                +vabs(matrix[13])
                +vabs(matrix[14])
                +vabs(matrix[15]-(C)1)
                <=GetError<C>()*16;
    }
    
    #ifndef __CUDACC__
    MFUNCV (bool)        __isIdentity(const C*matrix)
    {
        C eye[Dimensions*Dimensions];
        M::Mat::Raw::Eye<C,Dimensions>(eye);
		VecUnroll<16>::sub(eye,matrix);
        return VecUnroll<16>::sum(eye) < GetError<C>*Dimensions*Dimensions;
    }
	#endif


    MFUNC (void)        __eye3(C*out)
    {
        out[0] = 1;
        out[1] = 0;
        out[2] = 0;

        out[3] = 0;
        out[4] = 1;
        out[5] = 0;
        
        out[6] = 0;
        out[7] = 0;
        out[8] = 1;
    }
    
    
    MFUNC (void)        __eye4(C*out)
    {
        out[0]  = 1;
        out[1]  = 0;
        out[2]  = 0;
        out[3]  = 0;
        
        out[4]  = 0;
        out[5]  = 1;
        out[6]  = 0;
        out[7]  = 0;
        
        out[8]  = 0;
        out[9]  = 0;
        out[10] = 1;
        out[11] = 0;
        
        out[12] = 0;
        out[13] = 0;
        out[14] = 0;
        out[15] = 1;
    }





    #ifndef __BORLANDC__ //expecting a capable compiler here




    /* ok, this is the fastest i could do. It's almost (5% difference) as fast as simply writing down the sequential commands.*/


    template <class C0, class C1, class C2, count_t Height, count_t mWidth, count_t nWidth, index_t CY, index_t CX, index_t Walk> class MQMult
    {
        public:
                static MF_DECLARE	(void)    perform(const C0*m, const C1*n, C2*out, const C2&prev=0)
                                {
                                    MQMult<C0,C1,C2,Height,mWidth,nWidth,CY,CX,Walk-1>::perform(m+Height,n+1,out,prev+(*m)*(*n));
                                }
    };


    template <class C0, class C1, class C2, count_t Height, count_t mWidth, count_t nWidth, index_t CY, index_t CX> class MQMult<C0,C1,C2,Height,mWidth,nWidth,CY,CX,1>
    {
        public:
                static MF_DECLARE	(void)    perform(const C0*m, const C1*n, C2*out, const C2&prev)
                                {
                                    (*out) = (*m)*(*n)+prev;
                                    MQMult<C0,C1,C2,Height,mWidth,nWidth,CY-1,CX,mWidth>::perform(m+(1-Height*(mWidth-1)),n-(mWidth-1),out+1);
                                }
    };
    template <class C0, class C1, class C2, count_t Height, count_t mWidth, count_t nWidth, index_t CX> class MQMult<C0,C1,C2,Height,mWidth,nWidth,1,CX,1>
    {
        public:
                static MF_DECLARE	(void)    perform(const C0*m, const C1*n, C2*out, const C2&prev)
                                {
                                    (*out) = (*m)*(*n)+prev;
                                    MQMult<C0,C1,C2,Height,mWidth,nWidth,Height,CX-1,mWidth>::perform(m+(1-Height*mWidth),n+1,out+1);
                                }
    };
    template <class C0, class C1, class C2, count_t Height, count_t mWidth, count_t nWidth> class MQMult<C0,C1,C2,Height,mWidth,nWidth,1,1,1>
    {
        public:
                static MF_DECLARE	(void)    perform(const C0*m, const C1*n, C2*out, const C2&prev)
                                {
                                    (*out) = (*m)*(*n)+prev;
                                }
    };


    

    template <class C, count_t Dimensions, index_t DY, index_t DX> class MQEye
    {
    public:
            static MF_DECLARE	(void)    perform(C*out)
                            {
                                (*out) = DX == DY;
                                MQEye<C,Dimensions,DY-1,DX>::perform(out+1);
                            }
    };

    template <class C, count_t Dimensions, index_t DX> class MQEye<C,Dimensions,1,DX>
    {
    public:
            static MF_DECLARE	(void)    perform(C*out)
                            {
                                (*out) = DX ==1;
                                MQEye<C,Dimensions,Dimensions,DX-1>::perform(out+1);
                            }
    };

    template <class C, count_t Dimensions> class MQEye<C,Dimensions,1,1>
    {
    public:
            static MF_DECLARE	(void)    perform(C*out)
                            {
                                (*out) = 1;
                            }
    };

    

    template <class C0, class C1, count_t Dimensions, index_t DY=Dimensions, index_t DX=Dimensions> class MQTranspose
    {
    public:
            static MF_DECLARE (void)    perform(const C0*m, C1*out)
                            {
                                (*out) = (*m);
                                MQTranspose<C0,C1,Dimensions,DY-1,DX>::perform(m+Dimensions,out+1);
                            }
    };
    template <class C0, class C1, count_t Dimensions, index_t DX> class MQTranspose<C0,C1,Dimensions,1,DX>
    {
    public:
            static MF_DECLARE (void)     perform(const C0*m,C1*out)
                            {
                                (*out) = (*m);
                                MQTranspose<C0,C1,Dimensions,Dimensions,DX-1>::perform(m-(Dimensions-1)*Dimensions+1,out+1);
                            }
    };
    template <class C0, class C1, count_t Dimensions> class MQTranspose<C0,C1,Dimensions,1,1>
    {
    public:
            static MF_DECLARE (void)     perform(const C0*m,C1*out)
                            {
                                (*out) = (*m);
                            }
    };
    
    MFUNC2V	(void) __transpose(const C0*m, C1*out)
    {
        MQTranspose<C0,C1,Dimensions>::perform(m,out);
    }

    #else   //well, don't expect any efficiency here :P




    /* ok, this is the fastest i could do. It's almost (5% difference) as fast as simply writing down the sequential commands.*/


    

    MFUNCM	(void)    __multiply(const C0*m, const C1*n, C2*out)
    {
        __multiply(m,n,out,Height,mWidth,nWidth);
    }


    MFUNCV	(void)        __eye(C*out)
    {
        __eye(out,Dimensions);
    }
	

	

    MFUNCD	(void) __swapCols(C*m, index_t c1, index_t c2)
    {
        __swapCols(m,Rows,Cols,c1,c2);
    }

    MFUNCC	(void)    __swapCols(C*m)
    {
        __swapCols(m,Rows,Cols,Col0,Col1);
    }


    MFUNCR	(void)    __swapRows(C*m)
    {
        __swapRows(m,Rows,Cols,Row0,Row1);
    }

    MFUNCD	(void) __swapRows(C*m, index_t r1, index_t r2)
    {
        __swapRows(m,Rows,Cols,r1,r2);
    }


    MFUNC2V (void) __transpose(const C0*m, C1*out)
    {
        __transpose(m,out,Dimensions);
    }

    #endif
	MFUNC (void)		__swapCols4(C*m, index_t c1, index_t c2)
	{
		if (c1 == c2)
			return;
		swp(m[c1*4],m[c2*4]);
		swp(m[c1*4+1],m[c2*4+1]);
		swp(m[c1*4+2],m[c2*4+2]);
		swp(m[c1*4+3],m[c2*4+3]);
	}
	
	MFUNC (void)		__swapRows4(C*m, index_t c1, index_t c2)
	{
		if (c1 == c2)
			return;
		swp(m[c1],m[c2]);
		swp(m[4+c1],m[4+c2]);
		swp(m[8+c1],m[8+c2]);
		swp(m[12+c1],m[12+c2]);
	}

//}










#endif
